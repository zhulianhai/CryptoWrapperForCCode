#include "Thread.h"
#include <signal.h>
#include "TraceLog.h"

Thread::Thread ( const Thread::ThreadFunc &func, void *cb_obj )
    : pth_t_ ( 0 ),
//cond_t_(PTHREAD_COND_INITIALIZER),
      func_ ( func ),
      cb_obj_ ( cb_obj ),
      started_ ( false )
{
    pthread_cond_init ( &cond_t_, NULL );
}

Thread::~Thread()
{
    stop();
    pthread_cond_destroy ( &cond_t_ );
}

bool Thread::start()
{
    if ( func_ == NULL || cb_obj_ == NULL ) {
        return false;
    }

    if ( started_ ) {
        return false;
    }

    sigset_t signal_mask;
    sigemptyset ( &signal_mask );
    sigaddset ( &signal_mask, SIGPIPE );
    int rc = pthread_sigmask ( SIG_BLOCK, &signal_mask, NULL );
    if ( rc != 0 ) {
        LogERR ( "block sigpipe error\n" );
    }

    if ( pthread_create ( &pth_t_, NULL, &Thread::startThread, this ) ) {
        return false;
    }

    //wait for create thread finish
    {
        MutexLockGuard l ( mutex_ );
        while ( !started_ ) {
            pthread_cond_wait ( &cond_t_, mutex_.getMutex() );
        }
    }

    return true;
}

void Thread::stop()
{
    join();
}

void *Thread::startThread ( void *obj )
{
    static_cast<Thread *> ( obj )->run();

    return NULL;
}

void Thread::run()
{
    {
        MutexLockGuard l ( mutex_ );
        started_ = true;
        pthread_cond_signal ( &cond_t_ );
    }
    //do {
    func_ ( cb_obj_ );
    //} while (!quit_);
}

int Thread::join()
{
    return pthread_join ( pth_t_, NULL );
}
