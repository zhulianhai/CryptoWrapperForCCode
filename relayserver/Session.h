#ifndef SESSION_H
#define SESSION_H

#include "Channel.h"

class SessionManager;
class EventLoopThread;

class Session
{

public:
#define SAFE_DELETE(x) {if (x != NULL) {delete x; x = NULL;}}
    Session ( SessionManager *sess_manager, EventLoopThread *loop );
    virtual ~Session();

    bool calleeBind ( InetAddress &addr ) {
        return static_cast<Channel *> ( channel_ee_ )->bind ( addr );
    }

    void setCallerRemoteAddress ( InetAddress &addr );
    void setCalleeRemoteAddress ( InetAddress &addr );

    virtual bool checkSessionState ( uint64_t sec );

    uint64_t getName() const {
        return name_;
    }
    void setName ( uint64_t name ) {
        name_ = name;
    }

    virtual ChannelBase *createChannelPair() = 0;//return channel_er_;
    virtual void addChannelToPoll ();
    ChannelBase *getCaller() const {
        return channel_er_;
    }

private:
    bool checkChannelsTimeout ( uint64_t sec );

protected:
    SessionManager *manager_;
    uint64_t name_;
    EventLoopThread *loop_;
    ChannelBase *channel_er_;
    ChannelBase *channel_ee_;

private:

};

class UDPSession: public Session
{
public:
    UDPSession ( SessionManager *sess_manager, EventLoopThread *loop, Channel *chl );
    virtual ~UDPSession() {}

    void setRemoteAddress ( InetAddress &addr_caller, InetAddress &addr_callee );
    //void setAcceptChannel() {accept_channel_ = chl;}

    ChannelBase *createChannelPair();

private:
    Channel *accept_channel_;
};

class TCPSession: public Session
{
public:
    TCPSession ( SessionManager *sess_manager, EventLoopThread *loop, int conn_fd );
    virtual ~TCPSession() {}

    ChannelBase *createChannelPair();
    void resetCallerFD ( int fd ) {
        static_cast<TCPChannel *> ( channel_er_ )->resetSocket ( fd );
    }

private:
    int conn_fd_;
};

#endif // SESSION_H
