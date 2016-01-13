#ifndef ACCEPT_H
#define ACCEPT_H
#include "TypeDef.h"

#include "Channel.h"
#include <map>
class SessionManager;

class Accept
{
public:
    enum ACCEPT_TYPE {T_UDP = 0, T_TCP};
    static Accept *createAccept ( InetAddress &listen_addr, SessionManager *session_mgr, ACCEPT_TYPE type );
    static void deleteAccept ( Accept *accept );

    InetAddress &getListenAddress() {
        return listen_address_;
    }

    void start() {
        listen();
    }

private:
    virtual void listen() = 0;

protected:
    Accept ( InetAddress &listen_addr, SessionManager *session_mgr );
    virtual ~Accept() {}

protected:
    InetAddress listen_address_;
    SessionManager *session_mgr_;

    //Mutex mutex_;
};

class UDPAccept : public Accept, public UDPChannel
{

public:
    UDPAccept ( InetAddress &listen_addr, SessionManager *session_mgr );
    virtual ~UDPAccept();

    void setRemoteAddress ( InetAddress &addr );
    void close();

protected:
    void handleRecv();
    void handleSend();
    void handleError();
    void handleClose();

    void doSuccessAuth( InetAddress& addr_caller, InetAddress& addr_callee );
    void doHeartBeat (InetAddress& addr_caller) {}
private:
    void listen();
    char recv_buf_[PACK_MAX_LEN];
    ssize_t buf_len_;
};

class TCPAccept : public Accept, public TCPChannel
{

public:
    TCPAccept ( InetAddress &listen_addr, SessionManager *session_mgr );
    virtual ~TCPAccept();

    void setRemoteAddress ( InetAddress &addr );
    void close();

protected:
    void handleRecv();
    void handleSend() {}
    void handleError() {}
    void handleClose() {}

private:
    void listen();
};

#endif // ACCEPT_H

