#include "BaseThread.h"
#include <sys/types.h> 
#include <sys/timeb.h>
#include <assert.h>
#include <signal.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseThread::CBaseThread()
  :running_(false)
{
#ifndef _WIN32
  pthread_cond_init(&m_cond_t, NULL);
#endif
}

CBaseThread::~CBaseThread()
{
#ifndef _WIN32
  pthread_cond_destroy(&m_cond_t);
#endif
}

uint8_t CBaseThread::BeginThread(uint32_t(*apStartFunc)(void *), void *apParam, unsigned int& uiThreadID)
{
  //     STRU_THREAD_INFO *lpThreadInfo = NULL;
  //     lpThreadInfo = new struct STRU_THREAD_INFO;
  //     if (NULL == lpThreadInfo)
  //     {
  //         return 0;
  //     }
  if (running_) {
    return 0;
  }

  thread_info_.mpUserParam = apParam;
  thread_info_.mpThreadFunc = apStartFunc;

#ifdef WIN32

  uiThreadID = 0;
  m_hThreadHandle = _beginthreadex(NULL, 0, Win32ThreadFunc, this, 0, &uiThreadID);
  if (-1 == m_hThreadHandle) {
    return 0;
  }

  //TRACE4("CBaseThread::BeginThread, luThreadID = %u !\n", uiThreadID);
  CloseHandle((HANDLE)m_hThreadHandle);
  return 1;

#else
  sigset_t signal_mask;
  sigemptyset(&signal_mask);
  sigaddset(&signal_mask, SIGPIPE);
  int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
  if (rc != 0) {
    //LogERR("block sigpipe error\n");
  }
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (0 != pthread_create(&m_thread, &attr, LinuxThreadFunc, this)) {
    return 0;
  }
  //uiThreadID = (unsigned int)m_thread;
  //wait for create thread finish
  {
    CCriticalAutoLock l(m_critical_section);
    while (!running_) {
      pthread_cond_wait(&m_cond_t, m_critical_section.getCriticalSection());
    }
  }
  return 1;

#endif
}

#ifdef WIN32
unsigned int CBaseThread::Win32ThreadFunc(void * lpParam)
{
  CBaseThread *th = (CBaseThread *)lpParam;
  STRU_THREAD_INFO *lpThreadInfo = &th->thread_info_;

  //ASSERT(lpThreadInfo != NULL);
  //ASSERT(lpThreadInfo->mpThreadFunc != NULL);

  th->running_ = true;

  lpThreadInfo->mpThreadFunc(lpThreadInfo->mpUserParam);

  //if (lpThreadInfo != NULL)
  //{
  //	delete lpThreadInfo;
  //	lpThreadInfo = NULL;
  //}
  th->running_ = false;
  _endthreadex(0);
  return 1;
}

#else

void * CBaseThread::LinuxThreadFunc(void * lpParam)
{
  CBaseThread *th = (CBaseThread *)lpParam;
  struct STRU_THREAD_INFO *lpThreadInfo = &th->thread_info_;//(struct STRU_THREAD_INFO *)lpParam;

  assert(lpThreadInfo != NULL);
  assert(lpThreadInfo->mpThreadFunc != NULL);

  {
    CCriticalAutoLock l(th->m_critical_section);
    th->running_ = true;
    pthread_cond_signal(&th->m_cond_t);
  }

  lpThreadInfo->mpThreadFunc(lpThreadInfo->mpUserParam);

  //if (lpThreadInfo != NULL)
  //{
  //delete lpThreadInfo;
  //lpThreadInfo = NULL;
  //}

  th->running_ = false;

  return NULL;
}
#endif

void CBaseThread::Sleep(uint32_t dwMilliseconds)
{
#ifdef _WIN32
  ::Sleep(dwMilliseconds);
#else
  uint32_t ldwSleep = 1000 * dwMilliseconds;
  usleep(ldwSleep);
#endif
}


uint64_t CBaseThread::GetSystemTime()
{
  struct timeb loTimeb;
  //memset(&loTimeb, 0 , sizeof(timeb));
  ftime(&loTimeb);
  return ((uint64_t)loTimeb.time * 1000) + loTimeb.millitm;
}

bool CBaseThread::IsRunning()
{
  return running_;
}

#ifndef _WIN32
void CBaseThread::Join()
{
  pthread_join(m_thread, NULL);
}
#endif
