#ifndef __REMOTE_TRANSPORT_H__
#define __REMOTE_TRANSPORT_H__

#include "TraceLog.h"
#include "Utils.h"
#include "UdpNetClient.h"
#include "TcpNetClient.h"

class RemoteTransport : public PduHandler
{
public:
    typedef enum tagTransType
    {
        TRANS_UDP,
        TRANS_TCP
    }TransType;

    RemoteTransport(INetEventLoop* net_event_loop, PduHandler* pdu_handler);
    ~RemoteTransport(void);

    void SetAuthInfo(uint32_t auth_ip, uint16_t auth_port);
    void GetAuthInfo(uint32_t &auth_ip, uint16_t &auth_port)
    {
        auth_ip = auth_ip_;
        auth_port = auth_port_;
    }
    void SetHeartbeatPeriod(int nSeconds);
    void SetTimeout(int nSeconds);
    void ConnectRelaySvr(uint32_t relay_ip, uint16_t relay_port, TransType trans_type);
    void DisConnectRelaySvr();
    int SendRawData(char* data, uint16_t data_len);

    template<class Pdu>
    bool SendPdu(Pdu& pdu)
    {
        PDUSerial<Pdu> pdu_serial(pdu);
        m_nSchedulingTime = Utils::Time();
        return GetCurNetClient().SendPduData(pdu_serial);
    }

    void OnTimer();
    bool CheckTimeout();
    bool IsClosed();
    bool IsReady();
    bool IsAuthSucessed() { return auth_successed_; }

private:
    //pdu handler impl
    virtual void OnHandlePdu(uint16_t cmd_type, char* packet, uint16_t packet_len ,
        uint32_t remote_ip, uint16_t remote_port);
    virtual void OnSendPdu() {}

private:
    void SendAuthData(bool bInterval = false);
    void SendHeadBeat();
    NetClientBase& GetCurNetClient()
    {
        if(TRANS_TCP == trans_type_)
        {
            return tcp_net_client_;
        }
        return udp_net_client_;
    }
private:
    PduHandler* pdu_handler_;
    TcpNetClient tcp_net_client_;
    UdpNetClient udp_net_client_;
    uint32_t relay_ip_;
    uint16_t relay_port_;
    TransType trans_type_;
    uint32_t auth_ip_;
    uint16_t auth_port_;
    bool auth_successed_;
    int64_t m_nLastRecvTime;
    int64_t m_nSchedulingTime;
    int64_t m_nAuthTime;
    INetEventLoop* net_event_loop_;
    int64_t m_nHeartbeatSchedulingPeriod;
    int64_t m_nTimeout;
};

#endif

