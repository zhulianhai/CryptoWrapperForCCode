/*
 * =====================================================================================
 *
 *       Filename:  Poll.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/28/2014 11:01:55 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */
#ifndef POLL_H_
#define POLL_H_

#define MAX_EPOLL_LISTEN 1024

#include "Channel.h"

#include <vector>

class Poll
{
public:
    explicit Poll ( Mutex &mutex );
    ~Poll();
    void poll ( uint32_t timeout_ms );
    void pollAddChannel ( Channel * );
    void pollDelChannel ( Channel & );
    void pollModChannel ( Channel &, uint32_t );
    void stop() {
        stop_flag_ = true;
    }
    void postTimeoutChannel ( ChannelBase *chl ) {
        MutexLockGuard l ( mutex_ );
        tmout_chls_.push_back ( chl );
    }
private:
    void doEvent ( unsigned int , struct epoll_event * );
    void doPendingCloseChannels();

    Poll ( const Poll &lhs );
    Poll &operator= ( const Poll &lhs );

private:
    int epfd_;
    bool stop_flag_;

    typedef std::vector<ChannelBase *> TimeoutChls;//std::list
    TimeoutChls tmout_chls_;

    Mutex &mutex_;
};

#endif // POLL_H
