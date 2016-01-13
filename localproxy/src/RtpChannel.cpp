#include "RtpChannel.h"
#include "Utils.h"
#include "TraceLog.h"

RtpChannel::RtpChannel( INetEventLoop* net_event_loop )
{
    remote_data_sink_ = NULL;
    remote_trans_port_ = new RemoteTransport(net_event_loop, this);
    m_bUseTcp = false;
    m_bActive = false;
	m_bDeatched = false;
}

RtpChannel::~RtpChannel()
{
    SAFE_DELETE(remote_trans_port_);
}

int RtpChannel::Open(const InetAddress &nAuthAddr, bool bUseTcp)
{
	m_bUseTcp = bUseTcp;
	m_bActive = false;
	remote_trans_port_->SetAuthInfo(nAuthAddr.ip_, nAuthAddr.port_);
    return Common::ERROR_SUCESS;
}

int RtpChannel::Bind(IRemoteDataSink* remote_data_sink, ILocalDataSink** local_data_sink)
{
	ILocalDataSink *pLocalDataSink = this;
	(*local_data_sink) = pLocalDataSink;
	remote_data_sink_ = remote_data_sink;
	return Common::ERROR_SUCESS;
}

int RtpChannel::Clear()
{
	remote_data_sink_ = NULL;
	return Common::ERROR_SUCESS;
}

int RtpChannel::Close()
{
    m_bActive = false;
	Clear();
    remote_trans_port_->DisConnectRelaySvr();
    return Common::ERROR_SUCESS;
}

void RtpChannel::ConnectRelaySvr( uint32_t relay_ip, uint16_t relay_port, RemoteTransport::TransType trans_type )
{
    assert(NULL != remote_trans_port_);
    remote_trans_port_->ConnectRelaySvr(relay_ip, relay_port, trans_type);
    m_bActive = true;
}

bool RtpChannel::CheckTimeout()
{
    return remote_trans_port_->CheckTimeout();
}

bool RtpChannel::IsClosed()
{
    return remote_trans_port_->IsClosed();
}

void RtpChannel::OnTimer()
{
    remote_trans_port_->OnTimer();
}

//pdu handler interface implement
void RtpChannel::OnHandlePdu( uint16_t cmd_type, char* packet, uint16_t packet_len ,
    uint32_t remote_ip, uint16_t remote_port )
{
    if( NULL == remote_data_sink_ )
		return;
    
	if (packet_len > PACK_MAX_LEN)
    {
        LogERR("Rtp channel OnHandlePdu is too long:%u", packet_len);
        return;
    }
		
    if( !m_bActive )
    {
        LogERR("Rtp channel is not active. Handle pdu is wrong.");
        return;
    }
    uint32_t auth_ip = 0;
    uint16_t auth_port = 0;
    remote_trans_port_->GetAuthInfo(auth_ip, auth_port);
    InetAddress nAuthAddr(auth_ip, auth_port);
    DataBuffer *recv_data = mempool_.GetMemery();
    if( !memcpy(recv_data->data_, packet, packet_len) )
    {
        LogERR("memcpy error");
        return;
    }
    recv_data->data_len_ = packet_len;
    recv_data->ip_ = nAuthAddr.ip_;
    recv_data->port_ = nAuthAddr.port_;
    buffer_.Push(recv_data);
    remote_data_sink_->OnSendToLocalEvent(0);
}

//local data sink interface implement
int RtpChannel::OnSendToPeer( char* data, uint32_t data_len )
{
    assert(NULL != remote_trans_port_);
    if( !m_bActive )
    {
        LogERR("Rtp channel is not active. send to peer is wrong.");
        return -1;
    }
        
    int nResult = remote_trans_port_->SendRawData(data, data_len);
    if( nResult <= 0 )
    {
		LogINFO("send error:%u, nResult:%d", errno, nResult);
    }
    return nResult;
}

bool RtpChannel::GetRecvData(char* data, unsigned int &data_len, char* remote_ip, unsigned short &remote_port)
{
    DataBuffer *sendto_data = NULL;
    sendto_data = buffer_.pop();
    if( sendto_data == NULL )
        return false;

	if (sendto_data->data_len_ > PACK_MAX_LEN)
	{
		LogERR("GetRecvData, true data len is too long:%u.", data_len, sendto_data->data_len_);
		return false;
	}

    if( !memcpy(data, sendto_data->data_, sendto_data->data_len_) )
    {
        mempool_.Release(sendto_data);
        return false;
    }
    data_len = sendto_data->data_len_;
    InetAddress nAddr(sendto_data->ip_, sendto_data->port_);
    std::string strAddress = nAddr.ToString();
    memcpy(remote_ip, strAddress.c_str(), strAddress.size());
    remote_port = sendto_data->port_;
    mempool_.Release(sendto_data);
    return true;
}

void RtpChannel::Detach()
{
    m_bActive = false;
	m_bDeatched = true;
}
