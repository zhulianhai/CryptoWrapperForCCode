#include <assert.h>
#include "SipRelay.h"
#include "TraceLog.h"
#include "PackDef.h"

SipRelay::SipRelay( INetEventLoop* net_event_loop, NetTestManager* pNetTestManager ) : UdpNetClient( net_event_loop, NULL )
{
    m_pRemoteTransport = new RemoteTransport(net_event_loop, this);
    m_pRemoteTransport->SetHeartbeatPeriod(4);
    m_pRemoteTransport->SetTimeout(15);
    m_bActive = false;
    m_bRestart = false;
	m_bFirstUse = true;
    m_nClientSourceIp = 0;
    m_nClientSourcePort = 0;
    m_pNetTestManager = pNetTestManager;
	Open(0, 0);
    m_bPrintMessage = false;
}

SipRelay::~SipRelay()
{
	if (!IsClosed())
	{
		Close();
	}
    if( NULL != m_pRemoteTransport )
    {
        SAFE_DELETE(m_pRemoteTransport);
    }
}

bool SipRelay::GetState()
{
    assert(NULL != m_pRemoteTransport);
    return m_bActive && m_pRemoteTransport->IsAuthSucessed();
}

void SipRelay::GetService( uint32_t &local_ip, uint16_t &local_port )
{
    GetLocalAddr(local_ip, local_port);
}

bool SipRelay::IsRestarted() 
{
    bool bRestart = m_bRestart;
    m_bRestart = false;
    return bRestart; 
}

void SipRelay::ResetChannel()
{
	Close();
	if (m_pRemoteTransport != NULL)
		m_pRemoteTransport->DisConnectRelaySvr();

	return;
}

void SipRelay::SetAuthData( uint32_t auth_ip, uint16_t auth_port )
{
    assert(NULL != m_pRemoteTransport);
    m_pRemoteTransport->SetAuthInfo(auth_ip, auth_port);
}

//recv from remote relay server 
void SipRelay::OnHandlePdu(uint16_t cmd_type, char* packet, uint16_t packet_len ,
        uint32_t remote_ip, uint16_t remote_port)
{
    if( cmd_type == PACK_CMD_ID_RTP_RTCP_PACKET )
    {
        if (m_bPrintMessage)
        {
            std::string strMessage(packet, packet_len);
            TraceLog::log().totalLog("localproxy_sip_message", "lenth:(%u),Recv:\n%s\n", packet_len, strMessage.c_str());
        }
        SendData(packet, packet_len, m_nClientSourceIp, m_nClientSourcePort);
    }
    else if( cmd_type == PACK_CMD_ID_RELAY_IPMAP_RESP )
    {
        PACK_RELAYSVR_RELAY_IPMAP_RESP nIpMapResp;
        if( !nIpMapResp.unserial(packet, packet_len) )
        {
            LogERR("Unserial sip relay ip map error.");
            return;
        }
        for( uint16_t i = 0; i < nIpMapResp.count_; i++ )
        {
            InetAddress nRemoteAddr(remote_ip, remote_port);
            std::string strLan = InetAddress(nIpMapResp.lan_ip_[i], 0).ToString();
            std::string strWlan = InetAddress(nIpMapResp.wan_ip_[i], 0).ToString();

            CCriticalAutoLock nAutoLock(m_nWlanLanLock);
            m_nWlanLanMap[strWlan] = strLan;
            LogINFO("Ip map:server %s, wanip %s, lanip %s", nRemoteAddr.AsString().c_str(), strWlan.c_str(), strLan.c_str());
        }
    }
    else
    {       
        InetAddress nRemoteAddr(remote_ip, remote_port);
        LogERR("Packet type(%d), from %s", cmd_type, nRemoteAddr.AsString().c_str());
    }
    return;
}

//recv from local client 
void SipRelay::OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context, 
        uint32_t remote_ip, uint16_t remote_port)
{
    assert(NULL != m_pRemoteTransport);

    m_nClientSourceIp = remote_ip;
    m_nClientSourcePort = remote_port;
    if( 0 > m_pRemoteTransport->SendRawData(data, data_len) )
    {
        LogERR("Send to relay server wrong..");
    }
    if (m_bPrintMessage)
    {
        std::string strMessage(data, data_len);
        TraceLog::log().totalLog("localproxy_sip_message", "lenth:(%u), Send:\n%s\n", data_len, strMessage.c_str());
    }
    return;
}

int SipRelay::TransRemoteIp( const std::string &strWlanIp, std::string &strLanIp )
{
    CCriticalAutoLock nAutoLock(m_nWlanLanLock);
    WlanLanMap::iterator iItr = m_nWlanLanMap.find(strWlanIp);
    if( iItr == m_nWlanLanMap.end() )
    {
        LogERR("It's not find the wlan ip(%s), ip map size is(%u)", strWlanIp.c_str(), m_nWlanLanMap.size());
        return Common::ERROR_NOT_EXSIT;
    }
    strLanIp = iItr->second;

    return Common::ERROR_SUCESS;
}

void SipRelay::OnTimer()
{
    assert(NULL != m_pRemoteTransport);

    //check udp server closed
    if( IsClosed() )
    {
        m_bActive = false;
        LogINFO("The udp server is closed, set up it.");
        if( !Open(0, 0) )
        {
            LogERR("Open local udp socket error.");
            return;
        }
    }
    
    //check remote closed
    if (m_pRemoteTransport->IsClosed() || m_pRemoteTransport->CheckTimeout())
    {
        m_bActive = false;
        Priority nPriority;
        InetAddress nNewServer;
        m_pNetTestManager->GetNewSvr(m_nSipRelayServer, Common::SIP, nPriority, nNewServer);
        if (nNewServer.Empty())
        {
            return;
        }
        m_nSipRelayServer = nNewServer;
        LogINFO("Sip relay need to restart.Find the best relay server:%s", m_nSipRelayServer.AsString().c_str());
        m_pRemoteTransport->ConnectRelaySvr(m_nSipRelayServer.ip_, m_nSipRelayServer.port_, RemoteTransport::TRANS_TCP);
    }

    if (!m_bActive || m_nWlanLanMap.empty())
    {
        SendIpMapPacket();
    }
    m_pRemoteTransport->OnTimer(); 

    if (!m_bActive && !m_bFirstUse)
	{
        m_bRestart = true;
	}

    if (m_bFirstUse)
    {
        m_bFirstUse = false;
    }

    m_bActive = true;
}

void SipRelay::SendIpMapPacket()
{
	LogINFO("It will send ip map packet.");
	PACK_RELAYSVR_RELAY_CONF_REQ nConReq;
	nConReq.req_id_ = 0;
	m_pRemoteTransport->SendPdu(nConReq);
}
