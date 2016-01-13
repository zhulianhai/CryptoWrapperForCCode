#ifndef TIMER_H
#define TIMER_H

#include "Thread.h"
#include <stdint.h>
#include <sys/time.h>

class SessionManager;

static const int kMicroSecondsPerSecond = 1000 * 1000;
extern uint64_t now_sec();

class TimerCallback
{
public:
    virtual void onTimer ( uint64_t now_sec ) = 0;

protected:
    TimerCallback() {}
    virtual ~TimerCallback() {}
};

class Timer
{
public:
    explicit Timer ( bool use_exist_thread = false );
    virtual ~Timer();

    //typedef void (*TimerCallbackFunc)(void *obj);
    void setTimerCallback ( TimerCallback *obj ) {
        cb_obj_ = obj;
    }

    void readTimerfd();
    void resetTimerfd ( struct timespec &ts );

    static void process ( void *obj );

private:
    int createTimerfd();

    void run();

private:
    int timerfd_;
    Thread *thread_;

    TimerCallback *cb_obj_;
    //TimerCallbackFunc timer_callback_;
};

#endif // TIMER_H
