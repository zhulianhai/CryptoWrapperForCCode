#include "LocalProxyImpl.h"
#include "NetTestManager.h"

#define CHECK_ENABLE(x) {if (!x) { return Common::ERROR_NO_READY; }} 

//interface 
ILocalProxy* CreateLocalProxy()
{
	return new LocalProxyImpl();
}

void DestroyLocalProxy( ILocalProxy *pLocalProxy )
{
	delete static_cast<LocalProxyImpl*>(pLocalProxy);
	pLocalProxy = NULL;
	return;
}

//implement
LocalProxyImpl::LocalProxyImpl()
{
    m_pRtpChannelManager = NULL;
    m_pNetTestManager = NULL;
    m_pSipRelay = NULL;
    tcp_handler_ = new DefaultNetTcpEventHandler();
    udp_handler_ = new DefaultNetUdpEventHandler();

    m_bRelayEnable = true;
}

LocalProxyImpl::~LocalProxyImpl()
{
    SAFE_DELETE(udp_handler_);
    SAFE_DELETE(tcp_handler_);
}

int LocalProxyImpl::Init( )
{	
    //create network thread
    m_pNetEventLoop = CreateNetEventLoop();
    ITimerEvent *pTimerEvent = this;
    m_pNetEventLoop->Start(tcp_handler_, udp_handler_, pTimerEvent);

    //init
    m_pNetTestManager = new NetTestManager(m_pNetEventLoop);
    m_pRtpChannelManager = new RtpChannelManager(m_pNetEventLoop, m_pNetTestManager);
    m_pSipRelay = new SipRelay(m_pNetEventLoop, m_pNetTestManager);
    
    //start thread
    m_bStop = false;
    m_pThread = new CBaseThread();
    m_pThread->BeginThread(ThrendFun,this, m_nThreadID);

    return Common::ERROR_SUCESS;
}

int LocalProxyImpl::UnInit()
{
    m_bStop = true;
    m_pNetEventLoop->Stop();
    while( m_pThread->IsRunning() )
    {
        CBaseThread::Sleep(10);
        LogINFO("RtpChannelManager thread sleep for end..");
    }

    if( NULL != m_pRtpChannelManager )
    {
        m_pRtpChannelManager->ClearRtpChannel();
        SAFE_DELETE(m_pRtpChannelManager);
    }

    if( NULL != m_pSipRelay )
    {
        m_pSipRelay->Close();
        SAFE_DELETE(m_pSipRelay);
    }

    if( NULL != m_pNetTestManager )
    {
        SAFE_DELETE(m_pNetTestManager);
    }

    ReleaseNetEventLoop(m_pNetEventLoop);
    return Common::ERROR_SUCESS;
}

void LocalProxyImpl::SetRelayEnable( bool bRelayEnable )
{
    m_bRelayEnable = bRelayEnable;
}

int LocalProxyImpl::SetRelayServer( const char* pRelayServers, Common::RelayType nRerverType )
{
    assert(NULL != m_pSipRelay && NULL != m_pNetTestManager );
    CHECK_ENABLE(m_bRelayEnable);

    if( (NULL == pRelayServers) || (strlen(pRelayServers) <= 0) 
        || (nRerverType == Common::SIP && nRerverType == Common::RTP) )
    {
        LogERR("relay server param invlid!");
        return Common::ERROR_PARAM_INVALID;
    }

    LogINFO("Set %s's relay server.%s", (nRerverType == Common::SIP) ? "sip" : "rtp" ,pRelayServers);
	int nResult = m_pNetTestManager->AppendServer(pRelayServers, nRerverType);

	if (nRerverType == Common::SIP)
	{
		m_pSipRelay->ResetChannel();
	}
	else if (nRerverType == Common::RTP)
	{
		m_pRtpChannelManager->ResetRelayIp();
	}
	return nResult;
}

//for sip
int LocalProxyImpl::SetSipAuthData( const char* auth_ip, unsigned short auth_port )
{
    assert(NULL != m_pSipRelay);
    CHECK_ENABLE(m_bRelayEnable);

    if( NULL == auth_ip )
    {
        LogERR("SetAuthData, auth ip is null");
        return Common::ERROR_PARAM_INVALID;
    }
    InetAddress nAddr(auth_ip, auth_port);
    m_pSipRelay->SetAuthData(nAddr.ip_, nAddr.port_);
    return Common::ERROR_SUCESS;
}

int LocalProxyImpl::GetSipChannel( uint16_t &server_port )
{
    assert(NULL != m_pSipRelay);
    if( !m_bRelayEnable )
    {
        //LogINFO("Do not use relay");
        return 0;
    }

    uint32_t local_ip = 0;
    server_port = 0;
    if( !m_pSipRelay->GetState() )
    {
        //LogINFO("The sip channel is stopped.please wait..");
        return -1;
    }
    
    if( m_pSipRelay->IsRestarted() )
    {
        //when return -2 ,it need to be registed
        LogERR("The sip channel is restart again. Register the sip again.");
        return -2;
    }
    
    m_pSipRelay->GetService(local_ip, server_port);
    if( server_port <= 0 )
    {
        //-1 is error
        //LogERR("The sip channel server port is 0.");
        return -1;
    }

    //LogINFO("Sip server port %d.", server_port);
    return server_port;
}

int LocalProxyImpl::GetSipRelayStatus()
{
    assert(m_pSipRelay != NULL);
    CHECK_ENABLE(m_bRelayEnable);
    if( m_pSipRelay->GetState() )
    {
        LogINFO("Sip relay is ready");
        return Common::ERROR_SUCESS;
    }
    else
    {
        LogERR("Sip relay is not ready");
        return Common::ERROR_NO_READY;
    }
}

int LocalProxyImpl::TransRemoteIp( const char* pWlanIp, char* pLanIp )
{
    assert(NULL != m_pSipRelay);
    CHECK_ENABLE(m_bRelayEnable);

    if( NULL == pWlanIp || NULL == pLanIp )
    {
        LogERR("TransRemoteIp parameter is invalid");
        return Common::ERROR_PARAM_INVALID;
    }

    std::string strWlanIp(pWlanIp);
    std::string strLanIp;
    int nResult = Common::ERROR_SUCESS;
    nResult = m_pSipRelay->TransRemoteIp(strWlanIp, strLanIp);
    if( Common::ERROR_SUCESS != nResult )
    {
        return nResult;
    }

    memcpy(pLanIp, strLanIp.c_str(), strLanIp.size());
    return nResult;
}

//for rtp
int LocalProxyImpl::AddRtpChannel(const char* remote_ip, int remote_port, bool bUseTcp)
{
    assert(m_pRtpChannelManager != NULL);
    CHECK_ENABLE(m_bRelayEnable);
	if (remote_ip == NULL || strlen(remote_ip) <= 0 || remote_port <= 0)
	{
		LogERR("AddRtpChannel ERROR_PARAM_INVALID");
		return Common::ERROR_PARAM_INVALID;
	}

	std::string strRemoteIp(remote_ip);
	InetAddress nAddr(strRemoteIp, remote_port);
	return m_pRtpChannelManager->AddChannel(nAddr, bUseTcp);
}

int LocalProxyImpl::BindChannel(const char* remote_ip, int remote_port, IRemoteDataSink* remote_data_sink, ILocalDataSink** local_data_sink)
{
	assert(m_pRtpChannelManager != NULL);
	CHECK_ENABLE(m_bRelayEnable);
	std::string strRemoteIp(remote_ip);
	InetAddress nAddr(strRemoteIp, remote_port);
	return m_pRtpChannelManager->BindChannel(nAddr, remote_data_sink, local_data_sink);
}

int LocalProxyImpl::ClearRtpChannel()
{
    assert(m_pRtpChannelManager != NULL);
    CHECK_ENABLE(m_bRelayEnable);
    return m_pRtpChannelManager->ClearRtpChannel();;
}

int LocalProxyImpl::SetLogInfo( bool bEable, const char* pFileName )
{
    CHECK_ENABLE(m_bRelayEnable);
    if( bEable )
    {
        TraceLog::log().openLog((pFileName == NULL) ? "local_proxy.log" : pFileName, TL_DEBUG, true );
        if (NULL != m_pSipRelay)
        {
            m_pSipRelay->EnablePrintMessage(true);
        }
    }
    else
    {
        TraceLog::log().openLog("", TL_EMERG, false );
    }

    return Common::ERROR_SUCESS;
}

uint32_t LocalProxyImpl::ThrendFun(void* arg)
{
    LocalProxyImpl* pManager = (LocalProxyImpl*)arg;
    pManager->MyRun();
    return 0;
}

void LocalProxyImpl::MyRun()
{
    while( !m_bStop )
    {
        for( int i = 0; i < 3; i++ )
        {
            if( m_bStop )
            {
                return;
            }
            CBaseThread::Sleep(100);
        }
        
        if( NULL != m_pNetEventLoop )
        {
            m_pNetEventLoop->WakeUpEvent();
        }
    }
}

void LocalProxyImpl::OnTimerEvent()
{
    if( NULL != m_pNetTestManager )
    {
        m_pNetTestManager->OnTimer();
    }

    if( NULL != m_pRtpChannelManager )
    {
        m_pRtpChannelManager->OnTimer();
    }

    if( NULL != m_pSipRelay )
    {
        m_pSipRelay->OnTimer();
    }
}

