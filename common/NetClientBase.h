#ifndef __NET_CLIENT_BASE_H__
#define __NET_CLIENT_BASE_H__
#include "ParsePdu.h"
class NetClientBase
{
public:
    NetClientBase(INetEventLoop* net_event_loop, PduHandler* pdu_handler)
        :net_event_loop_(net_event_loop), pdu_handler_(pdu_handler)
    {
        socket_fd_ = -1;
        remote_ip_ = 0;
        remote_port_ = 0;
        local_ip_ = 0;
        local_port_ = 0;
    }
    virtual ~NetClientBase(void){}
    virtual bool Open(uint32_t remote_ip, uint16_t remote_port) = 0;
    virtual void Close() = 0;
    virtual bool IsClosed() = 0;
    virtual int SendRawData(char* data, uint32_t data_len) = 0;
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len) = 0;
    virtual void GetLocalAddr( uint32_t &local_ip, uint16_t &local_port ) = 0;
    virtual int GetSocketFd() = 0;
    virtual bool SendPduData(PDUSerialBase& pdu_serial) = 0;

protected:
    INetEventLoop* net_event_loop_;
    PduHandler* pdu_handler_;
    int socket_fd_;
    uint32_t remote_ip_;
    uint16_t remote_port_;
    uint32_t local_ip_;
    uint16_t local_port_;
};

#endif


