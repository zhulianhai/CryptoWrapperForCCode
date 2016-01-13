#ifndef TYPEDEF_H
#define TYPEDEF_H

#ifndef WIN32
#include <stdint.h>
#endif

#include <stdio.h>
#ifndef OVERRIDE
#define OVERRIDE
#endif

#define PACK_MAX_LEN 1500
#define UDP_BUF_SIZE PACK_MAX_LEN
#define TCP_BUF_SIZE PACK_MAX_LEN*10

#ifdef WIN32
typedef char                int8_t;
typedef short               int16_t;
typedef int                 int32_t;
typedef unsigned __int64    uint64_t;
typedef long long           int64_t;
typedef unsigned int        uint32_t;
typedef unsigned short      uint16_t;
typedef unsigned char       uint8_t;
#endif
#ifndef NULL
#define NULL 0
#endif

#define SAFE_DELETE(x) {if (x != NULL) {delete x; x = NULL;}}

#define OUT
#define IN

//will use Thread::Sleep
//#ifndef _WIN32
//#define Sleep(x) usleep((x)*1000)
//#endif




#endif // UDPCONNECTION_H
