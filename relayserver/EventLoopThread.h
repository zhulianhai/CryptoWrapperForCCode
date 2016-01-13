#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include "Thread.h"
#include "Poll.h"

#include <vector>

class Channel;

class EventLoopThread
{

public:
    enum CHANNEL_OPERATE {O_ADD = 0, O_MOD, O_DEL};
    typedef void ( *PROCESSEVENTFUNC ) ( Channel *obj );
    EventLoopThread();
    virtual ~EventLoopThread();

    bool start();
    void stop();

    void updateChannel ( Channel *chl, CHANNEL_OPERATE opt );
    void postTimeoutChannel ( ChannelBase *chl ) {
        poll_.postTimeoutChannel ( chl );
    }

private:
    static void process ( void *obj );

    void run();
    static void doActiveChannel ( Channel *obj );

private:
    Thread thread_;
    Poll poll_;
};

#endif // EVENTLOOPTHREAD_H
