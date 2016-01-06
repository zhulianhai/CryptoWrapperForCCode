#ifndef LOCAL_PROXY_COMMON_H__
#define LOCAL_PROXY_COMMON_H__

namespace Common
{
	typedef enum tagLOCAL_PROXY_ERROR
	{
		ERROR_SUCESS = 0,
		ERROR_COMMON =  -1000,
		ERROR_NET_ADDRESS,
		ERROR_NOT_INIT,
		ERROR_NOT_EXSIT,
		ERROR_ALREADY_EXSIT,
		ERROR_PARAM_INVALID,
        ERROR_NO_READY,
		ERROR_CANNOT_ALLOC,
        ERROR_SOCKET_INVALID
	}LOCAL_PROXY_ERROR;

    typedef enum tagNetType
    {
        UNKNOWN = -1,
        UDP,
        TCP,
        HTTP
    }NetType;

    typedef enum tagRelayType
    {
        SIP = 0,
        RTP
    }RelayType;
}

class ILocalDataSink
{
public:
    virtual int OnSendToPeer(char* data, unsigned int data_len) = 0;
    virtual void Detach() = 0;
    virtual bool GetRecvData(char* data, unsigned int &data_len, char* remote_ip, unsigned short &remote_port) = 0;
};

class IRemoteDataSink
{
public:
    virtual void OnSendToLocalEvent( int nEvent ) = 0;
};

#endif

