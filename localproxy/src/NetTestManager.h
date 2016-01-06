#ifndef __NET_TEST_MONITRO_H__
#define __NET_TEST_MONITRO_H__
#include <map>
#include "Utils.h"
#include "LocalProxyCommon.h"
#include "PackDef.h"
#include "BaseThread.h"
#include "TraceLog.h"
#include "NetClient.h"
#include "UdpNetClient.h"
#include "TcpNetClient.h"

const int64_t s_nTestPeriod = 10 * Utils::kNumMillisecsPerSec; // 10 seconds
const int64_t s_nTestInterval = 10 * 60 * Utils::kNumMillisecsPerSec; // 10 miniutes

typedef enum tagPriority
{
	USE_NONE,
    USE_ALL,
    USE_TCP,
    USE_UDP
}Priority;

typedef struct tagAttribute
{
    bool bUseTcp;
    bool bUseUdp;
    uint32_t rtt_;
    uint32_t update_time_;
    tagAttribute()
    {
        Reset();
    }
    void Reset()
    {
        bUseTcp = false;
        bUseUdp = false;
        rtt_ = 0;
        update_time_ = 0;
    }
	Priority GetPriority()
	{
		if (bUseTcp && bUseUdp)
			return USE_ALL;
		else if (bUseTcp)
			return USE_TCP;
		else if (bUseUdp)
			return USE_UDP;
		else
			return USE_NONE;
	}
}Attribute;
typedef std::map<InetAddress, Attribute> ResultMap;


class NetTestManager;

class NetTest : public PduHandler
{
protected:
    NetTest(NetTestManager* net_test_mgr, uint32_t test_type);

    //pud handler impl
    virtual void OnHandlePdu(uint16_t cmd_type, char* packet, uint16_t packet_len ,
        uint32_t remote_ip, uint16_t remote_port);
    virtual void OnSendPdu(){}

    NetTestManager* net_test_mgr_;
private:
    uint32_t test_type_;
};

class UdpNetTest : public NetTest
{
public:
    UdpNetTest(NetTestManager* net_test_mgr, INetEventLoop* net_event_loop);
    bool StartTest(ResultMap& test_dest);
    void StopTest();
private:
    UdpNetClient udp_net_client_;
};

class TcpNetTest : public NetTest
{
public:
    TcpNetTest(NetTestManager* net_test_mgr);
    bool StartTest(ResultMap& test_dest);
    void StopTest();
private:
    TcpNetClient* GetTcpNetClient(InetAddress const& addr);
private:
    CCriticalSection tcp_cli_map_lock_;
    std::map<InetAddress, TcpNetClient*> tcp_cli_map_;
};

//test manager
class NetTestManager
{
    friend class NetTest;
public:
    NetTestManager( INetEventLoop* net_event_loop );
    ~NetTestManager();
    int AppendServer( const char* pRelayServers, Common::RelayType nType );
    void GetNewSvr(const InetAddress& cur_using_svr, Common::RelayType relay_type, Priority& nPriority, InetAddress& new_svr);

    INetEventLoop* GetEventLoop() {return net_event_loop_;}

    void OnTimer();
private:
    void StartTest( bool bForced = false );
    void StopTest();
    void UpdateTestResult(const InetAddress &nInetAddr, uint32_t test_type, uint32_t rtt);
    void RemoveExpireResult(ResultMap& result_map);
private:

    //sip and rtp test results
    ResultMap m_nSipResult;
    ResultMap m_nRtpResult;

    //time tag
    int64_t m_nLastTestTime;
    bool m_bTestActive;


    CCriticalSection result_map_lock_;
    INetEventLoop* net_event_loop_;
    UdpNetTest udp_net_test_;
    TcpNetTest tcp_net_test_;

    CCriticalSection start_test_lock_;
};

#endif



