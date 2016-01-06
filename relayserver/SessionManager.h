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


#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "EventLoopThread.h"
#include "Session.h"
#include "Timer.h"
#include "Mutex.h"
#include "Hash.h"

#include <vector>
#include <map>
//#include <hash_map>

class Accept;
//class UDPAccept;

const uint64_t routine_time_to_check_sec = 1;

class SessionManager: public TimerCallback
{
public:
    SessionManager();
    virtual ~SessionManager();

    void onTimer ( uint64_t now_sec );
    void setAccept ( Accept *accept ) {
        accept_ = accept;
    }

    bool startEventloopThread ( int thd_num );
    EventLoopThread *getEventLoopThread();

    void updateSessions ( uint64_t usec );

    void deleteSession ( uint64_t sess_name );

    inline uint64_t createKey ( InetAddress &addr ) {
        uint64_t key = addr.ip;
        key = key << 16;
        key |= addr.port;

        return key;
    }

    void handleInvalidSessions ( std::vector<uint64_t> & dead_key ) {
        invalid_sess_key_.swap ( dead_key );
        /*static uint64_t del_invalid_call_routine_time = now_sec();
        uint64_t now = now_sec();
        if ( now - del_invalid_call_routine_time > routine_time_to_check_sec ) { //every routine_time_to_check_sec sec to delete invalid call
            handleRoutineTimeCheck ();
            del_invalid_call_routine_time = now;
        }*/
        handleRoutineTimeCheck();
    }

    inline void handleRoutineTimeCheck() {
        removeSessions();
    }

    virtual void removeSessions() = 0;

protected:
    Accept *accept_;
    typedef std::map<uint64_t, Session *> SessionMap;
    SessionMap sessions_;

    Mutex mutex_;
    std::vector<uint64_t> invalid_sess_key_;

private:
    typedef std::vector<EventLoopThread *> EventLoopThreadList;
    EventLoopThread *current_loop_thread_;
    EventLoopThreadList loop_threads_list_;

    std::size_t current_loop_id_;
};

class UDPSessionManager: public SessionManager
{
public:
    UDPSessionManager() {
        caller_chl_map_.Clear();
    }
    virtual ~UDPSessionManager() {}

    ChannelBase *findChannel ( uint64_t &key );

    void resetOrCreateSession ( InetAddress &addr_caller, InetAddress &addr_callee );
protected:
    Session *createSession ( InetAddress &addr_caller, InetAddress &addr_callee );

    inline void addCallerChlMap ( uint64_t caller_key, ChannelBase *chl );
    inline void removeCallerChlMap ( uint64_t caller_key );

    void removeSessions();
private:
    Hash caller_chl_map_;
};

class TCPSessionManager: public SessionManager
{
public:
    TCPSessionManager() {}
    virtual ~TCPSessionManager() {}

    void resetOrCreateSession ( InetAddress &addr_caller, int conn_fd );

protected:
    Session *createSession ( InetAddress &addr_caller, int conn_fd );

    void removeSessions() {
        //MutexLockGuard l(mutex_);
        std::vector<uint64_t>::iterator it = invalid_sess_key_.begin();
        for ( ; it != invalid_sess_key_.end(); it++ ) {
            deleteSession ( *it );
        }
        invalid_sess_key_.clear();
    }
};

#endif // SESSIONMANAGER_H
