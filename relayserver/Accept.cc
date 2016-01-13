#include <netinet/in.h>
#include <errno.h>
#include "Accept.h"
#include "SessionManager.h"
#include "TraceLog.h"
#include "ParsePdu.h"
#include "Channel.h"
#include <arpa/inet.h>

Accept *Accept::createAccept ( InetAddress &listen_addr, SessionManager *session_mgr, ACCEPT_TYPE type )
{
    Accept *apt = NULL;
    if ( type == T_UDP ) {
        apt = new UDPAccept ( listen_addr, session_mgr );
    } else {
        apt = new TCPAccept ( listen_addr, session_mgr );
    }
    return apt;
}

void Accept::deleteAccept ( Accept *accept )
{
    delete accept;
}

Accept::Accept ( InetAddress &listen_addr, SessionManager *session_mgr )
    : listen_address_ ( listen_addr ),
      session_mgr_ ( session_mgr )
{
}

UDPAccept::UDPAccept ( InetAddress &listen_addr, SessionManager *session_mgr )
    : Accept ( listen_addr, session_mgr ),
      UDPChannel ( session_mgr->getEventLoopThread(), 40 )
{
    //listen();
    addToPoll();
}

UDPAccept::~UDPAccept()
{

}

void UDPAccept::setRemoteAddress ( InetAddress &addr )
{
    Channel::setRemoteAddress ( addr );
}

void UDPAccept::listen()
{
    int reuse = 1;
    socket_->SetSocketOpt ( SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof ( int ) );
    socket_->Bind ( htonl(listen_address_.ip), listen_address_.port );

    LogDEBUG ( "listen ip = %s, port = %u", ConvertIp(htonl(listen_address_.ip)).c_str(), listen_address_.port );
}

void UDPAccept::handleRecv()
{
    struct sockaddr_in *from_addr;
    buf_len_ = socket_->RecvFrom ( recv_buf_, PACK_MAX_LEN, from_addr );
    if ( buf_len_ < 0 ) {
        LogERR ( "Recv data error,errno:%d, errdesc:%s", errno, strerror ( errno ) );
        return;
    }

    InetAddress addr_caller;
    InetAddress addr_callee;
    addr_caller.ip = ntohl ( from_addr->sin_addr.s_addr );
    addr_caller.port = ntohs ( from_addr->sin_port );

    UDPSessionManager *mng = static_cast<UDPSessionManager *> ( session_mgr_ );
    uint64_t key = mng->createKey ( addr_caller );

    ChannelBase *chl = mng->findChannel ( key );
    ParsePdu parseUDP;
    if ( chl != NULL ) {
        //is auth???
        if ( parseUDP.isOwnProtocol ( recv_buf_, buf_len_ ) ) {
            Channel &chl = *(static_cast<Channel*>(this));
            HandleRecvData::handlePduPack ( chl, addr_caller, recv_buf_, buf_len_ );
            HandleRecvData::responseAuth ( chl, addr_caller, RESPONSE::SUCC_AUTH );
            return;
        }
        //chl->setLastRecvTime ( now_sec() );
        Channel *peer_chl = static_cast<Channel *> ( chl->getPeerChannel() );
        peer_chl->copyBufferAndSend ( recv_buf_, buf_len_ );
    } else {
        if ( !parseUDP.isOwnProtocol ( recv_buf_, buf_len_ ) ) {
            Channel &chl = *(static_cast<Channel*>(this));
            HandleRecvData::responseAuth ( chl, addr_caller, RESPONSE::NEED_REAUTH );
            return;
        }
        Channel &chl = *(static_cast<Channel*>(this));
        HandleRecvData::handlePduPack ( chl, addr_caller, recv_buf_, buf_len_ );
    }
}

void UDPAccept::handleSend()
{
    UDPChannel::handleSend();
}

void UDPAccept::handleError()
{
    UDPChannel::handleError();
}

void UDPAccept::handleClose()
{
    UDPChannel::handleClose();
}

void UDPAccept::doSuccessAuth ( InetAddress& addr_caller, InetAddress& addr_callee )
{
    UDPSessionManager *mng = static_cast<UDPSessionManager *> ( session_mgr_ );
    mng->resetOrCreateSession(addr_caller, addr_callee);
}

void UDPAccept::close()
{
    UDPChannel::close();
}

TCPAccept::TCPAccept ( InetAddress &listen_addr, SessionManager *session_mgr )
    : Accept ( listen_addr, session_mgr ),
      TCPChannel ( session_mgr->getEventLoopThread(), NULL, 1 )
{
    //listen();
    addToPoll();
}

TCPAccept::~TCPAccept()
{

}

void TCPAccept::listen()
{
    int reuse = 1;
    socket_->SetSocketOpt ( SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof ( int ) );
    socket_->Bind ( htonl(listen_address_.ip), listen_address_.port );
    socket_->Listen ( SOMAXCONN );

    LogDEBUG ( "listen ip = %s, port = %u", ConvertIp(htonl(listen_address_.ip)).c_str(), listen_address_.port );
}

void TCPAccept::handleRecv()
{
    struct sockaddr_in addr_remote;
    int conn_fd = socket_->Accept ( addr_remote );
    if ( conn_fd < 0 ) {
        //do something
        return;
    }
    InetAddress addr_caller;
    addr_caller.ip = ntohl ( addr_remote.sin_addr.s_addr );
    addr_caller.port = ntohs ( addr_remote.sin_port );

    static_cast<TCPSessionManager *> ( session_mgr_ )->resetOrCreateSession ( addr_caller, conn_fd );
}

void TCPAccept::setRemoteAddress ( InetAddress &addr )
{
    Channel::setRemoteAddress ( addr );
}

void TCPAccept::close()
{
    Channel::close();
}
