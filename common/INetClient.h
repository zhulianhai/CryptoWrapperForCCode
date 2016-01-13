#ifndef __INETCLIENT_H__
#define __INETCLIENT_H__
#include "TypeDef.h"

class INetTcpEvent
{
public:
	virtual void OnConnected(int socket_fd, void* context) = 0;
	virtual void OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context) = 0;
	virtual void OnSendReady(int socket_fd, void* context) = 0;
	virtual void OnClose(int socket_fd, void* context) = 0;
    virtual bool isNeedSend(int socket_fd, void* context) = 0;
};

class DefaultNetTcpEventHandler : public INetTcpEvent
{
public:
    virtual void OnConnected(int socket_fd, void* context)
    {
        INetTcpEvent* tcp_event = static_cast<INetTcpEvent*>(context);
        tcp_event->OnConnected(socket_fd, context);
    }

    virtual void OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context)
    {
        INetTcpEvent* tcp_event = static_cast<INetTcpEvent*>(context);
        tcp_event->OnReciveData(socket_fd, data, data_len, context);
    }

    virtual void OnSendReady(int socket_fd, void* context)
    {
        INetTcpEvent* tcp_event = static_cast<INetTcpEvent*>(context);
        tcp_event->OnSendReady(socket_fd, context);
    }

    virtual void OnClose(int socket_fd, void* context)
    {
        INetTcpEvent* tcp_event = static_cast<INetTcpEvent*>(context);
        tcp_event->OnClose(socket_fd, context);
    }
    virtual bool isNeedSend(int socket_fd, void* context)
    {
        INetTcpEvent* tcp_event = static_cast<INetTcpEvent*>(context);
        return tcp_event->isNeedSend(socket_fd, context);
    }

};

class INetUdpEvent
{
public:
	virtual void OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context, 
		uint32_t remote_ip, uint16_t remote_port) = 0;
	virtual void OnClose(int socket_fd, void* context) = 0;
};

class DefaultNetUdpEventHandler : public INetUdpEvent
{
public:
    virtual void OnReciveData(int socket_fd, char* data, uint32_t data_len, void* context, 
        uint32_t remote_ip, uint16_t remote_port)
    {
        INetUdpEvent* udp_event = static_cast<INetUdpEvent*>(context);
        udp_event->OnReciveData(socket_fd, data, data_len, context, remote_ip, remote_port);
    }

    virtual void OnClose(int socket_fd, void* context)
    {
        INetUdpEvent* udp_event = static_cast<INetUdpEvent*>(context);
        udp_event->OnClose(socket_fd, context);
    }

};

class ITimerEvent
{
public:
    virtual void OnTimerEvent() = 0;
};

class INetEventLoop
{
public:
	virtual uint8_t Start(INetTcpEvent* tcp_event, INetUdpEvent* udp_event, ITimerEvent* timer_event) = 0;
	virtual void Stop() = 0;
	virtual int CreatUdpSocket() = 0;
	virtual int CreatTcpSocket() = 0;
	virtual int Bind(int fd, uint32_t local_ip, uint16_t &local_port) = 0;//if local_port=0 this func will out the local_port.if local_port!=0,this func bind local_port.return 0 bind succ, -1 bind faild
	virtual int Connect(int fd, void* context, uint32_t remote_ip, uint16_t remote_port) = 0;
	virtual void SetSocketContext(int socket_fd, void* context, int socket_typ) = 0;//socket_typ: 0 is tcp, 1 is udp
	virtual int SendTcpData(int socket_fd, char* data, uint32_t data_len) = 0;
	virtual int SendUdpData(int socket_fd, char* data, uint32_t data_len,
		uint32_t remote_ip, uint16_t remote_port) = 0;
	virtual void CloseSocket(int socket_fd) = 0;
    virtual int WakeUpEvent() = 0;
};

INetEventLoop* CreateNetEventLoop();
void ReleaseNetEventLoop(INetEventLoop*& event_loop);
#endif

