#include "EventLoopThread.h"

#include "Channel.h"

const int kPollTimeMs = 5000;

EventLoopThread::EventLoopThread()
    : thread_ ( EventLoopThread::process, this ),
      poll_ ( thread_.getMutex() )
{

}

EventLoopThread::~EventLoopThread()
{

}

bool EventLoopThread::start()
{
    return thread_.start();
}

void EventLoopThread::stop()
{
    poll_.stop();
    thread_.stop();
}

void EventLoopThread::updateChannel ( Channel *chl, CHANNEL_OPERATE opt )
{
    switch ( opt ) {
    case O_ADD:
        poll_.pollAddChannel ( chl );
        break;
    case O_MOD:
        poll_.pollModChannel ( *chl, chl->getPostEvents() );
        break;
    case O_DEL:
        poll_.pollDelChannel ( *chl );
        break;
    default:
        break;
    }
}

void EventLoopThread::process ( void *obj )
{
    static_cast<EventLoopThread *> ( obj )->run();
}

void EventLoopThread::run()
{
    poll_.poll ( kPollTimeMs );
}

