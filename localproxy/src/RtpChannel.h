#ifndef RELAY_CHANNEL_H__
#define RELAY_CHANNEL_H__
#include <iostream>
#include <assert.h>

#include "TypeDef.h"
#include "ParsePdu.h"
#include "PackDef.h"
#include "NetClient.h"
#include "LocalProxyCommon.h"
#include "RemoteTransport.h"
#include "Buffer.h"

struct  DataBuffer {
    char data_[PACK_MAX_LEN];
    uint16_t data_len_;
    uint32_t ip_;
    uint16_t port_;
};

class RtpChannel : public PduHandler, public ILocalDataSink
{
public:
    RtpChannel( INetEventLoop* net_event_loop );
    virtual ~RtpChannel();
	int Open(const InetAddress &nAuthAddr, bool bUseTcp);
	int Bind(IRemoteDataSink* remote_data_sink, ILocalDataSink** local_data_sink);
    int Clear();
	int Close();
    void ConnectRelaySvr( uint32_t relay_ip, uint16_t relay_port, RemoteTransport::TransType trans_type );
    bool CheckTimeout();
    bool IsClosed();
    bool CheckUseTcp() const { return m_bUseTcp; }
	bool CheckDeatched() const { return m_bDeatched; };
    void OnTimer();

private:
    //pdu handler interface implement
    virtual void OnHandlePdu( uint16_t cmd_type, char* packet, uint16_t packet_len ,
        uint32_t remote_ip, uint16_t remote_port );
    virtual void OnSendPdu() {}

    //local data sink interface implement
    virtual int OnSendToPeer( char* data, uint32_t data_len );
    virtual void Detach();
    virtual bool GetRecvData(char* data, unsigned int &data_len, char* remote_ip, unsigned short &remote_port);

private:
    RemoteTransport* remote_trans_port_;
    IRemoteDataSink* remote_data_sink_;
    bool m_bActive;
    bool m_bUseTcp;
	bool m_bDeatched;

    Buffer<DataBuffer, CCriticalSection> buffer_;
    MemPool<DataBuffer, CCriticalSection> mempool_;
};
#endif