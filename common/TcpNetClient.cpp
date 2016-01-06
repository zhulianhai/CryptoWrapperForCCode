#include "TraceLog.h"
#include "TcpNetClient.h"
#include "PackDef.h"
TcpNetClient::TcpNetClient(INetEventLoop* net_event_loop, PduHandler* pdu_handler)
:NetClientBase(net_event_loop, pdu_handler)
{
    m_bConnected = false;
}

TcpNetClient::~TcpNetClient(void)
{
}

bool TcpNetClient::Open(uint32_t remote_ip, uint16_t remote_port)
{
    if (-1 != socket_fd_)
    {
        LogERR("Create tcp socket error.socket_fd_ is no nul:%d", socket_fd_);
        return false;
    }

    pdu_parser_.Reset();
    m_bConnected = false;
    local_ip_ = 0;
    local_port_ = 0;
    socket_fd_ = net_event_loop_->CreatTcpSocket();
    if( 0 >= socket_fd_ )
    {
        LogERR("Create tcp socket error.");
        return false;
    }
    if( 0 > net_event_loop_->Bind(socket_fd_, local_ip_, local_port_) )
    {
        net_event_loop_->CloseSocket(socket_fd_);
        socket_fd_ = -1;
        LogERR("Bind tcp socket error.");
        return false;
    }
    INetTcpEvent* pTcp = this;
    net_event_loop_->SetSocketContext(socket_fd_, pTcp, 0);
    if (net_event_loop_->Connect(socket_fd_, pTcp, remote_ip, remote_port) < 0)
    {
        Close();
        LogERR("Tcp connect socket error.");
        return false;
    }

    remote_ip_ = remote_ip;
    remote_port_ = remote_port;
    return true;
}

void TcpNetClient::Close()
{
    if( -1 == socket_fd_ )
    {
        return;
    }

    net_event_loop_->CloseSocket(socket_fd_);
    socket_fd_ = -1;
    m_bConnected = false;
    local_ip_ = 0;
    local_port_ = 0;
}

bool TcpNetClient::IsClosed()
{
    return (-1 == socket_fd_);
}

void TcpNetClient::GetLocalAddr( uint32_t &local_ip, uint16_t &local_port )
{
    if( -1 == socket_fd_ )
    {
        local_ip = 0;
        local_port = 0;
        return;
    }

    local_ip = local_ip_;
    local_port = local_port_;
}

int TcpNetClient::SendRawData(char* data, uint32_t data_len)
{
    if(pdu_parser_.SendTcpPdu(data, data_len, PACK_CMD_ID_RTP_RTCP_PACKET, this))
    {
        return data_len;
    }
    return 0;
}

int TcpNetClient::SendPduData(uint16_t cmd_id, char* data, uint32_t data_len)
{
    if(pdu_parser_.SendTcpPdu(data, data_len, cmd_id, this))
    {
        return data_len;
    }
    return 0;
}

bool TcpNetClient::SendPduData(PDUSerialBase& pdu_serial)
{
    return pdu_parser_.SendTcpPdu(pdu_serial, this);
}

//implement INetTcpEvent
void TcpNetClient::OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context)
{
    if (socket_fd != socket_fd_)
    {
        return;
    }

    pdu_parser_.ParseTcpPdu(data, data_len, this);
}

void TcpNetClient::OnSendReady(int socket_fd, void* context)
{
    if (socket_fd != socket_fd_)
    {
        return;
    }

    if(0 == pdu_parser_.Flush(this))
    {
        //send buf is empty
        pdu_handler_->OnSendPdu();
    }
}

bool TcpNetClient::isNeedSend(int socket_fd, void* context)
{
    if (socket_fd != socket_fd_)
    {
        return false;
    }

    return pdu_parser_.isNeedSend();

}
void TcpNetClient::OnClose(int socket_fd, void* context)
{
    if (socket_fd != socket_fd_)
    {
        return;
    }
	Close();
	LogERR("Recv tcp on closed event...");
}

void TcpNetClient::OnHandleTcpPdu(uint16_t cmd_id, char* pdu_data, uint32_t data_len)
{
    pdu_handler_->OnHandlePdu(cmd_id, pdu_data, data_len, remote_ip_, remote_port_);
}

void TcpNetClient::OnConnected(int socket_fd, void* context)
{
    m_bConnected = true;
}

int  TcpNetClient::Send(char* data, uint32_t data_len)
{
    if( !m_bConnected )
    {
        //LogERR("Tcp is not ready.Please wait...");
        return -1;
    }

    return net_event_loop_->SendTcpData(socket_fd_, data, data_len);
}


