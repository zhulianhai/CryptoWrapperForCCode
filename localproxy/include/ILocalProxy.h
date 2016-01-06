#ifndef LOCAL_PROXY_INTERFACE_H__
#define LOCAL_PROXY_INTERFACE_H__
#include "LocalProxyCommon.h"


#ifdef LOCALPROXY_EXPORTS
#define LOCALPROXY_API __declspec(dllexport)
#else
#ifdef LOCALPROXY_IMPORTS
#define LOCALPROXY_API __declspec(dllimport)
#else 
#define LOCALPROXY_API
#endif
#endif

#ifdef __cplusplus
extern "C"{
#endif

	class ILocalProxy;

	//create and destroy interface
	LOCALPROXY_API ILocalProxy* CreateLocalProxy();
	LOCALPROXY_API void DestroyLocalProxy(ILocalProxy *pLocalProxy);

	class LOCALPROXY_API ILocalProxy
	{
	public:
		virtual int Init() = 0;
		virtual int UnInit() = 0;
		virtual void SetRelayEnable(bool bRelayEnable) = 0;
		virtual int SetRelayServer(const char* pRelayServers, Common::RelayType nRerverType) = 0;
		virtual int SetSipAuthData(const char* auth_ip, unsigned short auth_port) = 0;
		virtual int SetLogInfo(bool bEable, const char* pFileName) = 0;

		//sip relay api
		virtual int GetSipChannel(unsigned short &server_port) = 0; //=0: donot use relay; >0: server port; -1: wait again; -2: need to be regist again.
		virtual int GetSipRelayStatus() = 0;
		virtual int TransRemoteIp(const char* pWlanIp, char* pLanIp) = 0;

		//rtp relay api
		virtual int AddRtpChannel(const char* remote_ip, int remote_port, bool bUseTcp) = 0;
		virtual int BindChannel(const char* remote_ip, int remote_port, IRemoteDataSink* remote_data_sink, ILocalDataSink** local_data_sink) = 0;
		virtual int ClearRtpChannel() = 0;

	protected:
		ILocalProxy(){};
		virtual ~ILocalProxy(){};
	};

#ifdef __cplusplus
}
#endif

#endif
