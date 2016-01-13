#ifndef LOCAL_PROXY_IMPL_H__
#define LOCAL_PROXY_IMPL_H__

#include "ILocalProxy.h"
#include "RtpChannelManager.h"
#include "LocalProxyCommon.h"
#include "NetTestManager.h"
#include "SipRelay.h"
#include "TraceLog.h"

class LocalProxyImpl : public ILocalProxy, public ITimerEvent
{
public:
    virtual int Init();
    virtual int UnInit();
    virtual void SetRelayEnable( bool bRelayEnable );
    virtual int SetRelayServer( const char* pRelayServers, Common::RelayType nRerverType );
    virtual int SetLogInfo( bool bEable, const char* pFileName );

    //sip relay api
    virtual int SetSipAuthData( const char* auth_ip, unsigned short auth_port ); 
    virtual int GetSipChannel( unsigned short &server_port );
    virtual int GetSipRelayStatus();
    virtual int TransRemoteIp( const char* pWlanIp, char* pLanIp );

    //rtp relay api
	virtual int AddRtpChannel(const char* remote_ip, int remote_port, bool bUseTcp);
	virtual int BindChannel(const char* remote_ip, int remote_port, IRemoteDataSink* remote_data_sink, ILocalDataSink** local_data_sink);
    virtual int ClearRtpChannel();

public:
	LocalProxyImpl();
	virtual ~LocalProxyImpl();

private:
    //impl for ITimerEvent
    virtual void OnTimerEvent();

    //for thread
	static uint32_t ThrendFun(void* arg);
    void MyRun();
    CBaseThread* m_pThread;
    unsigned int m_nThreadID;
    bool m_bStop;

    RtpChannelManager* m_pRtpChannelManager;
    SipRelay* m_pSipRelay;
    NetTestManager* m_pNetTestManager;

    INetEventLoop* m_pNetEventLoop;
    DefaultNetUdpEventHandler* udp_handler_;
    DefaultNetTcpEventHandler* tcp_handler_;

    bool m_bRelayEnable;
};

#endif
