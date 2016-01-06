/*
 * =====================================================================================
 *
 *       Filename:  Poll.cc
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/28/2014 11:01:47 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */

#include <sys/epoll.h>
#include <sys/poll.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "TraceLog.h"
#include "Poll.h"

Poll::Poll ( Mutex& mutex )
    : epfd_ ( epoll_create1 ( EPOLL_CLOEXEC/*MAX_EPOLL_LISTEN*/ ) ),
      stop_flag_ ( true ),
      mutex_ ( mutex )
{
    LogINFO ( "Create Epoll" );
    if ( epfd_ < 0 ) {
        LogERR ( "epoll_create():%s", strerror ( errno ) );
    }
    tmout_chls_.clear();
}

Poll::~Poll()
{
    if ( epfd_ > 0 ) {
        ::close ( epfd_ );
        epfd_ = -1;
    }
}
void Poll::doEvent ( unsigned int ev_cnt, struct epoll_event *incoming_events )
{
    if ( !incoming_events ) {
        LogWARNING ( "invalided events ptr" );
        return;
    }

    unsigned int i;
    Channel *p = NULL;
    for ( i = 0; i < ev_cnt; i++ ) {
        p = ( Channel * ) ( incoming_events[i].data.ptr );
        p->setEvents ( incoming_events[i].events );
        p->handleEvent();
    }
}
void Poll::pollAddChannel ( Channel *channel_for_add )
{
    if ( channel_for_add == NULL ) {
        return;
    }
    struct epoll_event ev ;
    int fd = channel_for_add->getSocketFD(); //to be added for IO Mutiples
    ev.events = EPOLLIN | POLLPRI; //are you sure ??
    ev.data.ptr = ( void * ) channel_for_add;
    epoll_ctl ( epfd_, EPOLL_CTL_ADD, fd, &ev );
}
void Poll::pollDelChannel ( Channel &channel_for_del )
{
    int fd = channel_for_del.getSocketFD();
    epoll_ctl ( epfd_, EPOLL_CTL_DEL, fd, NULL );
}
void Poll::pollModChannel ( Channel &channel_for_mod, uint32_t state )
{
    int fd = channel_for_mod.getSocketFD();
    struct epoll_event ev;
    ev.events = state;//channel_for_mod.events();
    ev.data.ptr = ( void * ) &channel_for_mod;
    if ( epoll_ctl ( epfd_, EPOLL_CTL_MOD, fd, &ev ) < 0 ) {
        LogERR ( "mod state is %u. errno = %d, reason is %s\n", state, errno, strerror ( errno ) );
    }
}

void Poll::poll ( uint32_t timeout_ms )
{
    struct epoll_event incoming_events[100];
    int event_cnt = 0;

    if ( epfd_ < 0 ) {
        return;
    }
    stop_flag_ = false;
    while ( !stop_flag_ ) {
        event_cnt = epoll_wait ( epfd_, incoming_events, 100, timeout_ms );
        if ( event_cnt > 0 ) {
            doEvent ( event_cnt, incoming_events );
        } else if ( event_cnt == 0 ) {
            //continue;
        } else {
            if ( EINVAL == errno || EFAULT == errno ) {
                LogERR ( "epoll_wait():%s", strerror ( errno ) );
                break;
            }
            if ( EINTR == errno ) {
                continue;
            }
            LogERR ( "epoll_wait():%s", strerror ( errno ) );
        }

        doPendingCloseChannels();
    }
}

void Poll::doPendingCloseChannels()
{
    TimeoutChls chls;
    {
        MutexLockGuard l ( mutex_ );
        chls.swap ( tmout_chls_ );
    }

    for ( std::size_t i = 0; i < chls.size(); i++ ) {
        chls[i]->close();
    }
}
