#include "Utils.h"
#include "RtpChannelManager.h"
#include "TraceLog.h"

const int64_t s_nSchedulingPeriod = 1 * Utils::kNumMillisecsPerSec;

RtpChannelManager::RtpChannelManager( INetEventLoop* net_event_loop, NetTestManager* pNetTestManager )
{
    net_event_loop_ = net_event_loop;
    m_pNetTestManager = pNetTestManager;
    m_nPriority = USE_UDP;
    m_nSchedulingTime = Utils::Time();
    m_nLastGetServerTime = Utils::Time();
}

RtpChannelManager::~RtpChannelManager()
{
}

int RtpChannelManager::ClearRtpChannel()
{
    CCriticalAutoLock nAutoLock(m_nChannelMapLock);
    for( RtpChannelMap::iterator iItr = m_nRtpChannelMap.begin(); iItr != m_nRtpChannelMap.end(); iItr++ )
    {
        RtpChannel* pRtpChannel = iItr->second;
		if (NULL == pRtpChannel)
			continue;

        pRtpChannel->Clear();
    }
	LogINFO("Clear all rtp channel's local point");
    return Common::ERROR_SUCESS;
}

bool RtpChannelManager::GetState( char* pRtpRelayServer )
{
    if( NULL == pRtpRelayServer )
    {
        return false;
    }
    
    std::string strServer = m_nRelayServer.AsString();
    memcpy(pRtpRelayServer, strServer.c_str(), strServer.size());
    return true;
}

void RtpChannelManager::ResetRelayIp()
{
	m_nRelayServer.Clear();
	return;
}

void RtpChannelManager::OnTimer()
{
    //scheduling 1s to every time
    int64_t nCurTime = Utils::Time();
    if( (nCurTime - m_nSchedulingTime) < s_nSchedulingPeriod )
    {
        return;
    }
    m_nSchedulingTime = nCurTime;
	
    if ((nCurTime - m_nLastGetServerTime) >= s_nTestInterval)
	{
		m_pNetTestManager->GetNewSvr(m_nRelayServer, Common::RTP, m_nPriority, m_nRelayServer);
		LogINFO("RtpChannelManager OnTimer to start test to update new relay server ip:%s.", m_nRelayServer.ToString().c_str());
        m_nLastGetServerTime = nCurTime;
	}

	//delete delay object
	m_nDelayRtpChannel.RelayExpireObj(10 * 1000);
	
    //check channel timeout and deatch
    bool bTimeout = false;
    m_nChannelMapLock.Lock();
    size_t nRealTimeCount = 0;
    for( RtpChannelMap::iterator iItr = m_nRtpChannelMap.begin(); iItr != m_nRtpChannelMap.end(); )
    {
        RtpChannel* pRtpChannel = iItr->second;
		if (NULL == pRtpChannel)
		{
			LogERR("RtpChannel is null.");
			iItr++;
			continue;
		}
		if (pRtpChannel->CheckDeatched())
		{
			pRtpChannel->Close();
			m_nDelayRtpChannel.Attach(pRtpChannel);
			m_nRtpChannelMap.erase(iItr++);
			continue;
		}

        pRtpChannel->OnTimer();
        if (pRtpChannel->CheckTimeout() || pRtpChannel->IsClosed())
        {
            nRealTimeCount++;
        }
		iItr++;
    }

    if ((nRealTimeCount >= (m_nRtpChannelMap.size() / 2)) && nRealTimeCount > 0)
    {
        LogINFO("Channel is timeout, but do nothing");
		//LogINFO("RealTimeCount [%u], rtp channel size[%u]", nRealTimeCount, m_nRtpChannelMap.size());
        bTimeout = true;
    }
    m_nChannelMapLock.UnLock();

    if( !bTimeout && !m_nRelayServer.Empty() )
    {
        return;
    }

    m_pNetTestManager->GetNewSvr(m_nRelayServer, Common::RTP, m_nPriority, m_nRelayServer);
    if( m_nRelayServer.Empty() )
    {
        LogERR("Rtp relay server is null. Do nothing.");
        return;
    }
    LogINFO("Get rtp server:%s.", m_nRelayServer.AsString().c_str());
  /*  CCriticalAutoLock nAutoLock(m_nChannelMapLock);
    for( RtpChannelMap::iterator iItr = m_nRtpChannelMap.begin(); iItr != m_nRtpChannelMap.end(); iItr++ )
    {
        RtpChannel* pRtpChannel = iItr->second;
		if (NULL == pRtpChannel)
		{
			LogERR("Rtp channel is null.");
			continue;
		}

        RemoteTransport::TransType nType = RemoteTransport::TRANS_UDP;
        if( (m_nPriority == USE_ALL && pRtpChannel->CheckUseTcp()) 
            || m_nPriority == USE_TCP )
        {
            nType = RemoteTransport::TRANS_TCP;
        }
        pRtpChannel->ConnectRelaySvr(m_nRelayServer.ip_, m_nRelayServer.port_, nType);
    }*/

    return;
}

int RtpChannelManager::AddChannel(const InetAddress &nRemoteAddr, bool bUseTcp)
{
	CCriticalAutoLock nAutoLock(m_nChannelMapLock);
	RtpChannelMap::const_iterator iItr = m_nRtpChannelMap.find(nRemoteAddr);
	if (iItr != m_nRtpChannelMap.end())
	{
		InetAddress nAddr = nRemoteAddr;
		LogERR("AddChannel exsit.%s", nAddr.AsString().c_str());
		return Common::ERROR_ALREADY_EXSIT;
	}
    RtpChannel *pRtpChannel = new RtpChannel(net_event_loop_);
	int nResult = pRtpChannel->Open(nRemoteAddr, bUseTcp);
    if( !m_nRelayServer.Empty() )
    {
        RemoteTransport::TransType nType = RemoteTransport::TRANS_UDP;
        if( (m_nPriority == USE_ALL && pRtpChannel->CheckUseTcp()) 
            || m_nPriority == USE_TCP )
        {
            nType = RemoteTransport::TRANS_TCP;
        }
        pRtpChannel->ConnectRelaySvr(m_nRelayServer.ip_, m_nRelayServer.port_, nType);
    }
    
	m_nRtpChannelMap.insert(RtpChannelMap::value_type(nRemoteAddr, pRtpChannel));
    return nResult;
}

int RtpChannelManager::BindChannel(const InetAddress &nRemoteAddr, IRemoteDataSink* remote_data_sink, ILocalDataSink** local_data_sink)
{
	CCriticalAutoLock nAutoLock(m_nChannelMapLock);
	RtpChannelMap::iterator iItr = m_nRtpChannelMap.find(nRemoteAddr);
	if (iItr == m_nRtpChannelMap.end())
	{
		InetAddress nAddr = nRemoteAddr;
		//LogERR("BindChannel not exsit.%s", nAddr.AsString().c_str());
		return Common::ERROR_NOT_EXSIT;
	}
	RtpChannel *pRtpChannel = iItr->second;
	if (pRtpChannel == NULL)
	{
		LogERR("Rtp channel is null.");
		return Common::ERROR_NOT_INIT;
	}
	pRtpChannel->Bind(remote_data_sink, local_data_sink);
	return Common::ERROR_SUCESS;
}

