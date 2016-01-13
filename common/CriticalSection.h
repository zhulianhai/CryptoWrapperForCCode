
#ifndef __CRISECTION_H
#define __CRISECTION_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#define CRITICAL_SECTION pthread_mutex_t
#endif//_XNIX


class CCriticalSection
{
private:
	//windows OS
#ifdef WIN32
	CRITICAL_SECTION	m_oSection;
#else
	pthread_mutex_t m_hMutex;
#endif


public:
	CCriticalSection()
	{
#ifdef WIN32
		InitializeCriticalSection(&m_oSection);
#else
		//m_hMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
		//pthread_mutex_init(&m_hMutex,NULL);
		pthread_mutexattr_t   attr;   
		pthread_mutexattr_init(&attr);   
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);   
		pthread_mutex_init(&m_hMutex,&attr);
#endif
	};
	~CCriticalSection()
	{
#ifdef WIN32
		DeleteCriticalSection(&m_oSection);
#else
		pthread_mutex_destroy(&m_hMutex);
#endif
	}
	__inline void  Lock()
	{
#ifdef WIN32
		EnterCriticalSection(&m_oSection);
#else
		pthread_mutex_lock(&m_hMutex);
#endif
	}
	__inline void UnLock()
	{
#ifdef WIN32
		LeaveCriticalSection(&m_oSection);
#else
		pthread_mutex_unlock(&m_hMutex);
#endif
	}
    
    //for buffer api
    __inline void  lock()
    {
#ifdef WIN32
        EnterCriticalSection(&m_oSection);
#else
        pthread_mutex_lock(&m_hMutex);
#endif
    }
    __inline void unlock()
    {
#ifdef WIN32
        LeaveCriticalSection(&m_oSection);
#else
        pthread_mutex_unlock(&m_hMutex);
#endif
  }

  __inline CRITICAL_SECTION *getCriticalSection() {
#ifdef _WIN32
    return &m_oSection;
#else
    return &m_hMutex;
#endif
  }
};


class CCriticalAutoLock
{
public:
	CCriticalAutoLock(CCriticalSection& aCriticalSection)
	{
		m_pCriticalSection = &aCriticalSection;
		m_pCriticalSection->Lock();
	};	
	~CCriticalAutoLock(){
		m_pCriticalSection->UnLock();
	}
private:
	CCriticalSection *m_pCriticalSection;
};

#endif // __CRISECTION_H
