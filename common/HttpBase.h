#ifndef HTTPBASE_H__
#define HTTPBASE_H__

#include <iostream>
#include <vector>
using namespace std;

typedef std::vector<std::string> StringArray;

typedef enum tagHttpCode {
    HC_OK = 200,
    HC_NON_AUTHORITATIVE = 203,
    HC_NO_CONTENT = 204,
    HC_PARTIAL_CONTENT = 206,

    HC_MULTIPLE_CHOICES = 300,
    HC_MOVED_PERMANENTLY = 301,
    HC_FOUND = 302,
    HC_SEE_OTHER = 303,
    HC_NOT_MODIFIED = 304,
    HC_MOVED_TEMPORARILY = 307,

    HC_BAD_REQUEST = 400,
    HC_UNAUTHORIZED = 401,
    HC_FORBIDDEN = 403,
    HC_NOT_FOUND = 404,
    HC_PROXY_AUTHENTICATION_REQUIRED = 407,
    HC_GONE = 410,

    HC_INTERNAL_SERVER_ERROR = 500,
    HC_NOT_IMPLEMENTED = 501,
    HC_SERVICE_UNAVAILABLE = 503,
}HttpCode;

typedef enum tagHttpMethod
{
    METHOD_UNKNOWN = 0,
    METHOD_POST,
    METHOD_GET
}HttpMethod;

//for client
std::string HttpRequstGetHeader( std::string strUri, std::string strHost, unsigned int nPort );
std::string HttpRequstPostHeader( std::string strUri, std::string strHost, unsigned int nPort, unsigned int nBodyLength );

//for server
std::string HttpResponseGetHeader( unsigned int nResult, unsigned int nBodyLength );
std::string HttpResponsePostHeader( unsigned int nResult );

int SplitHttpHeader( const std::string &strHeader, StringArray &nHeaderLines);
int SplitString( const std::string &strContent, const std::string &strKey, StringArray &nValues );


#ifndef uint64
    typedef unsigned long long uint64;
#endif

// bytes define
static const uint64 BYTE_PER_KILO   = 1024;
static const uint64 KILO_PER_MEGA   = 1024;
static const uint64 MEGA_PER_GIGA   = 1024;
static const uint64 BYTE_PER_MEGA   = BYTE_PER_KILO * KILO_PER_MEGA;
static const uint64 BYTE_PER_GIGA   = BYTE_PER_MEGA * MEGA_PER_GIGA;
static const uint64 BYTE_PER_TERA   = BYTE_PER_MEGA * BYTE_PER_GIGA;

#endif // HTTPBASE_H__
