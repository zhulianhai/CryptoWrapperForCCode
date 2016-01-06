/*TODO:a channel is for a client connection or server connection.
 * eg:channel_er is connection between raycom and relay
 * channel_ee is connection between relay and mcu
 *
 * peer channel and current channel is a pair...eg:current channel is raycom to relay, peer channel is relay to mcu
 * remote addr for current channel, peer addr for peer channel.
 * peer address just for udp accept...
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#include "Socket.h"
#include "Buffer.h"
#include "Timer.h"
#include "Mutex.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "TypeDef.h"
#include "PackDef.h"

#include "NetCommon.h"

class Session;
using namespace NetCommon;

#define MEMPOOLSIZE 3

class EventLoopThread;

struct  DataBuffer {
    char data_[PACK_MAX_LEN];
    uint16_t data_len_;
    uint32_t ip_;
    uint16_t port_;
};

class ChannelBase
{
public:
    ChannelBase ( EventLoopThread *loop = NULL );
    virtual ~ChannelBase() {}

    virtual void setRemoteAddress ( InetAddress &addr ) {
        remote_addr_.ip = addr.ip;
        remote_addr_.port = addr.port;
    }
    InetAddress &getRemoteAddress() {
        return remote_addr_;
    }

    inline uint64_t getLastRecvTime() const {
        return last_recv_time_;
    }
    inline void setLastRecvTime ( uint64_t usec ) {
        //state_ = RW;
        last_recv_time_ = usec;
    }
    CHANNEL_STATE getState() const {
        return state_;
    }
    void setState ( CHANNEL_STATE state ) {
        state_ = state;
    }

    bool checkTimeout ( uint64_t sec );

    virtual void setPeerChannel ( ChannelBase *chl ) {
        peer_channel_ = chl;
    }
    ChannelBase *getPeerChannel() const {
        return peer_channel_;
    }

    virtual void close() {
        remote_addr_.ip = 0;
        remote_addr_.port = 0;
        state_ = CLOSED;
    }

    EventLoopThread *getLoop () const {
        return loop_;
    }

    void postTimeout ();
protected:
    InetAddress remote_addr_;
    InetAddress local_addr_;

    ChannelBase *peer_channel_;
    EventLoopThread *loop_;
    uint64_t last_recv_time_;

private:
    CHANNEL_STATE state_;
};

class Channel : public ChannelBase
{
public:
    virtual ~Channel();

    virtual bool bind ( InetAddress &addr ) {
        local_addr_.ip = addr.ip;
        local_addr_.port = addr.port;
        return socket_->Bind ( addr.ip, addr.port );
    }
    //InetAddress &getRemoteAddress() {return remote_addr_;}

    void addToPoll ();

    void handleEvent();

    virtual void close();

    void setEvents ( uint32_t events ) {
        events_ = events;
    }
    uint32_t getPostEvents() const {
        return post_events_;
    }

    int getSocketFD() const { //for get sd
        return socket_->GetSocket();
    }

    Socket *getSocket() {
        return socket_;
    }

    void copyBufferAndSend ( char *data_buf, uint16_t data_len ) {
        if ( getState() == CLOSED || getState() == TIMEOUT ) {
            return;
        }
        DataBuffer *recv_data = mempool_.GetMemery();
        if ( !memcpy ( recv_data->data_, data_buf, data_len ) ) {
            //syslog(LOG_ERR,..);
            return ;
        }
        recv_data->data_len_ = data_len;
        recv_data->ip_ = remote_addr_.ip;
        recv_data->port_ = remote_addr_.port;
        sendBuf ( recv_data );
    }

    virtual void sendBuf ( DataBuffer *buf ) {
        buffer_.Push ( buf );
        postSend();
    }
    void postRecv();
    void postSend();

    MemPool<DataBuffer, Mutex>& getMemPool() {
        return mempool_;
    }

    //TODO:move to a new class
    virtual void doSuccessAuth ( InetAddress& addr_caller, InetAddress& addr_callee ) = 0;
    virtual void doHeartBeat (InetAddress& addr_caller) = 0;
    virtual bool doRTPRTCP ( char* data, uint16_t len ) = 0;

protected:
    explicit Channel ( EventLoopThread *loop, int mempool_size = 5 );

    virtual void handleRecv() = 0;
    virtual void handleSend() = 0;
    virtual void handleError() = 0;
    virtual void handleClose() = 0;

protected:
    Socket *socket_;
    Buffer<DataBuffer, Mutex> buffer_;
    MemPool<DataBuffer, Mutex> mempool_;

    uint32_t events_;
    uint32_t post_events_;

    bool sending_;
    bool recving_;
};

class UDPChannel: public Channel
{
public:
    explicit UDPChannel ( EventLoopThread *loop, int mempool_size = 5 );
    ~UDPChannel() {}

    void setPeerInetAddress ( InetAddress &addr ) {
        peer_addr_.ip = addr.ip;
        peer_addr_.port = addr.port;
    }
    const InetAddress &getPeerInetAddress() const {
        return peer_addr_;
    }

    virtual void doSuccessAuth( InetAddress& addr_caller, InetAddress& addr_callee ) {}
    virtual void doHeartBeat (InetAddress& addr_caller) {}
    virtual bool doRTPRTCP ( char* data, uint16_t len ) {return true;}
protected:
    virtual void handleRecv();
    virtual void handleSend();
    virtual void handleError();
    virtual void handleClose();

protected:
    //peer_addr_ is for peer channel...
    InetAddress peer_addr_;//just for udp accept..
};

class TCPChannel: public Channel
{
public:
    TCPChannel ( EventLoopThread *loop, Session *session, int mempool_size = 5, int conn_fd = -1 );
    ~TCPChannel() {}

    void resetSocket ( int fd );

    void sendBuf ( DataBuffer *buf ) {
        buffer_.Push ( buf );
        if ( left_senddata_size_ == 0 ) {
            handleSend();
        } else {
            postSend();
        }
    }

    void close() {
        left_recvdata_size_ = 0;
        left_senddata_size_ = 0;
        Channel::close();
    }

    virtual void doSuccessAuth ( InetAddress& addr_caller, InetAddress& addr_callee );
    virtual void doHeartBeat (InetAddress& addr_caller);
    virtual bool doRTPRTCP ( char* data, uint16_t len );

protected:
    virtual void handleRecv();
    virtual void handleSend();
    virtual void handleError();
    virtual void handleClose();
    //int handleData ( char *data, uint16_t len );
private:
    //enum DATA_TYPE {T_OWN = 0, T_RTP, T_RTCP, T_UNKNOWN};

    void processData();

    inline void setSockOpt() {
        //int buf_size = 65535;
        //socket_->SetSocketOpt ( SOL_SOCKET, SO_RCVBUF, ( void * ) &buf_size, sizeof ( buf_size ) );
        //socket_->SetSocketOpt ( SOL_SOCKET, SO_SNDBUF, ( void * ) &buf_size, sizeof ( buf_size ) );
        socket_->SetTCPNoDelay();
    }

private:
    Session *session_;

#define BUF_SIZE PACK_MAX_LEN*10
    int left_recvdata_size_;
    char recv_buf_[BUF_SIZE];
    int left_senddata_size_;
    char send_buf_[PACK_MAX_LEN];
};

class HandleRecvData {
public:
    static int handlePduPack ( Channel& chl, InetAddress& addr_caller, char* data, uint16_t len );
    static void responseAuth ( Channel& chl, InetAddress &addr, uint8_t success );
    static void responseHeartbeat ( Channel& chl, InetAddress &addr );
    static void responseConf ( Channel& chl, InetAddress& addr );
};

#endif // CHANNEL_H
