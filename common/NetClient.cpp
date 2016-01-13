#include <errno.h>
#include <string>
#include <sstream>
#include <math.h>
#include "Utils.h"
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include "TraceLog.h"
#include "NetClient.h"
#include "BaseThread.h"
#ifndef _WIN32
//#include <sys/signal.h>
#include <signal.h>
#include <fcntl.h>
#endif
#if defined(LINUX)
#include <string.h>
#endif

/*#ifdef WIN32
typedef int socklen_t;
#endif*/


//bool getSourcePort(int socket, uint32_t ip, uint16_t& port);

NetClient::NetClient(void)
{
	notify_socket_fd_ = -1;
	notify_socket_port_ = 0;
	sink_tcp_event_ = NULL; 
	sink_udp_event_ = NULL;
    timer_event_ = NULL;
    memset(tcp_recv_buf_, 0, TCP_BUF_SIZE);
		memset(udp_recv_buf_, 0, UDP_BUF_SIZE);
		running_ = false;
}

NetClient::~NetClient(void)
{
	Stop();
}

uint8_t NetClient::Start(INetTcpEvent* tcp_event, INetUdpEvent* udp_event, ITimerEvent* timer_event)
{
	assert(tcp_event && udp_event);
	sink_tcp_event_ = tcp_event;
	sink_udp_event_ = udp_event;
    timer_event_ = timer_event;

	if (INVALID_SOCKET == (notify_socket_fd_ = CreatUdpSocket()))
		return -1;

	if ( 0 > Bind(notify_socket_fd_, ntohl(inet_addr("127.0.0.1")), notify_socket_port_)) {
        close_socket(notify_socket_fd_);
		return -1;
	}

	unsigned int thd = 0;
	thread_.BeginThread(ThreadFun, this, thd);
	
	return 0;
}

void NetClient::Stop()
{
	running_ = false;
    Wakeup();
    while( thread_.IsRunning() )
    {
        CBaseThread::Sleep(10);
    }
}

uint32_t NetClient::ThreadFun(void* arg)
{
	NetClient* net_event = (NetClient*)arg;
	net_event->MyRun();

	return 0;
}

void NetClient::MyRun()
{
    int socket_max_fd = notify_socket_fd_;
	bool socket_set_changed = false;

	std::map<int, void*> tcp_socket_map_tmp;
	std::map<int, void*> udp_socket_map_tmp;

	running_ = true;
	while (running_)
	{
		if(notify_socket_fd_ > socket_max_fd)
		{
			socket_max_fd = notify_socket_fd_;
		}

        fd_set fds_read;
        fd_set fds_write;
        fd_set fds_error;
		FD_ZERO(&fds_read);
		FD_ZERO(&fds_write);
		FD_ZERO(&fds_error);

		FD_SET(notify_socket_fd_, &fds_read);

		socket_map connecting_map_tmp;
		lock_connecting_map_.Lock();
		if (!connecting_map_.empty()) {
			connecting_map_tmp = connecting_map_;
		}
		lock_connecting_map_.UnLock();

		FillFdset(connecting_map_tmp, &fds_read, &fds_write, &fds_error, socket_max_fd, false);
		FillFdset(tcp_socket_map_tmp, &fds_read, &fds_write, &fds_error, socket_max_fd, true);
		FillFdset(udp_socket_map_tmp, &fds_read, NULL,/*&fds_write,*/ &fds_error, socket_max_fd, false);

		int result = select(socket_max_fd+1, &fds_read, &fds_write, &fds_error, 0);
		if (0 >= result)
		{
            //LogWARNING("select error: %s\r\n", strerror(errno));
            //for( size_t i = 0; i < fds_error.fd_count; i++ )
            //{
                //LogWARNING("fd error fd:%u", fds_error.fd_array[i]);
            //}
            lock_tcp_map_.Lock();
            tcp_socket_map_tmp = tcp_socket_map_;
            lock_tcp_map_.UnLock();

            lock_udp_map_.Lock();
            udp_socket_map_tmp = udp_socket_map_;
            lock_udp_map_.UnLock();
			continue;
		}

		if (FD_ISSET(notify_socket_fd_, &fds_read))
		{
			char data;
			uint32_t ip;
			uint16_t port;
            int nErrorCode = 0;
			int nRet = RecvUdpData(notify_socket_fd_, &data, sizeof (char), ip, port);
			if (nRet < 0 && !IsIgnoreSocketError_UDP(nErrorCode))
            {
                //ios re open the notify socket
                LogINFO("Notify socket is closed.socket is:%d, error code is %d, error des:%s", notify_socket_fd_, nErrorCode, strerror(nErrorCode));
                close_socket(notify_socket_fd_);
                int nLocalSocket = CreatUdpSocket();
                if (INVALID_SOCKET != nLocalSocket)
                {
                    notify_socket_port_ = 0;
                    if (0 > Bind(nLocalSocket, ntohl(inet_addr("127.0.0.1")), notify_socket_port_))
                    {
                        close_socket(nLocalSocket);
                    }
                    else
                    {
                        notify_socket_fd_ = nLocalSocket;
                        LogINFO("Notify socket is opened ok!.%d", notify_socket_fd_);
                    }
                }
            }

            if( NULL != timer_event_ && data == '1')
            {
                timer_event_->OnTimerEvent();
            }

			lock_tcp_map_.Lock();
			tcp_socket_map_tmp = tcp_socket_map_;
			lock_tcp_map_.UnLock();

			lock_udp_map_.Lock();
			udp_socket_map_tmp = udp_socket_map_;
			lock_udp_map_.UnLock();
		}

		HandleConnecting(connecting_map_tmp, &fds_read, &fds_write, &fds_error);
		HandleEvent(tcp_socket_map_tmp, &fds_read, &fds_write, &fds_error, HandleTcpSocketEvent);
		HandleEvent(udp_socket_map_tmp, &fds_read, &fds_write, &fds_error, HandleUdpSocketEvent);
	}
}

void NetClient::FillFdset(std::map<int, void*>& socket_map, fd_set* fds_read, fd_set* fds_write, fd_set* fds_error, int& socket_max_fd, bool check_write_event)
{
    std::map<int, void*>::iterator it = socket_map.begin();
    for (; it != socket_map.end(); it++)
    {
        if(it->first > socket_max_fd)
        {
            socket_max_fd = it->first;
        }

        if (fds_read != NULL)
	        FD_SET(it->first, fds_read);
        if (fds_write != NULL)
        {
            if (check_write_event)
            {
                if (sink_tcp_event_->isNeedSend(it->first, it->second))
                {
                    FD_SET(it->first, fds_write);
                }
            }
            else
            {
                FD_SET(it->first, fds_write);
            }
        }
        if (fds_error != NULL)
	        FD_SET(it->first, fds_error);
    }
}

int NetClient::HandleConnecting(socket_map &connecting_map, fd_set *fd_r, fd_set *fd_w, fd_set *fd_e)
{
	socket_map::iterator it = connecting_map.begin();
	for (; it != connecting_map.end(); it++) {
		bool connect_finished = false;
		int fd = it->first;
		if(FD_ISSET(fd, fd_r) && FD_ISSET(fd, fd_w)) {//如果套接口及可写也可读，需要进一步判断
			int error = 0;
			socklen_t len = sizeof(error);
			if(getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0) {
				lock_connecting_map_.Lock();
				connecting_map_.clear();
				lock_connecting_map_.UnLock();
				break;
			}
			if(error != 0) {
				lock_connecting_map_.Lock();
				socket_map::iterator it_c = connecting_map_.find(fd);
				connecting_map_.erase(it_c);
				lock_connecting_map_.UnLock();
                int nError;
                socklen_t errorlen = sizeof(nError);
                getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&nError, &errorlen);
				LogDEBUG("connect faild, fd is %d",fd, nError);
				continue;
			}
			sink_tcp_event_->OnConnected(fd, it->second);//如果error为0，则说明链接建立完成
			connect_finished = true;
		} else if((!FD_ISSET(fd, fd_r)) && FD_ISSET(fd, fd_w)) { //如果套接口可写不可读,则链接完成
			sink_tcp_event_->OnConnected(fd, it->second);
			connect_finished = true;
			//FD_CLR(fd, fd_w);
		}
		if (FD_ISSET(fd, fd_e)) {
			//LogDEBUG("connect faild, fd is %d, error is: %d %s",fd, errno, strerror(errno));
			connect_finished = true;
		}
		if (connect_finished) {
			lock_connecting_map_.Lock();
			socket_map::iterator it_c = connecting_map_.find(fd);
            if( it_c != connecting_map_.end() )
			    connecting_map_.erase(it_c);
			lock_connecting_map_.UnLock();
		}
	}
	return 0;
}

void NetClient::HandleEvent(std::map<int, void*>& socket_map, fd_set* fds_read, fd_set* fds_write,
    fd_set* fds_error, HandleSocketEvent handler)
{
    int socket_fd = -1;
    void* context = NULL;
    std::map<int, void*>::iterator it = socket_map.begin();
    for (; it != socket_map.end(); it++)
    {
        socket_fd = it->first;
        context = it->second;
        if (FD_ISSET(socket_fd, fds_read))
        {
            (handler)(this, socket_fd, EVENT_R, context);
        }

        if (FD_ISSET(socket_fd, fds_write))
        {
            (handler)(this, socket_fd, EVENT_W, context);
        }

        if (FD_ISSET(socket_fd, fds_error))
        {
            (handler)(this, socket_fd, EVENT_E, context);
        }

    }
}

void NetClient::HandleTcpSocketEvent(NetClient *clt, int socket_fd, EM_SOCKET_EVENT ent, void* context)
{
	clt->HandleTcpSocketEvent(socket_fd, ent, context);
}

void NetClient::HandleUdpSocketEvent(NetClient *clt, int socket_fd, EM_SOCKET_EVENT ent, void* context)
{
	clt->HandleUdpSocketEvent(socket_fd, ent, context);
}

void NetClient::HandleTcpSocketEvent(int socket_fd, EM_SOCKET_EVENT ent, void* context)
{
    bool need_close = false;
    switch (ent)
    {
    case EVENT_W:
        sink_tcp_event_->OnSendReady(socket_fd, context);
        break;
    case EVENT_R:
        while (true) {
            int size = recv(socket_fd, tcp_recv_buf_, TCP_BUF_SIZE, 0);
            if (0 == size) {
                need_close = true;
                break;
            }

            if (0 > size)
            {
                int nError = 0;
                if (IsIgnoreSocketError(nError))
                {
                    break;
                }
                LogDEBUG("recv tcp scoket %d, err is %d", socket_fd, nError);
                need_close = true;
                break;
            }

            sink_tcp_event_->OnReciveData(socket_fd, tcp_recv_buf_, size, context);
        }
        break;
    case EVENT_E:
        need_close = true;
    	break;
    }
    if (!need_close)
    {
        return;
    }
    sink_tcp_event_->OnClose(socket_fd, context);

}
void NetClient::HandleUdpSocketEvent(int socket_fd, EM_SOCKET_EVENT ent, void* context)
{
	bool need_close = false;
    switch (ent)
    {
    case EVENT_W:
        break;
    case EVENT_R: {
            uint32_t ip;
            uint16_t port;
            while (true) {
	            int size = RecvUdpData(socket_fd, udp_recv_buf_, UDP_BUF_SIZE, ip, port);

	            if (0 > size)
	            {
                    
                    int nError = 0;
					if (IsIgnoreSocketError_UDP(nError))
                    {
                        break;
                    }
                    LogDEBUG("recv udp scoket %d, err is %d", socket_fd, nError);
		            need_close = true;
		            break;
	            }

	            sink_udp_event_->OnReciveData(socket_fd, udp_recv_buf_, size, context, ip, port);
            }
        }
        break;
		case EVENT_E:
			need_close = true;
			break;
		}
		if (!need_close)
		{
			return;
		}
		sink_udp_event_->OnClose(socket_fd, context);
}

int NetClient::CreatUdpSocket()
{
	return CreateSocket(false);
}

int NetClient::CreatTcpSocket()
{
	return CreateSocket(true);
}

int NetClient::CreateSocket(bool usingtcp /* = false */)
{
    do {
        int typ = usingtcp ? SOCK_STREAM : SOCK_DGRAM;
        int fd = socket(AF_INET, typ, 0);
        if (fd < 0)
            break;

				if (usingtcp) {
					int val = 1;
					setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&val, sizeof(val));
					//setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&val, sizeof(val)); 
				} else {
					int buf_size = 65535;
					setsockopt ( fd, SOL_SOCKET, SO_RCVBUF, ( const char * ) &buf_size, sizeof ( buf_size ) );
					setsockopt ( fd, SOL_SOCKET, SO_SNDBUF, ( const char * ) &buf_size, sizeof ( buf_size ) );
				}

#ifndef WIN32
                //recv timeout
                struct timeval timeout={15,0};//15s
                setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#endif

#ifdef IOS
                int set = 1;  
                setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));  
#endif

        if (!SetNonblocking(fd, true)) {
            close_socket(fd);
            break;
        }

        return fd;
    } while (0);

	return INVALID_SOCKET;
}

int NetClient::Bind(int fd, uint32_t local_ip, uint16_t &local_port)
{
	int ret = 0;
 	if (local_port != 0) {
		struct sockaddr_in local;
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = htonl(local_ip);//htonl(local_ip);
		local.sin_port = htons(local_port);
		if ((ret = ::bind(fd, (struct sockaddr*)&local, sizeof(local))) < 0) {
            //do nothing.close outside
		}
	} else {
		uint16_t port = 0;
		if (!getSourcePort(fd, local_ip, port)) {
            //do nothing.close outside
			return -1;
		}
		local_port = port;
	}

	return ret;
}

int NetClient::Connect(int fd, void* context, uint32_t remote_ip, uint16_t remote_port)
{
    if (fd == INVALID_SOCKET)
    {
        LogERR("connect is error, error code: INVALID_SOCKET");
        return -1;
    }

	struct sockaddr_in remote;
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = htonl(remote_ip);
	remote.sin_port = htons(remote_port);

	int ret = ::connect(fd, (struct sockaddr*)(&remote), sizeof (remote));
    int nError = 0;
    if (ret < 0 && !IsIgnoreSocketError(nError))
    {
        //LogERR("connect is error, error code: %d", nError);
        return -1;
    }

    lock_connecting_map_.Lock();
    connecting_map_[fd] = context;
    lock_connecting_map_.UnLock();

    Wakeup();
	return 0;
}

void NetClient::SetSocketContext(int socket_fd, void* context, int socket_typ)
{
	if (0 == socket_typ) {
		lock_tcp_map_.Lock();
		tcp_socket_map_[socket_fd] = context;
		lock_tcp_map_.UnLock();
	} else if (1 == socket_typ) {
		lock_udp_map_.Lock();
		udp_socket_map_[socket_fd] = context;
		lock_udp_map_.UnLock();
	}
	Wakeup();
}

int NetClient::SendTcpData(int socket_fd, char* data, uint32_t data_len)
{
	int sended_bytes = ::send(socket_fd, data, data_len, 0);
    if (-1 == sended_bytes)
    {
        int nError = 0;
        if (IsIgnoreSocketError(nError))
        {
            //wake up select, add tcp write event
            Wakeup();
        }
        LogDEBUG("NetClient tcp socket(%d) send error: %d", socket_fd, nError);
        return sended_bytes;
    }
    if (sended_bytes < data_len)
    {
        //wake up select, add tcp write event
        Wakeup();
    }

    return sended_bytes;
}

int NetClient::SendUdpData(int socket_fd, char* data, uint32_t data_len,
	uint32_t remote_ip, uint16_t remote_port)
{
	struct sockaddr_in to;
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = htonl(remote_ip);
	to.sin_port = htons(remote_port);
	return ::sendto(socket_fd, data, data_len, 0, (struct sockaddr*)&to, sizeof (to));
}

int NetClient::RecvTcpData(int socket_fd, char* data, uint32_t data_len)
{
	return ::recv(socket_fd, data, data_len, 0);
}

int NetClient::RecvUdpData(int socket_fd, char* data, int data_len,
														 uint32_t &remote_ip, uint16_t &remote_port)
{
	struct sockaddr_in from;
    socklen_t len = sizeof(from);
    int ret = ::recvfrom(socket_fd, data, data_len, 0, (struct sockaddr*)&from, &len);
	remote_ip = ntohl(from.sin_addr.s_addr);
	remote_port = ntohs(from.sin_port);

	return ret;
}

int NetClient::Wakeup()
{
	char d = 'd';
	return SendUdpData(notify_socket_fd_, &d, sizeof (char), ntohl(inet_addr("127.0.0.1")), notify_socket_port_);
}

int NetClient::WakeUpEvent()
{
    char d = '1';
    return SendUdpData(notify_socket_fd_, &d, sizeof (char), ntohl(inet_addr("127.0.0.1")), notify_socket_port_);
}

void NetClient::CloseSocket(int socket_fd)
{
	//FD_CLR
    lock_tcp_map_.Lock();
	std::map<int, void*>::iterator it_t = tcp_socket_map_.find(socket_fd);
	if (it_t != tcp_socket_map_.end()) 
    {
		tcp_socket_map_.erase(socket_fd);
	} 
    lock_tcp_map_.UnLock();

    lock_udp_map_.Lock();
	std::map<int, void*>::iterator it_u = udp_socket_map_.find(socket_fd);
	if (it_u != udp_socket_map_.end()) {
		udp_socket_map_.erase(socket_fd);
	}
    lock_udp_map_.UnLock();

    lock_connecting_map_.Lock();
    std::map<int, void*>::iterator iItr = connecting_map_.find(socket_fd);
    if( iItr != connecting_map_.end() )
    {
        connecting_map_.erase(socket_fd);
    }
    lock_connecting_map_.UnLock();
    
    //LogDEBUG("close scoket %d", socket_fd);
	close_socket(socket_fd);
    Wakeup();
}

bool NetClient::SetNonblocking(int fd, bool bNb)
{
#ifdef _WIN32
    unsigned long l = bNb ? 1 : 0;
    int n = ioctlsocket(fd, FIONBIO, &l);
    if (n != 0) {
        return false;
    }
    return true;
#else
    if (bNb) {
        if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
            return false;
        }
    } else {
        if (fcntl(fd, F_SETFL, 0) == -1) {
            return false;
        }
    }
    return true;
#endif
}

bool NetClient::IsIgnoreSocketError(int &nErrorCode)
{
#ifdef WIN32
	nErrorCode = GetLastError();
	if (WSAETIMEDOUT == nErrorCode
		|| WSAEWOULDBLOCK == nErrorCode
		|| WSAEINPROGRESS == nErrorCode
		|| WSAEINTR == nErrorCode) {
		return true;
	}
#else
	nErrorCode = errno;
	if (EAGAIN == errno 
		|| EINTR == errno 
		|| EINPROGRESS == errno
		|| EWOULDBLOCK == errno) {
		return true;
	}
#endif
	return false;
	}

bool NetClient::IsIgnoreSocketError_UDP(int &nErrorCode)
{
	if (IsIgnoreSocketError(nErrorCode))
	{
		return true;
	}
#ifdef WIN32
	if (WSAECONNRESET == nErrorCode)
	{
		return true;
	}
#else
	if (ECONNRESET == nErrorCode)
	{
		return true;
	}
#endif
	return false;
}

#ifdef _WIN32
bool StartWSA()
{
	WSAData wsaData;

	if( 0 == WSAStartup(MAKEWORD(2,2),&wsaData) ) {
		return true;
	}

	return false;
}
void CleanWSA()
{
	WSACleanup();
}
#else
void SetupSignal() {
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	//sigemptyset()用来将参数set信号集初始化并清空
	if (sigemptyset(&sa.sa_mask) == -1 ||
		sigaction(SIGPIPE, &sa, 0) == -1) {
			exit(-1);
	}
}
#endif

INetEventLoop* CreateNetEventLoop()
{
#ifdef _WIN32
	StartWSA();
#else
	SetupSignal();
#endif

	return new NetClient;
}
void ReleaseNetEventLoop(INetEventLoop*& event_loop)
{
	delete (NetClient*)event_loop;
	event_loop = NULL;

#ifdef _WIN32
	CleanWSA();
#endif
}



