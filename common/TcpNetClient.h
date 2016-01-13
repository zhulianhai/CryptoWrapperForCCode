#ifndef __TCP_NETCLIENT_H__
#define __TCP_NETCLIENT_H__

#include "TcpParsePdu.h"
#include "INetClient.h"
#include "NetClientBase.h"
class TcpNetClient : public NetClientBase, public INetTcpEvent
{
public:
    TcpNetClient(INetEventLoop* net_event_loop, PduHandler* pdu_handler);
    virtual ~TcpNetClient(void);

    //net client base impl
    virtual bool Open(uint32_t remote_ip, uint16_t remote_port);
    virtual void Close();
    virtual bool IsClosed();
    virtual int SendRawData(char* data, uint32_t data_len);
    virtual int SendPduData(uint16_t cmd_id, char* data, uint32_t data_len);
    void GetLocalAddr( uint32_t &local_ip, uint16_t &local_port );
    virtual bool SendPduData(PDUSerialBase& pdu_serial);
    virtual int GetSocketFd() { return socket_fd_; }

    //tcpparsepud callback
    void OnHandleTcpPdu(uint16_t cmd_id, char* pdu_data, uint32_t data_len);
    int  Send(char* data, uint32_t data_len);

private:
    //implement INetTcpEvent
    virtual void OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context);
    virtual void OnSendReady(int socket_fd, void* context);
    virtual void OnClose(int socket_fd, void* context);
    virtual void OnConnected(int socket_fd, void* context);
    virtual bool isNeedSend(int socket_fd, void* context);
private:
    TcpParsePdu pdu_parser_;
    bool m_bConnected;
};

#endif

