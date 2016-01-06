/*
    Copyright (c) 2014 <copyright holder> <email>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/


#include "SessionManager.h"
#include "EventLoopThread.h"
#include "Accept.h"
#include "TraceLog.h"

#include "PackDef.h"
#include "ParsePdu.h"
//static uint16_t current_port = 10000;//set a range??

SessionManager::SessionManager()
    : accept_ ( NULL ),
      current_loop_thread_ ( NULL ),
      current_loop_id_ ( 0 )
{
    invalid_sess_key_.clear();
    loop_threads_list_.clear();
    sessions_.clear();
}

SessionManager::~SessionManager()
{
    std::vector<EventLoopThread *>::iterator it = loop_threads_list_.begin();
    for ( ; it != loop_threads_list_.end(); it++ ) {
        EventLoopThread *loop = *it;
        loop->stop();
        delete loop;
    }
    loop_threads_list_.clear();
    //delete sessions_;
}

void SessionManager::onTimer ( uint64_t now_sec )
{
    updateSessions ( now_sec );
}

bool SessionManager::startEventloopThread ( int thd_num )
{
    for ( int i = 0; i < thd_num; i++ ) {
        EventLoopThread *loop = new EventLoopThread();
        loop->start();
        loop_threads_list_.push_back ( loop );
    }

    LogINFO ( "thread number = %d", thd_num ); //log thread id

    return true;
}

EventLoopThread *SessionManager::getEventLoopThread()
{
    EventLoopThread *loop = loop_threads_list_[current_loop_id_++];

    if ( current_loop_id_ >= loop_threads_list_.size() ) {
        current_loop_id_ = 0;
    }

    return loop;
}

void SessionManager::deleteSession ( uint64_t sess_name )
{
    Session *session = sessions_[sess_name];
    sessions_.erase ( sess_name );
    SAFE_DELETE ( session );
}

void SessionManager::updateSessions ( uint64_t usec )
{
    std::vector<uint64_t> dead;
    MutexLockGuard l ( mutex_ );
    std::map<uint64_t, Session *>::iterator iter = sessions_.begin();
    for ( ; iter != sessions_.end(); iter++ ) { //change timer to sort...can save some time.
        Session *sess = iter->second;
        if ( sess != NULL ) {
            if ( sess->checkSessionState ( usec ) ) {
                dead.push_back ( sess->getName() );
            }
        } else {
            uint16_t port = iter->first & 0xffff;
            uint32_t ip = iter->first >> 16;
            LogERR ( "when updateSessions, sess is NULL, ip is %s, port is %u", ConvertIp(ip).c_str(), port );
        }
    }
    if ( !dead.empty() ) {
        handleInvalidSessions ( dead );
    }
}

ChannelBase *UDPSessionManager::findChannel ( uint64_t &key )
{
    MutexLockGuard l ( mutex_ );
    ChannelBase *chl = static_cast<ChannelBase *> ( caller_chl_map_.HashSearch ( key ) );
    if ( chl ) {
        chl->setLastRecvTime ( now_sec() );
    }

    return chl;
}

void UDPSessionManager::addCallerChlMap ( uint64_t caller_key, ChannelBase *chl )
{
    //  caller_chl_map_[caller_key] = chl;
    caller_chl_map_.HashInsert ( caller_key, static_cast<void *> ( chl ) );
    LogDEBUG ( "addCallerChlMap caller_key = %llu", caller_key );
}

void UDPSessionManager::removeCallerChlMap ( uint64_t caller_key )
{
    caller_chl_map_.HashDelKey ( caller_key );
    LogDEBUG ( "removeCallerChlMap caller_key = %llu", caller_key );
}

void UDPSessionManager::removeSessions()
{
    std::vector<uint64_t>::iterator it = invalid_sess_key_.begin();
    for ( ; it != invalid_sess_key_.end(); it++ ) {
        removeCallerChlMap ( *it );
        deleteSession ( *it );
    }
    invalid_sess_key_.clear();
}

void UDPSessionManager::resetOrCreateSession ( InetAddress &addr_caller, InetAddress &addr_callee )
{
    uint64_t caller_key = createKey ( addr_caller );

    bool exist = false;
    {
        MutexLockGuard l ( mutex_ );
        std::map<uint64_t, Session *>::iterator it = sessions_.find ( caller_key );
        if ( it != sessions_.end() ) {
            UDPSession *se = static_cast<UDPSession *> ( it->second );
            exist = true;
            //reset

            LogDEBUG ( "reset udpsession caller = %s:%u, callee = %s:%u",
                       ConvertIp(addr_caller.ip ).c_str(), addr_caller.port, ConvertIp(addr_callee.ip).c_str(), addr_callee.port );
            se->setRemoteAddress ( addr_caller, addr_callee );

            return;
        }
    }

    if ( !exist ) {
        //create
        LogDEBUG ( "new udpsession caller = %s:%u, callee = %s:%u",
                   ConvertIp(addr_caller.ip ).c_str(), addr_caller.port, ConvertIp(addr_callee.ip ).c_str(), addr_callee.port );
        createSession ( addr_caller, addr_callee );
    }
}

Session *UDPSessionManager::createSession ( InetAddress &addr_caller, InetAddress &addr_callee )
{
    UDPSession *sess = new UDPSession ( this, getEventLoopThread(), static_cast<UDPAccept *> ( accept_ ) );
    if ( sess == NULL ) {
        LogERR ( "createSession failed" );
        return NULL;
    }
    sess->setName ( createKey ( addr_caller ) );
    sess->createChannelPair();
    sess->setRemoteAddress ( addr_caller, addr_callee );
    sess->addChannelToPoll();
    /*Channel::InetAddress addr;
    addr.ip = accept_->getListenAddress().ip;//htonl(inet_addr("192.168.1.46"));
    do {
      addr.port = current_port++;
      if (current_port >= 65535) {
        current_port = 10000;
      }
    } while (!sess->calleeBind(addr));*/

    //accept_->addCallerChlMap(sess->getName(), call_er);

    //lock sessions_...create in the accept thread.but del in the timer thread...delete session in the timer???
    MutexLockGuard l ( mutex_ );
    addCallerChlMap ( sess->getName(), sess->getCaller() );
    sessions_[sess->getName()] = sess;

    return sess;
}

void TCPSessionManager::resetOrCreateSession ( InetAddress &addr_caller, int conn_fd )
{
    uint64_t caller_key = createKey ( addr_caller );

    bool exist = false;
    {
        MutexLockGuard l ( mutex_ );
        std::map<uint64_t, Session *>::iterator it = sessions_.find ( caller_key );
        if ( it != sessions_.end() ) {
            Session *se = it->second;
            exist = true;
            //reset
            LogDEBUG ( "reset tcpsession caller ip = %s, caller port = %u, %d",
                       ConvertIp(addr_caller.ip ).c_str(), addr_caller.port, conn_fd );
            se->setCallerRemoteAddress ( addr_caller );
            static_cast<TCPSession *> ( se )->resetCallerFD ( conn_fd );

            return;
        }
    }

    if ( !exist ) {
        //create
        Session *s = createSession ( addr_caller, conn_fd );
        LogDEBUG ( "new tcpsession caller ip = %s, port = %u, call_er = %p",
                   ConvertIp(addr_caller.ip ).c_str(), addr_caller.port, s->getCaller() );
    }
}

Session *TCPSessionManager::createSession ( InetAddress &addr_caller, int conn_fd )
{
    Session *sess = new TCPSession ( this, getEventLoopThread(), conn_fd );
    if ( NULL == sess ) {
        LogERR ( "createSession failed" );
        return NULL;
    }

    sess->setName ( createKey ( addr_caller ) );
    sess->createChannelPair();
    sess->setCallerRemoteAddress ( addr_caller );
    sess->addChannelToPoll();

    //lock sessions_...create in the accept thread.but del in the timer thread
    MutexLockGuard l ( mutex_ );
    sessions_[sess->getName()] = sess;

    return sess;
}
