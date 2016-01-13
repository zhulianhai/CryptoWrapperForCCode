#include "Channel.h"
#include "EventLoopThread.h"
#include "Socket.h"
#include <sys/poll.h>
#include <errno.h>
#include <arpa/inet.h>
#include "TraceLog.h"
#include "Session.h"
#include "PackDef.h"
#include "ParsePdu.h"
//#include <assert.h>

using namespace NetCommon;

ChannelBase::ChannelBase ( EventLoopThread *loop )
    : peer_channel_ ( NULL ),
      loop_ ( loop ),
      last_recv_time_ ( now_sec() ),
      state_ ( INIT )
{
}

bool ChannelBase::checkTimeout ( uint64_t sec )
{
    if ( sec - last_recv_time_ < DEFAULTTIMEOUTSEC ) {
        return false;
    }

    if ( state_ == TIMEOUT
            || state_ == CLOSED ) {
        return true;
    }

    LogDEBUG ( "timeout remote = %s:%u, peer remote = %s:%u", ConvertIp(remote_addr_.ip).c_str(),\
        remote_addr_.port, ConvertIp(peer_channel_->getRemoteAddress().ip).c_str(), peer_channel_->getRemoteAddress().port );

    //state_ = TIMEOUT;

    //postTimeout();

    return true;
}

void ChannelBase::postTimeout()
{
    setState ( TIMEOUT );
    if ( loop_ != NULL ) {
        loop_->postTimeoutChannel ( this );
    } else {
        close();
    }
}

Channel::Channel ( EventLoopThread *loop, int mempool_size )
    : ChannelBase ( loop ),
      socket_ ( NULL ),
      mempool_ ( mempool_size ),
      events_ ( 0 ),
      post_events_ ( 0 ),
      sending_ ( false ),
      recving_ ( false )
{
}

Channel::~Channel()
{
    SAFE_DELETE ( socket_ );
}

void Channel::addToPoll()
{
    loop_->updateChannel ( this, EventLoopThread::O_ADD );
    setState ( RW );
}

void Channel::handleEvent()
{
    //eventHandling_ = true;
    if ( ( events_ & POLLHUP ) && ! ( events_ & POLLIN ) ) {
        handleClose();
    }

    if ( events_ & POLLNVAL ) {
        ;
    }

    if ( events_ & ( POLLERR | POLLNVAL ) ) {
        handleError();
    }
    if ( events_ & ( POLLIN | POLLPRI | POLLRDHUP ) ) {
        handleRecv();
    }
    if ( events_ & POLLOUT ) {
        handleSend();
    }
    //eventHandling_ = false;
}

void Channel::postRecv()
{
    post_events_ = POLLIN | POLLPRI;
    loop_->updateChannel ( this, EventLoopThread::O_MOD );
}

void Channel::postSend()
{
    post_events_ = POLLIN | POLLPRI | POLLOUT;
    loop_->updateChannel ( this, EventLoopThread::O_MOD );
}

void Channel::close()
{
    //deletepoll;
    //wait for socket finished??
    LogDEBUG ( "close channel remote addr = %s:%u", ConvertIp(remote_addr_.ip).c_str(), remote_addr_.port );
    loop_->updateChannel ( this, EventLoopThread::O_DEL );
    socket_->Close();
    /*if ( sending_ || recving_ ) {
        return;
    }*/

    ChannelBase::close();
}

UDPChannel::UDPChannel ( EventLoopThread *loop, int mempool_size )
    : Channel ( loop, mempool_size )
{
    socket_ = CreateNonblockSocket ( CREATE_UDP, -1 );
    int buf_size = 65535;
    socket_->SetSocketOpt ( SOL_SOCKET, SO_RCVBUF, ( void * ) &buf_size, sizeof ( buf_size ) );
    socket_->SetSocketOpt ( SOL_SOCKET, SO_SNDBUF, ( void * ) &buf_size, sizeof ( buf_size ) );
}

void UDPChannel::handleClose()
{
    //socket_.close();
}

void UDPChannel::handleRecv()
{
    recving_ = true;
    int data_len = 0;
    setLastRecvTime ( now_sec() );
    struct sockaddr_in *from_addr;
    Channel *peer_chl = static_cast<Channel *> ( getPeerChannel() );
    if ( NULL == peer_chl ) {
        LogERR ( "peer channel is null" );
        return;
    }

    DataBuffer *recv_data = peer_chl->getMemPool().GetMemery();
    data_len = socket_->RecvFrom ( recv_data->data_, PACK_MAX_LEN, from_addr );
    if ( data_len < 0 ) {
        peer_chl->getMemPool().Release ( recv_data );
        LogERR ( "socket recvfrom is failed, errno = %d, err desc:%s", errno, strerror ( errno ) );
        return;
    }

    recv_data->data_len_ = data_len;
    const InetAddress &addr = getPeerInetAddress();
    recv_data->ip_ = addr.ip;
    recv_data->port_ = addr.port;
    peer_chl->sendBuf ( recv_data );
    recving_ = false;
}

void UDPChannel::handleSend()
{
    sending_ = true;

    DataBuffer *sendto_data = NULL;
    sendto_data = buffer_.pop();
    while ( sendto_data ) {
        socket_->SetRemoteAddr ( sendto_data->ip_, sendto_data->port_ );
        if ( -1 == socket_->SendTo ( sendto_data->data_, sendto_data->data_len_ ) ) {
            //if(errno == EAGAIN || errno == EINTR){}
            buffer_.PushFront ( sendto_data );
            break;
        }
        mempool_.Release ( sendto_data );
        sendto_data = buffer_.pop();
    }

    if ( buffer_.isEmpty() ) {
        postRecv();
    }

    sending_ = false;
}

void UDPChannel::handleError()
{

}

TCPChannel::TCPChannel ( EventLoopThread *loop, Session *session, int mempool_size, int conn_fd )
    :Channel ( loop, mempool_size ),
     session_ ( session ),
     left_recvdata_size_ ( 0 ),
     left_senddata_size_ ( 0 )
{
    memset ( recv_buf_, 0, sizeof ( recv_buf_ ) );
    memset ( send_buf_, 0, sizeof ( send_buf_ ) );
    socket_ = CreateNonblockSocket ( CREATE_TCP, conn_fd );
    setSockOpt();
}

void TCPChannel::resetSocket ( int fd )
{
    left_recvdata_size_ = 0;
    left_senddata_size_ = 0;
    loop_->updateChannel ( this, EventLoopThread::O_DEL );
    getSocket()->resetSockFD ( fd );
    setSockOpt();
    addToPoll();
    //loop_->updateChannel ( this, EventLoopThread::O_ADD );
}

void TCPChannel::processData()
{
    char *data_start = recv_buf_;
    int proc_len = 0;
    ParsePdu parse;
    while ( left_recvdata_size_ >= 8 ) {
        if ( !parse.isOwnProtocol ( data_start, left_recvdata_size_ ) ) {
            struct in_addr in1;
            in1.s_addr = htonl ( remote_addr_.ip );
            LogERR ( "it's not own protocol data, remote ip = %s:%u", inet_ntoa ( in1 ), remote_addr_.port ); //ip and port
            left_recvdata_size_ = 0;
            break;
        } else {
            if ( ( proc_len = HandleRecvData::handlePduPack ( *this, remote_addr_, data_start, left_recvdata_size_ ) ) == 0 ) {
                break;
            }
        }
        left_recvdata_size_ -= proc_len;
        data_start += proc_len;
    }
    if ( left_recvdata_size_ != 0 && recv_buf_ != data_start ) {
        memcpy ( recv_buf_, data_start, left_recvdata_size_ );
    }
}

void TCPChannel::handleRecv()
{
    recving_ = true;
    do {
        setLastRecvTime ( now_sec() );
        int size = 0;
        if ( ( size = socket_->Recv ( recv_buf_ + left_recvdata_size_, BUF_SIZE - left_recvdata_size_ ) ) > 0 ) {
            left_recvdata_size_ += size;
        } else if ( size == 0 ) {
            left_recvdata_size_ = 0;
            handleClose();
            break;
        } else {
            if ( errno != EINTR && errno != EAGAIN ) {
                left_recvdata_size_ = 0;
                handleError();
            }
            break;
        }

        processData();
    } while ( 0 );
    recving_ = false;
}

void TCPChannel::handleSend()
{
    sending_ = true;

    int sent = -1;
    char *send_pos = send_buf_;
    //while ( data ) {
    do {
        if ( 0 == left_senddata_size_ ) {
            DataBuffer *data = buffer_.pop();
            if ( data == NULL ) {
                break;
            }
            uint16_t data_len = PACK_MAX_LEN;
            ParsePdu parse;
            if ( !parse.isOwnProtocol ( data->data_, data->data_len_ ) ) {
                PACK_RELAYSVR_RTP_RTCP_PACKET rtp_rtcp ( data->data_len_, data->data_ );
                if ( !parse.packPduBuf<PACK_RELAYSVR_RTP_RTCP_PACKET> ( rtp_rtcp, send_buf_, data_len ) ) {
                    break;
                }
            } else {
                memcpy ( send_buf_, data->data_, data->data_len_ );
                data_len = data->data_len_;
            }
            left_senddata_size_ = data_len;
            mempool_.Release ( data );
            send_pos = send_buf_;
        }
        if ( ( sent = socket_->Send ( send_pos, left_senddata_size_ ) ) < 0 ) {
            if ( errno != EINTR && errno != EAGAIN ) {
                if ( errno == EPIPE || errno == ECONNRESET ) { //any others?
                    left_senddata_size_ = 0;
                    if ( getPostEvents() & POLLOUT ) {
                        postRecv();
                    }
                    handleClose();
                    break;
                }
            }
            if ( send_buf_ != send_pos ) {
                memcpy ( send_buf_, send_pos, left_senddata_size_ );
            }
            break;
        }
        left_senddata_size_ -= sent;
        send_pos += sent;

    } while ( left_senddata_size_ >= 0 );

    if ( buffer_.isEmpty() ) {
        postRecv();
    }

    sending_ = false;
}

void TCPChannel::handleClose()
{
    last_recv_time_ -= DEFAULTTIMEOUTSEC;
    loop_->updateChannel ( this, EventLoopThread::O_DEL );
    socket_->Close();
    //setState(TIMEOUT);
    //loop_->postTimeoutChannel(this);
    //close();
}

void TCPChannel::handleError()
{
    if ( errno != EINTR && errno != EAGAIN ) {
        handleClose();
    }
}

void TCPChannel::doSuccessAuth ( InetAddress& addr_caller, InetAddress& addr_callee )
{
    LogDEBUG ( "tcp PACK_CMD_ID_RELAYSVR_AUTH_REQ caller ip = %s, port = %u, %p",
               ConvertIp(addr_caller.ip).c_str(), addr_caller.port, this );
    peer_channel_->setRemoteAddress ( addr_callee );

    //ResponseFUNC::responseAuth ( *this, addr_caller, RESPONSE::SUCC_AUTH );
}

void TCPChannel::doHeartBeat ( InetAddress& addr_caller )
{
    //peer_channel_->setRemoteAddress ( addr_caller );
    peer_channel_->setLastRecvTime ( now_sec() );
}

bool TCPChannel::doRTPRTCP ( char *data, uint16_t len )
{
    if ( peer_channel_->getRemoteAddress().ip == 0
            || peer_channel_->getRemoteAddress().port == 0 ) {
        return false;
    }

    PACK_RELAYSVR_RTP_RTCP_PACKET rtp_rtcp;
    rtp_rtcp.unserial ( data, len );
    static_cast<Channel *> ( peer_channel_ )->copyBufferAndSend ( rtp_rtcp.data_, rtp_rtcp.data_len_ );
    return true;
}

int HandleRecvData::handlePduPack ( Channel& chl, InetAddress& addr_caller, char* data, uint16_t len )
{
    int process_len = 0;
    //TODO:8 is own protocl lenght
    ParsePdu parse;
    uint16_t cmd_type = 0;
    char *data1 = NULL;
    uint16_t data1_len = 0;
    if ( !parse.parsePduHead ( data, len, cmd_type, data1, data1_len ) ) {
        LogERR ( "parse pdu error: buflen:%d, cmd_type:%d, data_len%d", len, cmd_type, data1_len );
        return len;
    }

    if ( len - 8 < data1_len ) {
        return 0;
    }

    switch ( cmd_type ) {
    case PACK_CMD_ID_RELAYSVR_AUTH_REQ: {
        process_len = data1_len + 8;

        PACK_RELAYSVR_AUTH_REQ auth_req;
        if ( !auth_req.unserial ( data1, data1_len ) ) {
            process_len = len;
            break;
            LogERR ( "unserial auth req error: data_len%d", data1_len );
        }

        if ( 0 == auth_req.ip_ || 0 == auth_req.port_ ) {
            responseAuth ( chl, addr_caller, RESPONSE::INVALID_ARG );
            break;
        }

        uint32_t lan_ip = INADDR_NONE;
        if (!IpWhiteList::GetIpWhiteList().GetLanIp(auth_req.ip_, lan_ip))
        {
            LogERR ( "ip white auth req error: req ip:%s, lan ip:%s", 
                ConvertIp(auth_req.ip_).c_str(), ConvertIp(lan_ip).c_str());
            responseAuth ( chl, addr_caller, RESPONSE::INVALID_ARG );
            break;
        }
        
        InetAddress addr_callee ( lan_ip, auth_req.port_ );
        chl.doSuccessAuth ( addr_caller, addr_callee );
        responseAuth ( chl, addr_caller, RESPONSE::SUCC_AUTH );
        break;
    }
    case PACK_CMD_ID_HEARTBEAT_PACKET: {
        process_len = data1_len + 8;
        chl.doHeartBeat ( addr_caller );
        responseHeartbeat ( chl, addr_caller );
        break;
    }
    case PACK_CMD_ID_RTP_RTCP_PACKET: {
        process_len = data1_len + 8;
        if ( !chl.doRTPRTCP ( data1, data1_len ) ) {
            responseAuth ( chl, addr_caller, RESPONSE::NEED_REAUTH );
        }
        break;
    }
    case PACK_CMD_ID_RELAY_CONF_REQ: {
        process_len = data1_len + 8;
        responseConf(chl, addr_caller);
        break;
    }
    default:
        //responseAuth ()
        LogDEBUG ( "INVALID DATA cmd_type = %d", cmd_type );
        process_len = data1_len + 8;
        responseAuth ( chl, addr_caller, RESPONSE::INVALID_ARG );
        break;
    }

    return process_len;
}

void HandleRecvData::responseAuth ( Channel& chl, InetAddress &addr, uint8_t success )
{
    PACK_RELAYSVR_AUTH_RESP auth_resp;
    auth_resp.success_ = success;

    DataBuffer *data_buf = NULL;
    data_buf = chl.getMemPool().GetMemery();
    data_buf->data_len_ = PACK_MAX_LEN;
    data_buf->ip_ = addr.ip;
    data_buf->port_ = addr.port;
    ParsePdu parseUDP;
    if ( !parseUDP.packPduBuf ( auth_resp, ( char * ) data_buf->data_, data_buf->data_len_ ) ) {
        LogERR ( "packet buf failed" );
        chl.getMemPool().Release ( data_buf );
        return;
    }
    chl.sendBuf ( data_buf );
}

void HandleRecvData::responseHeartbeat ( Channel& chl, InetAddress &addr )
{
    PACK_RELAYSVR_HEARTBEAT_PACKET resp;
    resp.heartbeat_tag_ = 1;

    DataBuffer *data_buf = NULL;
    data_buf = chl.getMemPool().GetMemery();
    data_buf->data_len_ = PACK_MAX_LEN;
    data_buf->ip_ = addr.ip;
    data_buf->port_ = addr.port;
    ParsePdu parseUDP;
    if ( !parseUDP.packPduBuf ( resp, ( char * ) data_buf->data_, data_buf->data_len_ ) ) {
        LogERR ( "packet buf failed" );
        chl.getMemPool().Release ( data_buf );
        return;
    }
    chl.sendBuf ( data_buf );
}

void HandleRecvData::responseConf ( Channel& chl, InetAddress& addr )
{
    PACK_RELAYSVR_RELAY_IPMAP_RESP resp;
    const std::map<uint32_t, uint32_t> &m = IpWhiteList::GetIpWhiteList().GetWanLanMap();
    std::map<uint32_t, uint32_t>::const_iterator it = m.begin();
    int c = 0;
    for (; it != m.end(); it++) {
        resp.wan_ip_[c] = it->first;
        resp.lan_ip_[c] = it->second;
        c++;
    }
    resp.count_ = c;
    
    DataBuffer *data_buf = NULL;
    data_buf = chl.getMemPool().GetMemery();
    data_buf->data_len_ = PACK_MAX_LEN;
    data_buf->ip_ = addr.ip;
    data_buf->port_ = addr.port;
    ParsePdu parseUDP;
    if ( !parseUDP.packPduBuf ( resp, ( char * ) data_buf->data_, data_buf->data_len_ ) ) {
        LogERR ( "packet buf failed" );
        chl.getMemPool().Release ( data_buf );
        return;
    }
    chl.sendBuf ( data_buf );
}
