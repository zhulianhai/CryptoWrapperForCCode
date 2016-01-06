#ifndef __NETCLIENT_H__
#define __NETCLIENT_H__

#include "Utils.h"
#include "INetClient.h"
#include "CriticalSection.h"
#include "BaseThread.h"
#include "NetCommon.h"
/*#ifndef WIN32 
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#else
#include <Windows.h>
#include <WinSock2.h>
//#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#pragma comment( lib,"winmm.lib" )
#endif*/

//host byte order: use it in program
//net byte order: use it when send data or use socket api

typedef std::vector<InetAddress> InetAddressArray;
typedef std::set<InetAddress> InetAddressSet;

class NetClient : public INetEventLoop
{
public:
	NetClient(void);
	virtual ~NetClient(void);

public:
	virtual uint8_t Start(INetTcpEvent* tcp_event, INetUdpEvent* udp_event, ITimerEvent* timer_event);
	virtual void Stop();
	virtual int CreatUdpSocket();
	virtual int CreatTcpSocket();
	virtual int Bind(int fd, uint32_t local_ip, uint16_t &local_port);
	virtual int Connect(int fd, void* context, uint32_t remote_ip, uint16_t remote_port);
	virtual void SetSocketContext(int socket_fd, void* context, int socket_typ);
	virtual int SendTcpData(int socket_fd, char* data, uint32_t data_len);
	virtual int SendUdpData(int socket_fd, char* data, uint32_t data_len,
		uint32_t remote_ip, uint16_t remote_port);
	virtual void CloseSocket(int socket_fd);
    virtual int WakeUpEvent();

private:
	typedef std::map<int, void*> socket_map;

private:
    typedef enum {EVENT_R, EVENT_W, EVENT_E} EM_SOCKET_EVENT;
    typedef void (*HandleSocketEvent)(NetClient *, int socket_fd, EM_SOCKET_EVENT ent, void* context);

    static uint32_t ThreadFun(void* arg);
    void MyRun();
    void FillFdset(std::map<int, void*>& socket_map, fd_set* fds_read, fd_set* fds_write, fd_set* fds_error, int& socket_max_fd, bool check_write_event);
    void HandleEvent(std::map<int, void*>& socket_map, fd_set* fds_read, fd_set* fds_write, fd_set* fds_error, HandleSocketEvent handler);
    void HandleTcpSocketEvent(int socket_fd, EM_SOCKET_EVENT ent, void* context);
    void HandleUdpSocketEvent(int socket_fd, EM_SOCKET_EVENT ent, void* context);

    static void HandleTcpSocketEvent(NetClient *clt, int socket_fd, EM_SOCKET_EVENT ent, void* context);
    static void HandleUdpSocketEvent(NetClient *clt, int socket_fd, EM_SOCKET_EVENT ent, void* context);

    int CreateSocket(bool usingtcp = false);
    bool SetNonblocking(int fd, bool bNb);

    virtual int RecvTcpData(int socket_fd, char* data, uint32_t data_len);
    virtual int RecvUdpData(int socket_fd, char* data, int data_len,
			uint32_t &remote_ip, uint16_t &remote_port);
    int Wakeup();
    int HandleConnecting(socket_map &connecting_map, fd_set *fd_r, fd_set *fd_w, fd_set *fd_e);
    bool IsIgnoreSocketError( int &nErrorCode );
	bool IsIgnoreSocketError_UDP(int &nErrorCode);

private:

#define MAKE_SOCKADDR_IN(var,adr,prt) /*adr,prt must be in network order*/ \
	struct sockaddr_in var;\
	var.sin_family = AF_INET;\
	var.sin_addr.s_addr = (adr);\
	var.sin_port = (prt);

#ifdef _WIN32
#define close_socket(x) if (x != INVALID_SOCKET) {closesocket(x); x = INVALID_SOCKET;}
#else
#define SOCKET int
#define INVALID_SOCKET -1
#define close_socket(x) if (x != INVALID_SOCKET) {close(x); x = INVALID_SOCKET;}
#endif

	CCriticalSection lock_tcp_map_;
	socket_map tcp_socket_map_;

	CCriticalSection lock_udp_map_;
	socket_map udp_socket_map_;

	CCriticalSection lock_connecting_map_;
	socket_map connecting_map_;

	CBaseThread thread_;

	int notify_socket_fd_;
	uint16_t notify_socket_port_;
	INetTcpEvent* sink_tcp_event_; 
	INetUdpEvent* sink_udp_event_;
    ITimerEvent* timer_event_;
    char tcp_recv_buf_[TCP_BUF_SIZE];
		char udp_recv_buf_[UDP_BUF_SIZE];

		bool running_;
};
#endif

