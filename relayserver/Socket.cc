/*
 * =====================================================================================
 *
 *       Filename:  Socket.cc
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/23/2014 11:21:16 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:
 *        Company:
 *
 * =====================================================================================
 */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>

#include "Socket.h"
#include "TraceLog.h"



Socket::Socket ( int sock_type, int fd )
    :sd_save_flag ( 0 ),
     sd_ ( fd )
{
    if ( fd == -1 ) {
        if ( sock_type == NetCommon::CREATE_UDP ) {
            sd_ = ::socket ( AF_INET, SOCK_DGRAM, 0 );
        } else {
            sd_ = ::socket ( AF_INET, SOCK_STREAM, 0 );
        }
    }

    if ( sd_ < 0 ) {
        LogERR ( "socket():%s", strerror ( errno ) );
    }
}
Socket::~Socket()
{
    Close();
}
#if 0
bool Socket::init ( int sock_type )
{
    if ( sock_type == UDP ) {
        sd_ = socket ( AF_INET, SOCK_DGRAM, 0 );
    } else {
        sd_ = socket ( AF_INET, SOCK_STREAM, 0 );
    }
    if ( sd_ < 0 ) {
        syslog ( LOG_ERR, "socket():%s", strerror ( errno ) );
        return false;
    }
    return true;
}
#endif
bool Socket::Bind ( uint32_t ip, int port )
{
    struct sockaddr_in laddr;
    bzero ( &laddr, 0 );
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons ( port );
    laddr.sin_addr.s_addr = htonl ( ip );
    if ( ::bind ( sd_, ( struct sockaddr * ) &laddr, sizeof ( laddr ) ) < 0 ) {
        LogERR ( "bind():%s, sd_ = %d, %u, %u", strerror ( errno ), sd_, ip, port );
        return false;
    }
    return true;
}
#if 0
int Socket::Connect()
{
    return connect ( sd_, ( void * ) &raddr, sizeof ( raddr ) );
}
#endif
void Socket::SetRemoteAddr ( uint32_t raddr, uint16_t port )
{
    raddr_.sin_family = AF_INET;
    raddr_.sin_port = htons ( port );
    //inet_pton(AF_INET, raddr, &raddr_.sin_addr);  //unnormal
    raddr_.sin_addr.s_addr = htonl ( raddr );
}

int Socket::Send ( void *data_buf, size_t len )
{
    int ret = 0;
    if ( ( ret = ::send ( sd_, data_buf, len, 0 ) ) < 0 ) {
        LogERR ( "send(): errno = %d reason:%s", errno, strerror ( errno ) );
        //return -1;
    }
    return ret;
}
int Socket::SendTo ( void *send_buf, size_t buf_size )
{
    int ret = 0;
    if ( raddr_.sin_addr.s_addr == 0 || raddr_.sin_port == 0 ) {
        return -1;
    }

    if ( ( ret = ::sendto ( sd_, send_buf, buf_size, 0, ( struct sockaddr * ) &raddr_, sizeof ( raddr_ ) ) ) < 0 ) {
        //syslog(LOG_ERR,"sento():%s",strerror(errno));
        LogERR ( "errno = %d reason:%s, ip = %s, port = %d\n", errno, strerror ( errno ), ConvertIp(raddr_.sin_addr.s_addr).c_str(),  raddr_.sin_port );
        //return -1;
    }
    return ret;
}

int Socket::Recv ( void *data_buf, size_t buf_len )
{
    int ret = 0;
    if ( ( ret = ::recv ( sd_, data_buf, buf_len, 0 ) ) < 0 ) {
        if ( errno != EINTR && errno != EAGAIN && errno != EPIPE && errno != ECONNRESET ) {
            LogERR ( "errno = %d reason:%s %d\n", errno, strerror ( errno ) , sd_ );
        }
        /*if ( errno==EINTR || errno==EAGAIN ) {
            continue;
        } else {
            break;
        }*/
    }
    return ret;
}

int Socket::RecvFrom ( void *recv_buf, size_t bufsize, struct sockaddr_in *&from_addr )
{
    socklen_t raddr_len = sizeof ( raddr_ );
    from_addr = &raddr_;
    int recv_lenth = 0;
    if ( ( recv_lenth = ::recvfrom ( sd_, recv_buf, bufsize, 0, ( struct sockaddr * ) &raddr_, &raddr_len ) ) < 0 ) {
        /*if ( errno==EINTR || errno==EAGAIN ) {
            continue;
        } else {
            break;
        }*/
    }
    return recv_lenth;
}

int Socket::Close()
{
    if ( sd_ > -1 ) {
        ::close ( sd_ );
        sd_ = -1;
    }

    return 0;
}

void Socket::resetSockFD ( int fd )
{
    if ( sd_ > 0 ) {
        ::close ( sd_ );
    }

    sd_ = fd;
    SetNonblocking();
}

void Socket::SetNonblocking()
{
    if ( sd_ < 0 ) {
        return;
    }
    sd_save_flag = fcntl ( sd_, F_GETFL );
    ::fcntl ( sd_, F_SETFL, sd_save_flag | O_NONBLOCK );
}

void Socket::ResetSdFlag()
{
    ::fcntl ( sd_, F_SETFL, sd_save_flag );
}

int Socket::GetSocket()
{
    return sd_;
}

int Socket::SetSocketOpt ( int level, int optname, const void *optval, socklen_t optlen )
{
    return ::setsockopt ( sd_, level, optname, optval, optlen );
}
int Socket::SetTCPNoDelay()
{
    int nodelay = 1;
    return SetSocketOpt ( IPPROTO_TCP, TCP_NODELAY, ( void * ) &nodelay, sizeof ( int ) );
}


bool Socket::Listen ( int pending )
{
    int res = listen ( sd_, pending );
    if ( !res ) {
        return true;
    } else {
        return false;
    }
}

bool Socket::Connect ( struct sockaddr_in *& raddr )
{
    int res = connect ( sd_, ( struct sockaddr * ) raddr, sizeof ( raddr ) );
    if ( !res ) {
        return true;
    } else {
        return false;
    }
}

int Socket::Accept ( struct sockaddr_in &raddr )
{
    socklen_t raddr_len = sizeof ( struct sockaddr_in );
    int resfd = accept ( sd_, ( struct sockaddr * ) &raddr, &raddr_len );
    if ( resfd < 0 ) {
        //LOG_ERR("accept():%s",strerror ( errno ));
        return -1;
    }
    return resfd;
}

Socket *CreateNonblockSocket ( int sock_type, int sd )
{
    Socket *sock = new Socket ( sock_type, sd );
    sock->SetNonblocking();
    return sock;
}
