#ifndef __UDP_NETCLIENT_H__
#define __UDP_NETCLIENT_H__
#include "ParsePdu.h"
#include "INetClient.h"
#include "NetClientBase.h"
class UdpNetClient : public NetClientBase, public INetUdpEvent
{
public:
    UdpNetClient(INetEventLoop* net_event_loop, PduHandler* pdu_handler);
    virtual ~UdpNetClient(void);

    //net client base impl
    virtual bool Open(uint32_t remote_ip, uint16_t remote_port);
    virtual void Close();
    virtual bool IsClosed();
    virtual int SendRawData(char* data, uint32_t data_len);
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len);
    virtual void GetLocalAddr( uint32_t &local_ip, uint16_t &local_port );
    virtual bool SendPduData(PDUSerialBase& pdu_serial);
    virtual int GetSocketFd() { return socket_fd_; }

    int SendData(char* data, uint32_t data_len,
        uint32_t remote_ip, uint16_t remote_port);
    int SendPduData(PDUSerialBase& pdu_serial, uint32_t remote_ip, uint16_t remote_port);
private:
    virtual void OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context, 
        uint32_t remote_ip, uint16_t remote_port);
    virtual void OnClose(int socket_fd, void* context);
};
#endif

