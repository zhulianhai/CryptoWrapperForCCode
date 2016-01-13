#ifndef __SIP_RELAY_H__
#define __SIP_RELAY_H__

#include "Utils.h"
#include "NetClient.h"
#include "UdpNetClient.h"
#include "RemoteTransport.h"
#include "LocalProxyCommon.h"
#include "NetTestManager.h"

typedef std::map<std::string, std::string> WlanLanMap;

class SipRelay : public UdpNetClient, public PduHandler
{
public:
    SipRelay( INetEventLoop* net_event_loop, NetTestManager* pNetTestManager );
    virtual ~SipRelay(void);

    //set configuration api
    void SetAuthData( uint32_t auth_ip, uint16_t auth_port );

    //state api
    void GetService( uint32_t &local_ip, uint16_t &local_port );
    bool GetState();
    bool IsRestarted();
	void ResetChannel();
    
    //other
    int TransRemoteIp( const std::string &strWlanIp, std::string &strLanIp );
    void OnTimer();
    void EnablePrintMessage(bool bPrintMessage) { m_bPrintMessage = bPrintMessage; }

private:
    //pdu handler interface impl
    virtual void OnHandlePdu(uint16_t cmd_type, char* packet, uint16_t packet_len ,
        uint32_t remote_ip, uint16_t remote_port);
    virtual void OnSendPdu(){}

    //udp net client, local 
    virtual void OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context, 
        uint32_t remote_ip, uint16_t remote_port);
	void SendIpMapPacket();

private:
    RemoteTransport* m_pRemoteTransport;

    //state
    bool m_bActive;
    bool m_bRestart;
	bool m_bFirstUse;

    //transport ip map
    CCriticalSection m_nWlanLanLock;
    WlanLanMap m_nWlanLanMap;

    //client ip and port.for port self learning
    uint32_t m_nClientSourceIp;
    uint16_t m_nClientSourcePort;

    NetTestManager* m_pNetTestManager;
    InetAddress m_nSipRelayServer;

    bool m_bPrintMessage;
};

#endif

