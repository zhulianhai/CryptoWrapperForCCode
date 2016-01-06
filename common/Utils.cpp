#include "Utils.h"
#if defined(OSX) || defined(IOS)
#include <sys/time.h>
#include <mach/mach_time.h>
#endif
#if defined(LINUX)
#include <stdarg.h>
#include <time.h>
#endif

#ifdef _WIN32
#include <Mmsystem.h>
#pragma comment( lib,"winmm.lib" )
#endif

uint32_t Utils::Time() {
    return static_cast<uint32_t>(TimeNanos() / kNumNanosecsPerMillisec);
}

uint64_t Utils::TimeNanos() {
    int64_t ticks = 0;
#if defined(OSX) || defined(IOS)
    static mach_timebase_info_data_t timebase;
    if (timebase.denom == 0) {
        // Get the timebase if this is the first time we run.
        // Recommended by Apple's QA1398.
        //VERIFY(KERN_SUCCESS == mach_timebase_info(&timebase));
        KERN_SUCCESS == mach_timebase_info(&timebase);
    }
    // Use timebase to convert absolute time tick units into nanoseconds.
    ticks = mach_absolute_time() * timebase.numer / timebase.denom;
#elif defined(ANDROID) || defined(LINUX)
    struct timespec ts;
    // TODO: Do we need to handle the case when CLOCK_MONOTONIC
    // is not supported?
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ticks = kNumNanosecsPerSec * static_cast<int64_t>(ts.tv_sec) +
        static_cast<int64_t>(ts.tv_nsec);
#elif defined(WIN32)
    static volatile LONG last_timegettime = 0;
    static volatile int64_t num_wrap_timegettime = 0;
    volatile LONG* last_timegettime_ptr = &last_timegettime;
    DWORD now = timeGetTime();
    // Atomically update the last gotten time
    DWORD old = InterlockedExchange(last_timegettime_ptr, now);
    if (now < old) {
        // If now is earlier than old, there may have been a race between
        // threads.
        // 0x0fffffff ~3.1 days, the code will not take that long to execute
        // so it must have been a wrap around.
        if (old > 0xf0000000 && now < 0x0fffffff) {
            num_wrap_timegettime++;
        }
    }
    ticks = now + (num_wrap_timegettime << 32);
    // TODO: Calculate with nanosecond precision.  Otherwise, we're just
    // wasting a multiply and divide when doing Time() on Windows.
    ticks = ticks * kNumNanosecsPerMillisec;
#endif
    return ticks;
}

//string utils
std::string Utils::String::Format( const char *pszFormat, ... )
{
    va_list pArgList;

    va_start(pArgList, pszFormat);
    std::string strRes = String::FormatV(pszFormat, pArgList);
    va_end(pArgList);

    return strRes;
}

std::string Utils::String::FormatV( const char *pszFormat, va_list pArgList )
{
    std::string strRes;
    int   nSize = 0;
    char *pszBuffer = NULL;

#ifdef WIN32
    nSize = _vscprintf(pszFormat, pArgList);
#else
    va_list pArgCopy;
    va_copy(pArgCopy, pArgList);
    nSize = vsnprintf(NULL, 0, pszFormat, pArgCopy);
    va_end(pArgCopy);
#endif //OS_PLATFORM_WINDOWS

    pszBuffer = new char[nSize + 1];
    pszBuffer[nSize] = 0;

    vsprintf(pszBuffer, pszFormat, pArgList);

    strRes = pszBuffer;
    delete []pszBuffer;

    return strRes;
}

std::string &Utils::String::AppendFormat( std::string &strOrg, const char *pszFormat, ... )
{
    va_list pArgList;

    va_start(pArgList, pszFormat);
    std::string strAppend = String::FormatV(pszFormat, pArgList);
    va_end(pArgList);

    strOrg.append(strAppend.c_str());
    return strOrg;
}

int Utils::String::Split( const std::string &strContent, Utils::StringArray &nValues, const std::string &strKey )
{
    int nStartPos = 0;
    int nFindPos = 0;
    int nItemCount = 0;
    int nKeyLen = (int)strKey.size();
    int nEndPos = (int)strContent.size();

    nValues.clear();

    if( strKey.size() == 0 ) return 0;
    if( nEndPos <= 0 || nEndPos >= (int)strContent.size() )
    {
        nEndPos = (int)strContent.size();
    }

    while( nStartPos < nEndPos )
    {
        nFindPos = (int)strContent.find(strKey.c_str(), nStartPos);
        if( nFindPos < 0 || nFindPos >= nEndPos )
        {
            nFindPos = nEndPos;
        }

        if( nFindPos < nStartPos )
        {
            break;
        }

        if( nFindPos > nStartPos )
        {
            std::string strNewValue = (nFindPos > nStartPos) ? strContent.substr(nStartPos, nFindPos - nStartPos) : std::string("");
            nValues.push_back(strNewValue);
            nItemCount ++;
        }

        nStartPos = nFindPos + nKeyLen;
    }

    return nItemCount;
}

