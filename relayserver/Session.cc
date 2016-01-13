#include "Session.h"
#include "EventLoopThread.h"
#include "Accept.h"
#include "SessionManager.h"

#include <assert.h>

Session::Session ( SessionManager *sess_manager, EventLoopThread *loop )
    : manager_ ( sess_manager ),
      loop_ ( loop ),
      channel_er_ ( NULL ),
      channel_ee_ ( NULL )
{
}

Session::~Session()
{
    SAFE_DELETE ( channel_er_ );
    SAFE_DELETE ( channel_ee_ );
}

bool Session::checkSessionState ( uint64_t sec )
{
    assert ( channel_er_ != NULL && channel_ee_ != NULL );
    /*if (channel_ee_->getState() == Channel::CLOSED
      && channel_er_->getState() == Channel::CLOSED) {
        return true;
    }*/
    return checkChannelsTimeout ( sec );
    //return false;
}

void Session::setCallerRemoteAddress ( InetAddress &addr )
{
    channel_er_->setState ( INIT );
    channel_er_->setLastRecvTime ( now_sec() );
    channel_er_->setRemoteAddress ( addr );
}

void Session::setCalleeRemoteAddress ( InetAddress &addr )
{
    channel_ee_->setState ( INIT );
    channel_ee_->setLastRecvTime ( now_sec() );
    channel_ee_->setRemoteAddress ( addr );
}

bool Session::checkChannelsTimeout ( uint64_t sec )
{
    bool er = channel_er_->checkTimeout ( sec );
//    bool ee = channel_ee_->checkTimeout ( sec );
    if ( !er /*&& !ee*/ ) {
        return false;
    }

    //channel_er_->close();
    //channel_ee_->close();
    if ( channel_ee_->getState() == CLOSED
            && channel_er_->getState() == CLOSED ) {
        return true;
    }

    if ( channel_er_->getState() != TIMEOUT
            && channel_er_->getState() != CLOSED ) {
        channel_er_->postTimeout();
        channel_ee_->postTimeout();
    }
    /*if ( channel_ee_->getState() != TIMEOUT
            && channel_ee_->getState() != CLOSED ) {
        channel_ee_->postTimeout();
    }*/

    return false;
}

void Session::addChannelToPoll()
{
    if ( channel_er_->getLoop() != NULL ) {
        static_cast<Channel*> ( channel_er_ )->addToPoll();
    }
    if ( channel_ee_->getLoop() != NULL ) {
        static_cast<Channel*> ( channel_ee_ )->addToPoll();
    }
}

UDPSession::UDPSession ( SessionManager *sess_manager, EventLoopThread *loop, Channel *chl )
    : Session ( sess_manager, loop ),
      accept_channel_ ( chl )
{
}

ChannelBase *UDPSession::createChannelPair()
{
    channel_er_ = new ChannelBase();  //accept_channel_;
    channel_ee_ = new UDPChannel ( loop_ );

    //loop_->updateChannel(channel_ee_, EventLoopThread::O_ADD);
    channel_er_->setPeerChannel ( channel_ee_ );
    channel_ee_->setPeerChannel ( accept_channel_ ); //channel_er_);

    return channel_er_;
}

void UDPSession::setRemoteAddress ( InetAddress &addr_caller, InetAddress &addr_callee )
{
    //channel_er_->setPeerInetAddress ( addr_callee );
    static_cast<UDPChannel *> ( channel_ee_ )->setPeerInetAddress ( addr_caller );
    setCallerRemoteAddress ( addr_caller );
    setCalleeRemoteAddress ( addr_callee );
}

TCPSession::TCPSession ( SessionManager *sess_manager, EventLoopThread *loop, int conn_fd )
    : Session ( sess_manager, loop ),
      conn_fd_ ( conn_fd )
{

}

ChannelBase *TCPSession::createChannelPair()
{
    channel_er_ = new TCPChannel ( loop_, this, 1, conn_fd_ );
    channel_ee_ = new UDPChannel ( loop_ );

    channel_er_->setPeerChannel ( channel_ee_ );
    channel_ee_->setPeerChannel ( channel_er_ );

    return channel_er_;
}
