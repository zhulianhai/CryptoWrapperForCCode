#include "TypeDef.h"
#include "PackDef.h"
#include "NetClient.h"
#include "RemoteTransport.h"

const int64_t s_nAuthPeriod = 1 * Utils::kNumMillisecsPerSec;
RemoteTransport::RemoteTransport(INetEventLoop* net_event_loop, PduHandler* pdu_handler)
: tcp_net_client_(net_event_loop, this), udp_net_client_(net_event_loop, this)
{
    auth_successed_ = false;
    m_nLastRecvTime = 0;
    m_nSchedulingTime = Utils::Time();
    m_nAuthTime = 0;
    net_event_loop_ = net_event_loop;
    pdu_handler_ = pdu_handler;
    auth_ip_ = 0;
    auth_port_ = 0;
    m_nHeartbeatSchedulingPeriod = 2 * Utils::kNumMillisecsPerSec;
    m_nTimeout = 4 * Utils::kNumMillisecsPerSec;
}

RemoteTransport::~RemoteTransport(void)
{
    DisConnectRelaySvr();
}

void RemoteTransport::SetHeartbeatPeriod(int nSeconds)
{
    if (nSeconds <= 0 || nSeconds > 100)
        return;

    m_nHeartbeatSchedulingPeriod = nSeconds * Utils::kNumMillisecsPerSec;
}

void RemoteTransport::SetTimeout(int nSeconds)
{
    if (nSeconds <= 0 || nSeconds > 100)
        return;

    m_nTimeout = nSeconds * Utils::kNumMillisecsPerSec;
}

void RemoteTransport::SetAuthInfo(uint32_t auth_ip, uint16_t auth_port)
{
    if (auth_ip == 0 || auth_port == 0)
    {
        InetAddress nAddr(auth_ip, auth_port);
        LogERR("!!!!!!!!!SetAuthInfo error. %s", nAddr.AsString().c_str());
        return;
    }
    
    if (auth_ip_ == auth_ip 
        && auth_port_ == auth_port)
    {
        return;
    }

    auth_successed_ = false;
    auth_ip_ = auth_ip;
    auth_port_ = auth_port;
}

void RemoteTransport::ConnectRelaySvr(uint32_t relay_ip, uint16_t relay_port, TransType trans_type)
{
    m_nLastRecvTime = Utils::Time();
    auth_successed_ = false;

    relay_ip_ = relay_ip;
    relay_port_ = relay_port;
    trans_type_ = trans_type;

    tcp_net_client_.Close();
    udp_net_client_.Close();

    if (TRANS_TCP == trans_type)
    {
        tcp_net_client_.Open(relay_ip, relay_port);
		SendAuthData();
        return;
    }

    //udp
    udp_net_client_.Open(relay_ip_, relay_port_);
    SendAuthData();
}

void RemoteTransport::DisConnectRelaySvr()
{
    if( !tcp_net_client_.IsClosed() )
    {
        tcp_net_client_.Close();
    }
    if( !udp_net_client_.IsClosed() )
    {
        udp_net_client_.Close();
    }
}

int RemoteTransport::SendRawData(char* data, uint16_t data_len)
{
	if (!auth_successed_)
	{
		SendAuthData();
	}
    m_nSchedulingTime = Utils::Time();
    return GetCurNetClient().SendRawData(data, data_len);
}

bool RemoteTransport::CheckTimeout()
{
    if ((Utils::Time() - m_nLastRecvTime) > m_nTimeout)
    {
        return true;
    }

    return false;
}

bool RemoteTransport::IsClosed()
{
    return GetCurNetClient().IsClosed();
}

bool RemoteTransport::IsReady()
{
    //??? when is ready??
    assert(false);
    return true;
}

void RemoteTransport::OnTimer()
{
    //1.check net
    if (0 == relay_ip_ || 0 == relay_port_ || GetCurNetClient().IsClosed())
    {
        return;
    }

    if (!auth_successed_)
    {
        SendAuthData(true);
    }
    else
    {
        int64_t nCurTime = Utils::Time();
        if ((nCurTime - m_nSchedulingTime) > m_nHeartbeatSchedulingPeriod)
        {
            SendHeadBeat();
            //LogINFO("Send head beat, socket id: %d", GetCurNetClient().GetSocketFd());
            m_nSchedulingTime = nCurTime;
        }
    }
}

void RemoteTransport::OnHandlePdu(uint16_t cmd_type, char* packet, uint16_t packet_len ,
                         uint32_t remote_ip, uint16_t remote_port)
{
    switch (cmd_type)
    {
    case PACK_CMD_ID_RELAYSVR_AUTH_RESP:
        {
            PACK_RELAYSVR_AUTH_RESP auth_resp;
            auth_resp.unserial(packet, packet_len);
            //it need auth again
            if( RESPONSE::NEED_REAUTH == auth_resp.success_ )
            {
                auth_successed_ = false;
				LogERR("Auth false.");
            }
            else if (RESPONSE::SUCC_AUTH == auth_resp.success_ )
            {
                auth_successed_ = true;
				LogERR("Auth ok.");
            }
            else
            {
                LogERR("Auth error.");
            }
        }
        break;
    case PACK_CMD_ID_HEARTBEAT_PACKET:
    	break;
    default:
        pdu_handler_->OnHandlePdu(cmd_type, packet, packet_len, remote_ip, remote_port);
    }
    
    m_nLastRecvTime = Utils::Time();
}

void RemoteTransport::SendAuthData(bool bInterval)
{
    if (0 == auth_ip_ || 0 == auth_port_)
    {
        return;
    }

    if (GetCurNetClient().IsClosed())
    {
        return;
    }

	int64_t nCurTime = Utils::Time();
    if (bInterval)
    {
        if ((nCurTime - m_nAuthTime) < s_nAuthPeriod)
        {
            return;
        }
    }
	
	m_nAuthTime = nCurTime;

    InetAddress nAddr(auth_ip_, auth_port_);
    LogINFO("Send auth data.%s", nAddr.AsString().c_str());
    std::string strAuthData = "Hello, this is auth data!";
    PACK_RELAYSVR_AUTH_REQ auth_req;
    auth_req.ip_ = auth_ip_;
    auth_req.port_ = auth_port_;
    memcpy(auth_req.data_, strAuthData.data(), strAuthData.length());
    auth_req.datalen_ = strAuthData.length();

    SendPdu(auth_req);
}

void RemoteTransport::SendHeadBeat()
{
    PACK_RELAYSVR_HEARTBEAT_PACKET heart_beat;
    heart_beat.heartbeat_tag_ = 0;
    SendPdu(heart_beat);
}

