#include "Timer.h"
#include "SessionManager.h"

#include <sys/timerfd.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

uint64_t now_sec()
{
    struct timeval tv;
    gettimeofday ( &tv, NULL );
    int64_t seconds = tv.tv_sec;
    return ( seconds /* * kMicroSecondsPerSecond + tv.tv_usec*/ );
}

Timer::Timer ( bool use_exist_thread )
    : timerfd_ ( createTimerfd() ),
      thread_ ( NULL )
{
    if ( !use_exist_thread ) {
        thread_ = new Thread ( Timer::process, this );
        assert ( thread_->start() );
    }
}

Timer::~Timer()
{
    close ( timerfd_ );
    if ( thread_ != NULL ) {
        thread_->stop();
        delete thread_;
    }
}

void Timer::process ( void *obj )
{
    static_cast<Timer *> ( obj )->run();
}

void Timer::run()
{
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    select ( 0, 0, 0, 0, &tv );

    cb_obj_->onTimer ( now_sec() );
}

int Timer::createTimerfd()
{
    int timer_fd = ::timerfd_create ( CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC );
    if ( timer_fd < 0 ) {
        //"Failed in timerfd_create";
    }
    return timer_fd;
}

void Timer::readTimerfd()
{
    uint64_t buf;
    ssize_t n = ::read ( timerfd_, &buf, sizeof buf );

    assert (n == sizeof (buf));
}

void Timer::resetTimerfd ( struct timespec &ts )
{
    struct itimerspec utmr;
    struct itimerspec otmr;
    bzero ( &utmr, sizeof utmr );
    bzero ( &otmr, sizeof otmr );
    utmr.it_value = ts;
    ::timerfd_settime ( timerfd_, 0, &utmr, &otmr );
}

