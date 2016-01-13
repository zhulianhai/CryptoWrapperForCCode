#include <stdio.h>
#include <time.h>
#include <string.h>
#include "TraceLog.h"
#ifdef ANDROID
#include <android/log.h>
#endif

TraceLog &TraceLog::log()
{
    static TraceLog log;
    return log;
}
bool TraceLog::openLog ( const char *filename, int min_loglevel, bool is_stdout )
{
    print_stdout_ = is_stdout;
    min_log_level_ = min_loglevel;
    
    if (filename != NULL)
    {
        log_file_ = fopen ( filename, "a+" );
    }

    return true;
}
void TraceLog::closeLog()
{
    if ( log_file_ ) {
        fclose ( log_file_ );
        log_file_ = NULL;
    }
}

void TraceLog::traceLog ( const char *curr_file, int line_no, int loglevel, const char *format, ... )
{
    if (curr_file == NULL || format == NULL)
    {
        return;
    }
    
    if ( !log_file_ && !print_stdout_ ) {
        return;
    }
    if ( loglevel < min_log_level_ ) {
        return;
    }
    const char *short_file = curr_file;
    const char *end1 = ::strrchr ( curr_file, '/' );
    const char *end2 = ::strrchr ( curr_file, '\\' );
    if ( end1 || end2 ) {
        short_file = ( end1 > end2 ) ? end1 + 1 : end2 + 1;
    }

    time_t now = time ( NULL );
    
    char nBuffer[1024] = {0};
#ifdef WIN32
    struct tm* tm_now;
    tm_now = localtime(&now);
    if ( log_file_ )
    {
        fprintf ( log_file_, "%d-%d-%d_%d:%d:%d [%d][%s:%d]->",
            tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
            tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, loglevel, short_file, line_no );
    }
    sprintf(nBuffer, "%d-%d-%d_%d:%d:%d [%d][%s:%d]->",
        tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
        tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, loglevel, short_file, line_no );
    
#else
    struct tm tm_now;
    localtime_r ( &now, &tm_now );
    if ( log_file_ ) 
    {
        fprintf ( log_file_, "%d-%d-%d_%d:%d:%d [%d][%s:%d]->",
            tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
            tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, loglevel, short_file, line_no );
    }

    sprintf(nBuffer, "[TraceLog]%d-%d-%d_%d:%d:%d [%d][%s:%d]->",
        tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
        tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, loglevel, short_file, line_no );
    
#endif

    va_list aplist;
    va_start ( aplist, format );
    if ( log_file_ ) 
    {
        vfprintf ( log_file_, format, aplist );
        fprintf ( log_file_, "\n" );
        fflush ( log_file_ );
    }

    if ( print_stdout_ ) 
    {
#ifdef ANDROID
		__android_log_vprint(ANDROID_LOG_DEBUG, "TraceLog", format, aplist);
#elif !defined(LINUX)
        vprintf ( nBuffer, aplist );
        vprintf ( format, aplist );
        printf ( "\n" );
#endif
    }

    va_end ( aplist );
}

void TraceLog::totalLog(const char* file_name, const char * format, ...)
{
    if (file_name == NULL || format == NULL)
    {
        return;
    }

#ifdef WIN32
    FILE *fp = NULL;
    fp = fopen(file_name, "a+");
    if (NULL != fp) {
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_s(&tm_now, &now);
        fprintf(fp, "%d-%d-%d_%d:%d:%d ",
            tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
            tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
        va_list apptr;
        va_start(apptr, format);
        vfprintf(fp, format, apptr);
        fclose(fp);
    }
#elif defined(OSX) || defined(IOS)
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    fprintf(stdout, "[%s]%d-%d-%d_%d:%d:%d ", file_name,
        tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
        tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    va_list apptr;
    va_start(apptr, format);
    vfprintf(stdout, format, apptr);
    fflush(stdout);
    va_end(apptr);
#elif defined(LINUX)
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    fprintf(stdout, "[%s]%d-%d-%d_%d:%d:%d ", file_name,
        tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
        tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    va_list apptr;
    va_start(apptr, format);
    vfprintf(stdout, format, apptr);
    fflush(stdout);
    va_end(apptr);
#elif defined (ANDROID)
    va_list apptr;
    va_start(apptr, format);
    __android_log_vprint(ANDROID_LOG_DEBUG, file_name, format, apptr);
    va_end(apptr);
#endif
}

//
//void LOG(int loglevel, const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(loglevel, format, apptr);
//    va_end(apptr);
//}
//
//void LogEMERG(const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(TL_EMERG, format, apptr);
//    va_end(apptr);
//}
//
//void LogALERT(const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(TL_ALERT, format, apptr);
//    va_end(apptr);
//}
//
//void LogCRIT(const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(TL_CRIT, format, apptr);
//    va_end(apptr);
//}
//
//void LogERR(const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(TL_ERR, format, apptr);
//    va_end(apptr);
//}
//
//void LogWARNING(const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(TL_WARNING, format, apptr);
//    va_end(apptr);
//}
//
//void LogNOTICE(const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(TL_NOTICE, format, apptr);
//    va_end(apptr);
//}
//
//void LogINFO(const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(TL_INFO, format, apptr);
//    va_end(apptr);
//}
//
//void LogDEBUG(const char * format, ...)
//{
//    va_list apptr;
//    va_start(apptr,format);
//    TraceLog::log().traceLog(TL_DEBUG, format, apptr);
//    va_end(apptr);
//}

