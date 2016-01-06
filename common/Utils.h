#ifndef __UTILS_H__
#define __UTILS_H__
#include <iostream>
#include <map>
#include <string>
#include <list>
#include <set>
#include <vector>
#include <assert.h>
#include "TypeDef.h"
#include "CriticalSection.h"

namespace Utils
{
    //for time 
    static const int64_t kNumMillisecsPerSec = 1000;
    static const int64_t kNumMicrosecsPerSec = 1000 * 1000;
    static const int64_t kNumNanosecsPerSec = 1000 * 1000 * 1000;

    static const int64_t kNumMicrosecsPerMillisec = kNumMicrosecsPerSec / kNumMillisecsPerSec;
    static const int64_t kNumNanosecsPerMillisec =  kNumNanosecsPerSec / kNumMillisecsPerSec;

    // Returns the current time in milliseconds.
    uint32_t Time();
    // Returns the current time in nanoseconds.
    uint64_t TimeNanos();

    typedef std::vector<std::string> StringArray;
    class String
    {
    public:
        // as sprintf format %s,%d,%u,%I64d,%lld ...
        static std::string  Format( const char *pszFormat, ... );
        static std::string  FormatV( const char *pszFormat, va_list pArgList );
        static std::string &AppendFormat( std::string &strOrg, const char *pszFormat, ... );
        static int Split( const std::string &strContent, Utils::StringArray &nValues, const std::string &strKey );
    };
}

template<class T>
class DelayRelease
{
private:
    typedef struct tagDelayNode 
    {
        T* obj_;
        uint32_t timestamp_; 
        tagDelayNode(T* obj)
        {
            obj_ = obj;
            timestamp_ = Utils::Time(); 
        }
    }DelayNode;
public:
    void Attach(T* obj)
    {
        node_lst_lock.Lock();
        delay_node_lst_.push_back(DelayNode(obj));
        node_lst_lock.UnLock();
    }

    void RelayExpireObj(uint32_t delay_ms)
    {
        uint32_t now = Utils::Time();
        node_lst_lock.Lock();
        while(!delay_node_lst_.empty())
        {
            if (now - delay_node_lst_.begin()->timestamp_ < delay_ms)
            {
                break;
            }
            delete delay_node_lst_.begin()->obj_;
            delay_node_lst_.pop_front();
        }
        node_lst_lock.UnLock();
    }
private:
    std::list<DelayNode> delay_node_lst_;
    CCriticalSection node_lst_lock;
};
#endif