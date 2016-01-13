#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include "Mutex.h"

class Thread
{

public:
    typedef void ( *ThreadFunc ) ( void *obj );

    explicit Thread ( const ThreadFunc &func, void *cb_obj );
    virtual ~Thread();

    bool start();
    void stop();

    Mutex &getMutex() {
        return mutex_;
    }

private:
    static void *startThread ( void *obj );

    void run();
    int join();

private:
    pthread_t pth_t_;
    pthread_cond_t cond_t_;
    Mutex mutex_;

    ThreadFunc func_;
    void *cb_obj_;
    bool started_;
};

#endif // THREAD_H
