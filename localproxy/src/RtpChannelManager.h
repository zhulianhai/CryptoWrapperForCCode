#ifndef CHANNEL_MANAGER_H__
#define CHANNEL_MANAGER_H__

#include "Utils.h"
#include "CriticalSection.h"
#include "LocalProxyCommon.h"
#include "RtpChannel.h"
#include "NetTestManager.h"

typedef std::map<InetAddress, RtpChannel*> RtpChannelMap;

class RtpChannelManager
{
public:
	RtpChannelManager( INetEventLoop* net_event_loop, NetTestManager* pNetTestManager );
	~RtpChannelManager();

	int AddChannel(const InetAddress &nRemoteAddr, bool bUseTcp);
	int BindChannel(const InetAddress &nRemoteAddr, IRemoteDataSink* remote_data_sink, ILocalDataSink** local_data_sink);
    int ClearRtpChannel();
    bool GetState( char* pRtpRelayServer );
    void OnTimer();
	void ResetRelayIp();

private:
    //for channle;
    CCriticalSection m_nRelayServeLock;
	InetAddress m_nRelayServer;
    Priority m_nPriority;

    CCriticalSection m_nChannelMapLock;
    RtpChannelMap m_nRtpChannelMap;
	
    INetEventLoop* net_event_loop_;

    //for NetTestManager
    NetTestManager* m_pNetTestManager;
    DelayRelease<RtpChannel> m_nDelayRtpChannel;

    int64_t m_nSchedulingTime;
    int64_t m_nLastGetServerTime;
};

#endif
