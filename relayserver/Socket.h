/*
 * =====================================================================================
 *
 *       Filename:  socket.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/23/2014 11:21:36 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:
 *        Company:  suirui
 *
 * =====================================================================================
 */

#ifndef SOCKT_H_
#define SOCKT_H_

#include <arpa/inet.h>
#include "NetCommon.h"
//enum sock_type {UDP = 0,TCP};

class Socket
{
public:
    explicit Socket ( int sock_type = NetCommon::CREATE_UDP, int fd = -1 );
    bool Bind ( uint32_t ip, int port );
    //int Connect();
    int Send ( void *data_buf, size_t len );
    int SendTo ( void *send_buf, size_t buf_len );
    int Recv ( void *data_buf, size_t buf_len );
    int RecvFrom ( void *recv_buf, size_t bufsize, sockaddr_in*& from_addr );
    int Close();
    void SetRemoteAddr ( uint32_t raddr, uint16_t port );
    void GetRemodeAddr();
    int GetSocket();
    void SetNonblocking();  //to be called to set non blocking
    void ResetSdFlag();	//to be called after SetNonblocking()
    bool Listen ( int pending );
    bool Connect ( struct sockaddr_in*& raddr );
    int Accept ( sockaddr_in &raddr );
    ~Socket();
    int SetTCPNoDelay();
    int SetSocketOpt ( int level, int optname, const void *optval, socklen_t optlen );
    void resetSockFD ( int fd );

private:
    int sd_save_flag;
    struct sockaddr_in raddr_;
    int sd_;
};
Socket *CreateNonblockSocket ( int sock_type, int sd = -1 );
#endif
