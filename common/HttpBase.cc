#include "HttpBase.h"
const std::string s_strRequstGetHeader = 
    "GET /%s HTTP/1.1\r\n"
    "Host: %s:%u\r\n"
    "Connection: keep-alive\r\n"
    "Referer: http://%s\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/36.0.1985.125 Safari/537.36\r\n"
    "Accept-Encoding: gzip,deflate,sdch\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\n"
    "\r\n";
const std::string s_strRequstPostHeader = 
    "POST /%s HTTP/1.1\r\n"
    "Host: %s:%u\r\n"
    "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.7.6) Gecko/20050225 Firefox/1.0.1\r\n"
    "Content-Type: application/octet-stream\r\n"
    "Connection: Keep-Alive\r\n"
    "Referer: http://%s\r\n"
    "Content-Length: %u\r\n"
    "\r\n";
	
const std::string s_strResponseGetHeader = 
    "HTTP/1.1 %u OK\r\n"
    "Server: nginx/1.6.0\r\n"
    "Content-Length: %u\r\n"
    "Connection: Keep-Alive\r\n"
    "Content-Type: application/octet-stream\r\n"
    "\r\n";

const std::string s_strResponsePostHeader = 
    "HTTP/1.1 %u OK\r\n"
    "Server: nginx/1.6.0\r\n"
    "Connection: Keep-Alive\r\n"
    "Content-Type: application/octet-stream\r\n"
    "\r\n";

//for client
std::string HttpRequstGetHeader( std::string strUri, std::string strHost, unsigned int nPort )
{
	 char nHttpHeader[10240] = {0};
     sprintf(nHttpHeader, s_strRequstGetHeader.c_str(), strUri.c_str(), strHost.c_str(), nPort, strHost.c_str());
	 return std::string(nHttpHeader);
}
std::string HttpRequstPostHeader( std::string strUri, std::string strHost, unsigned int nPort, unsigned int nBodyLength )
{
    char nHttpHeader[10240] = {0};
    sprintf(nHttpHeader, s_strRequstPostHeader.c_str(), strUri.c_str(), strHost.c_str(), nPort, strHost.c_str(), nBodyLength);
    return std::string(nHttpHeader);
}

//for server
std::string HttpResponseGetHeader( unsigned int nResult, unsigned int nBodyLength )
{
    char nHttpHeader[10240] = {0};
    sprintf(nHttpHeader, s_strResponseGetHeader.c_str(), nResult, nBodyLength);
    return std::string(nHttpHeader);
}
std::string HttpResponsePostHeader( unsigned int nResult )
{
    char nHttpHeader[10240] = {0};
    sprintf(nHttpHeader, s_strResponsePostHeader.c_str(), nResult);
    return std::string(nHttpHeader);
}

int SplitHttpHeader( const std::string &strHeader, StringArray &nHeaderLines)
{
    return SplitString(strHeader, "\r\n", nHeaderLines);
}

int SplitString( const std::string &strContent, const std::string &strKey, StringArray &nValues )
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