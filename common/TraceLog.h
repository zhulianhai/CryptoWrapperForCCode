#ifndef _LOGE__H_
#define _LOGE__H_
#include <stdarg.h>
#include "TypeDef.h"
#define TL_EMERG      7
#define TL_ALERT      6
#define TL_CRIT       5
#define TL_ERR        4
#define TL_WARNING    3
#define TL_NOTICE     2
#define TL_INFO       1
#define TL_DEBUG      0

#ifndef _WIN32 //add by tjh
#define LOCATION __FILE__, __LINE__, __func__
#endif

class TraceLog
{
public:
    static TraceLog& log();
    bool openLog ( const char* filename, int min_loglevel, bool is_stdout );
    void closeLog();
    void traceLog ( const char* curr_file, int line_no, int loglevel, const char * format, ... );

    //other way to logging
    void totalLog(const char* file_name, const char * format, ...);

private:
    FILE* log_file_;
    int min_log_level_;
    bool print_stdout_;
private:
    TraceLog() {
        min_log_level_ = 0;
        log_file_ = NULL;
        print_stdout_ =false;
    }
    TraceLog ( const TraceLog& );
};
#define LOG(level, format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, level, format, ##__VA_ARGS__)
#define LogEMERG(format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, TL_EMERG, format, ##__VA_ARGS__)
#define LogALERT(format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, TL_ALERT, format, ##__VA_ARGS__)
#define LogCRIT(format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, TL_CRIT, format, ##__VA_ARGS__)
#define LogERR(format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, TL_ERR, format, ##__VA_ARGS__)
#define LogWARNING(format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, TL_WARNING, format, ##__VA_ARGS__)
#define LogNOTICE(format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, TL_NOTICE, format, ##__VA_ARGS__)
#define LogINFO(format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, TL_INFO, format, ##__VA_ARGS__)
#define LogDEBUG(format, ...) TraceLog::log().traceLog(__FILE__, __LINE__, TL_DEBUG, format, ##__VA_ARGS__)

#endif
