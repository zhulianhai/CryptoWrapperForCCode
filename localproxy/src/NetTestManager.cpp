#include <stdlib.h>
#include "NetTestManager.h"

NetTest::NetTest(NetTestManager* net_test_mgr, uint32_t test_type)
: net_test_mgr_(net_test_mgr), test_type_(test_type)
{

}

void NetTest::OnHandlePdu(uint16_t cmd_type, char* packet, uint16_t packet_len ,
                          uint32_t remote_ip, uint16_t remote_port)
{
    if (PACK_CMD_ID_HEARTBEAT_PACKET != cmd_type)
    {
        LogERR("unkown packet!");
        return;
    }

    PACK_RELAYSVR_HEARTBEAT_PACKET nResp;
    if( !nResp.unserial(packet, packet_len) )
    {
        LogERR("Parse heartbeat error!");
        return;
    }
    uint32_t rtt = Utils::Time() - nResp.reserved_;
    InetAddress nInetAddr(remote_ip, remote_port);
    net_test_mgr_->UpdateTestResult(nInetAddr, test_type_, rtt);
}

UdpNetTest::UdpNetTest(NetTestManager* net_test_mgr, INetEventLoop* net_event_loop)
:NetTest(net_test_mgr, Common::UDP), udp_net_client_(net_event_loop, this)
{
}

bool UdpNetTest::StartTest(ResultMap& test_dest)
{
    if (udp_net_client_.IsClosed())
    {
        udp_net_client_.Open(0, 0);
    }
    if (udp_net_client_.IsClosed())
    {
        return false;
    }

    PACK_RELAYSVR_HEARTBEAT_PACKET nHeartReq;
    nHeartReq.heartbeat_tag_ = RESPONSE::HEARTBEAT_REQ;
    nHeartReq.reserved_ = Utils::Time();
    PDUSerial<PACK_RELAYSVR_HEARTBEAT_PACKET> pdu_serial(nHeartReq);
    ResultMap::iterator it = test_dest.begin();
    for (; it != test_dest.end(); it++)
    {
        udp_net_client_.SendPduData(pdu_serial, it->first.ip_, it->first.port_);
    }
    return true;
}

void UdpNetTest::StopTest()
{
    if( !udp_net_client_.IsClosed() )
    {
        udp_net_client_.Close();
    }
}

TcpNetTest::TcpNetTest(NetTestManager* net_test_mgr)
:NetTest(net_test_mgr, Common::TCP)
{

}

bool TcpNetTest::StartTest(ResultMap& test_dest)
{
    PACK_RELAYSVR_HEARTBEAT_PACKET nHeartReq;
    nHeartReq.heartbeat_tag_ = RESPONSE::HEARTBEAT_REQ;
    nHeartReq.reserved_ = Utils::Time();
    PDUSerial<PACK_RELAYSVR_HEARTBEAT_PACKET> pdu_serial(nHeartReq);

    TcpNetClient* net_client = NULL;
    bool has_succ = false;
    ResultMap::iterator it = test_dest.begin();
    for (; it != test_dest.end(); it++)
    {
        net_client = GetTcpNetClient(it->first);
        assert(net_client);

        if (net_client->IsClosed())
        {
            net_client->Open(it->first.ip_, it->first.port_);
        }
        if (net_client->IsClosed())
        {
            //LogERR("open error");
            continue;
        }

        net_client->SendPduData(pdu_serial);
        has_succ = true;
    }
    return has_succ;
}

void TcpNetTest::StopTest()
{
    CCriticalAutoLock guard(tcp_cli_map_lock_);
    std::map<InetAddress, TcpNetClient*>::iterator it = tcp_cli_map_.begin();
    for( ; it != tcp_cli_map_.end(); it++ )
    {
        TcpNetClient* tcp_net_cli = it->second;
        if( !tcp_net_cli->IsClosed() )
        {
            tcp_net_cli->Close();
        }
    }
}

TcpNetClient* TcpNetTest::GetTcpNetClient(InetAddress const& addr)
{
    TcpNetClient* tcp_net_cli = NULL;
    CCriticalAutoLock guard(tcp_cli_map_lock_);
    std::map<InetAddress, TcpNetClient*>::iterator it = tcp_cli_map_.find(addr);
    if (it != tcp_cli_map_.end())
    {
        tcp_net_cli = it->second;
    }
    
    if (NULL == tcp_net_cli)
    {
        tcp_net_cli = new TcpNetClient(net_test_mgr_->GetEventLoop(), this);
        tcp_cli_map_[addr] = tcp_net_cli;
    }
    return tcp_net_cli;
}

NetTestManager::NetTestManager( INetEventLoop* net_event_loop )
:udp_net_test_(this, net_event_loop), tcp_net_test_(this)
{
    assert(NULL != net_event_loop);
    net_event_loop_ = net_event_loop;
    m_nLastTestTime = 0;
    m_bTestActive = false;
}

NetTestManager::~NetTestManager()
{

}

void NetTestManager::OnTimer()
{
    uint32_t now = Utils::Time();
    if (now - m_nLastTestTime > s_nTestInterval)
    {
        StartTest(true);
    }
    
    if( m_bTestActive && (now - m_nLastTestTime > s_nTestPeriod) )
    {
        StopTest();
    }

}

void NetTestManager::GetNewSvr(const InetAddress& cur_using_svr, Common::RelayType relay_type, Priority& nPriority, InetAddress& new_svr)
{
    result_map_lock_.Lock();
    ResultMap& svr_map = relay_type == Common::SIP ? m_nSipResult : m_nRtpResult;

	//save and reset the cur result
	ResultMap::iterator iItr = svr_map.find(cur_using_svr);
	if (iItr != svr_map.end() && svr_map.size() > 1)
	{
		Attribute &nCurAttr = iItr->second;
		//save the default value
		Priority nTempPriority = nCurAttr.GetPriority();
		if (nTempPriority != USE_NONE)
		{
			new_svr = cur_using_svr;
			nPriority = nTempPriority;
		}

		nCurAttr.Reset();
	}

	InetAddress nPriorityAll;
	InetAddress nPriorityTcp;
	InetAddress nPriorityUdp;
	uint32_t nPriorityAllUpdateTime = Utils::Time();
	uint32_t nPriorityTcpUpdateTime = Utils::Time();
	uint32_t nPriorityUdpUpdateTime = Utils::Time();
	
	//get the last update ip.
	for (ResultMap::iterator iItr = svr_map.begin(); iItr != svr_map.end(); iItr++)
    {
        InetAddress nItrAddr = iItr->first;
        Attribute nAttribute = iItr->second;
		Priority nPriority = nAttribute.GetPriority();
		if (nPriority  == USE_ALL)
        {
			if (nAttribute.update_time_ < nPriorityAllUpdateTime)
			{
				nPriorityAll = nItrAddr;
				nPriorityAllUpdateTime = nAttribute.update_time_;
			}
            continue;
        }
		else if (nPriority == USE_TCP)
        {
			if (nAttribute.update_time_ < nPriorityTcpUpdateTime)
			{
				nPriorityTcp = nItrAddr;
				nPriorityTcpUpdateTime = nAttribute.update_time_;
			}
            continue;
        }
		else if (nPriority == USE_UDP)
        {
			if (nAttribute.update_time_ < nPriorityUdpUpdateTime)
			{
				nPriorityUdp = nItrAddr;
				nPriorityUdpUpdateTime = nAttribute.update_time_;
			}
            continue;
        }
    }

    result_map_lock_.UnLock();

	bool bUseTcpFirst = (relay_type == Common::SIP) ? true : false;
	if (bUseTcpFirst)
	{
		if (!nPriorityAll.Empty())
		{
			LogINFO("GetNewSvr USE_ALL: %s---", nPriorityAll.AsString().c_str());
			new_svr = nPriorityAll;
			nPriority = USE_ALL;
		}
		//tcp first
		else if (!nPriorityTcp.Empty())
		{
			LogINFO("GetNewSvr USE_TCP: %s---", nPriorityTcp.AsString().c_str());
			new_svr = nPriorityTcp;
			nPriority = USE_TCP;
		}
		else if (!nPriorityUdp.Empty())
		{
			LogINFO("GetNewSvr USE_UDP: %s---", nPriorityUdp.AsString().c_str());
			new_svr = nPriorityUdp;
			nPriority = USE_UDP;
		}
	}
	else
	{
		if (!nPriorityAll.Empty())
		{
			LogINFO("GetNewSvr USE_ALL: %s---", nPriorityAll.AsString().c_str());
			new_svr = nPriorityAll;
			nPriority = USE_ALL;
		}
		//udp first
		else if (!nPriorityUdp.Empty())
		{
			LogINFO("GetNewSvr USE_UDP: %s---", nPriorityUdp.AsString().c_str());
			new_svr = nPriorityUdp;
			nPriority = USE_UDP;
		}
		else if (!nPriorityTcp.Empty())
		{
			LogINFO("GetNewSvr USE_TCP: %s---", nPriorityTcp.AsString().c_str());
			new_svr = nPriorityTcp;
			nPriority = USE_TCP;
		}
	}

    StartTest();
}

void NetTestManager::StartTest( bool bForced )
{
    CCriticalAutoLock nAutoLock(start_test_lock_);
    if (!bForced && (Utils::Time() - m_nLastTestTime < s_nTestPeriod))
    {
        //LogINFO("Ignore this test, because it have be started within 5 seconds ");
        return;
    }
	LogINFO("Do Start Test now... ");
    m_nLastTestTime = Utils::Time();

    ResultMap udp_test_dest;
    ResultMap tcp_test_dest;
    bool udp_succ = false;
    bool tcp_succ = false;

    result_map_lock_.Lock();
    //udp
    udp_test_dest.insert(m_nRtpResult.begin(), m_nRtpResult.end());
    //tcp
    tcp_test_dest.insert(m_nRtpResult.begin(), m_nRtpResult.end());
    tcp_test_dest.insert(m_nSipResult.begin(), m_nSipResult.end());
	//reset result
    for( ResultMap::iterator iItr = m_nRtpResult.begin(); iItr != m_nRtpResult.end(); iItr++ )
    {
        InetAddress nAddr = iItr->first;
        //LogINFO("---Start test rtp %s---", nAddr.AsString().c_str());
        Attribute& svr_attr = iItr->second;
        if( !bForced )
        {
            svr_attr.Reset();
        }
    }
    for( ResultMap::iterator iItr = m_nSipResult.begin(); iItr != m_nSipResult.end(); iItr++ )
    {
        InetAddress nAddr = iItr->first;
        //LogINFO("---Start test sip %s---", nAddr.AsString().c_str());
        Attribute& svr_attr = iItr->second;
        if( !bForced )
        {
            svr_attr.Reset();
        }
    }
    result_map_lock_.UnLock();

	if (udp_test_dest.empty() && tcp_test_dest.empty())
	{
		LogERR("The test server is emyty, do not test.");
		return;
	}

    udp_succ = udp_net_test_.StartTest(udp_test_dest);
    tcp_succ = tcp_net_test_.StartTest(tcp_test_dest);

    if (udp_succ || tcp_succ)
    {
        m_bTestActive = true;
    }
}

void NetTestManager::StopTest()
{
    if (!m_bTestActive)
    {
        return;
    }
    udp_net_test_.StopTest();
    tcp_net_test_.StopTest();
    m_bTestActive = false;
}

int NetTestManager::AppendServer( const char* pRelayServers, Common::RelayType nType )
{
    //parse servers
    Utils::StringArray nValues;
    std::string strRelayServers(pRelayServers);
    Utils::String::Split(strRelayServers, nValues, "/");
    
    Attribute svr_attr;
    result_map_lock_.Lock();
    ResultMap& result_map = Common::SIP == nType ? m_nSipResult : m_nRtpResult;
	result_map.clear();
    for( size_t i = 0; i < nValues.size(); i++ )
    {
        Utils::StringArray nAddrIpPort;
        Utils::String::Split(nValues[i], nAddrIpPort, ":");
        if( nAddrIpPort.size() == 2 )
        {
            std::string strIp = nAddrIpPort[0];
            uint16_t nPort = (uint16_t)atoi(nAddrIpPort[1].c_str());
            InetAddress nAddr(strIp, nPort);
            //if it's a new ip, push a new Attribute, other keep it
            if( result_map.find(nAddr) == result_map.end() )
            {
                result_map[nAddr] = svr_attr;
            }
        }
        else
        {
            InetAddress nAddr1(nValues[i], 80);
            InetAddress nAddr2(nValues[i], 443);
            if( result_map.find(nAddr1) == result_map.end() )
            {
                result_map[nAddr1] = svr_attr;
            }
            if( result_map.find(nAddr2) == result_map.end() )
            {
                result_map[nAddr2] = svr_attr;
            }
            LogINFO("AppendServer %s", nAddr1.AsString().c_str());
            LogINFO("AppendServer %s", nAddr2.AsString().c_str());
        }
    }
    result_map_lock_.UnLock();

    //start test
    StartTest(true);

    return Common::ERROR_SUCESS;
}

void NetTestManager::UpdateTestResult(const InetAddress &nInetAddr, uint32_t test_type, uint32_t rtt)
{
    CCriticalAutoLock guard(result_map_lock_);
    ResultMap::iterator it = m_nRtpResult.find(nInetAddr);
    if (it != m_nRtpResult.end())
    {
        bool& is_net_enbale = (Common::UDP == test_type) ? it->second.bUseUdp : it->second.bUseTcp;
        is_net_enbale = true;
        it->second.rtt_ = rtt;
        it->second.update_time_ = Utils::Time();
        InetAddress nAddr = it->first;
        LogINFO("---Update rtp result: %s, type:%s---", nAddr.AsString().c_str(), (Common::UDP == test_type) ? "UDP" : "TCP");
    }
	else
	{
		//for udp Dynamic IP with rtp
		if (Common::UDP == test_type)
		{
			Attribute nAttribute;
			nAttribute.bUseUdp = true;
			nAttribute.rtt_ = rtt;
			nAttribute.update_time_ = Utils::Time();

			m_nRtpResult[nInetAddr] = nAttribute;
			InetAddress nTempAddr = nInetAddr;
			LogINFO("---Update rtp dynamic result: %s, type:UDP---", nTempAddr.AsString().c_str());
		}
	}

    if (Common::UDP == test_type)
    {
        return;
    }

    it = m_nSipResult.find(nInetAddr);
    if (it != m_nSipResult.end())
    {
        it->second.bUseTcp = true;
        it->second.rtt_ = rtt;
        it->second.update_time_ = Utils::Time();
        InetAddress nAddr = it->first;
        LogINFO("---Update sip result: %s, type:%s---", nAddr.AsString().c_str(), (Common::UDP == test_type) ? "UDP" : "TCP");
    }
}

