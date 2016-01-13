#pragma once


#ifdef WIN32
#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#else
#include <pthread.h>	
#endif
#include "TypeDef.h"
#include "CriticalSection.h"

class CBaseThread
{
public:
  CBaseThread();
  ~CBaseThread();
public:
  //Start Thread
  uint8_t BeginThread(uint32_t(*apStartFunc)(void *), void *apParam, unsigned int& uiThreadID);

  //Thread Sleep
  void static Sleep(uint32_t dwMilliseconds);

  //Get system millisecond
  uint64_t static GetSystemTime();

  //Close Thread
  //BOOL Close(WORD awWaitMilliSecond);

  bool IsRunning();

#ifndef _WIN32
  void Join();
#endif

  CCriticalSection &getCriticalSection() { return m_critical_section; }
private:
  struct STRU_THREAD_INFO
  {
    void * mpUserParam;
    uint32_t(*mpThreadFunc)(void *);
  };

  STRU_THREAD_INFO thread_info_;
  bool running_;

  CCriticalSection m_critical_section;

#ifdef _WIN32
  uintptr_t m_hThreadHandle;
  static unsigned int __stdcall Win32ThreadFunc(void * apParam);
#else
  pthread_t m_thread;
  pthread_cond_t m_cond_t;

  static void * LinuxThreadFunc(void * lpParam);
#endif
};


