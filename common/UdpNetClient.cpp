#include "UdpNetClient.h"
#include "TraceLog.h"

UdpNetClient::UdpNetClient( INetEventLoop* net_event_loop, PduHandler* pdu_handler )
:NetClientBase(net_event_loop, pdu_handler)
{
    socket_fd_ = -1;
    local_ip_ = 0;
    local_port_ = 0;
}

UdpNetClient::~UdpNetClient(void)
{
    
}

bool UdpNetClient::Open(uint32_t remote_ip, uint16_t remote_port)
{
    socket_fd_ = net_event_loop_->CreatUdpSocket();
    if( -1 == socket_fd_ )
    {
        LogERR("Open socket error, invalid socket");
        return false;
    }

    local_ip_ = 0;
    local_port_ = 0;
    if( 0 > net_event_loop_->Bind(socket_fd_, local_ip_, local_port_) )
    {
        net_event_loop_->CloseSocket(socket_fd_);
        socket_fd_ = -1;
        LogERR("udp open error, bind error.");
        return false;
    }
    INetUdpEvent* pUdp = this;
    net_event_loop_->SetSocketContext(socket_fd_, pUdp, 1);

    remote_ip_ = remote_ip;
    remote_port_ = remote_port;

    return true;
}

void UdpNetClient::Close()
{
    if( -1 != socket_fd_ )
    {
        net_event_loop_->CloseSocket(socket_fd_);
        socket_fd_ = -1;
    }

    return;
}

bool UdpNetClient::IsClosed()
{
    return (-1 == socket_fd_);
}

int UdpNetClient::SendRawData(char* data, uint32_t data_len)
{
    return SendData(data, data_len, remote_ip_, remote_port_);
}

int UdpNetClient::SendPduData(uint16_t cmd_id, char* data, uint32_t data_len)
{
    ParsePdu parse;

    char nBuffer[PACK_MAX_LEN] = {0};
    uint16_t nLength = PACK_MAX_LEN;
    int len = parse.packPduBuf(data, data_len, cmd_id, nBuffer);
    return SendRawData(nBuffer, len);
}

bool UdpNetClient::SendPduData(PDUSerialBase& pdu_serial)
{
    ParsePdu parse;

    char nBuffer[PACK_MAX_LEN] = {0};
    uint16_t nLength = PACK_MAX_LEN;
    if(!parse.packPduBuf(pdu_serial, nBuffer, nLength))
    {
        return false;
    }

    return (SendData(nBuffer, nLength, remote_ip_, remote_port_) > 0);
}

int UdpNetClient::SendPduData(PDUSerialBase& pdu_serial, uint32_t remote_ip, uint16_t remote_port)
{
    ParsePdu parse;

    char nBuffer[PACK_MAX_LEN] = {0};
    uint16_t nLength = PACK_MAX_LEN;
    if(!parse.packPduBuf(pdu_serial, nBuffer, nLength))
    {
        return 0;
    }
    return SendData(nBuffer, nLength, remote_ip, remote_port);

}
int UdpNetClient::SendData( char* data, uint32_t data_len,
              uint32_t remote_ip, uint16_t remote_port )
{
    if( -1 == socket_fd_ )
    {
        LogERR("Send udp data error, invalid socket");
        return 0;
    }

    return net_event_loop_->SendUdpData(socket_fd_, data, data_len, remote_ip, remote_port);
}

void UdpNetClient::GetLocalAddr( uint32_t &local_ip, uint16_t &local_port )
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

void UdpNetClient::OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context, 
                          uint32_t remote_ip, uint16_t remote_port)
{
    remote_ip_ = remote_ip;
    remote_port_ = remote_port;
    uint16_t nType = 0;
    char *pData  = NULL;
    uint16_t nDataLen = 0;
    ParsePdu nParsePdu;
    if( !nParsePdu.parsePduHead(data, data_len, nType, pData, nDataLen) )
    {
        pdu_handler_->OnHandlePdu(0, data, data_len, remote_ip_, remote_port_);
    }
    else
    {
        pdu_handler_->OnHandlePdu(nType, pData, nDataLen, remote_ip_, remote_port_);
    }
}

void UdpNetClient::OnClose(int socket_fd, void* context)
{
    Close();
	LogERR("Recv udp on closed event...");
}
