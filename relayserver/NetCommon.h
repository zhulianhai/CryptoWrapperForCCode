#ifndef NET_COMMON_H_
#define NET_COMMON_H_

#include <TypeDef.h>
#include <string>
#include <map>
#include <set>
namespace NetCommon
{
#define DEFAULTTIMEOUTSEC 30
static const uint64_t DEFAULTTIMEOUTUSEC = DEFAULTTIMEOUTSEC * 1000 * 1000;
enum CREATE_SOCKET {CREATE_UDP = 0, CREATE_TCP, CREATE_NULL};
enum CHANNEL_STATE {INIT = 0, RW, TIMEOUT, CLOSED};
struct InetAddress { //local byte order
    InetAddress() : ip ( 0 ), port ( 0 ) {}
    InetAddress ( InetAddress &addr ) : ip ( addr.ip ), port ( addr.port ) {}
    InetAddress ( uint32_t ip, uint16_t port ) : ip ( ip ), port ( port ) {}
    uint32_t ip;
    uint16_t port;
};
}
inline std::string ConvertIp(uint32_t ip_little_end)
{
    struct in_addr in;
    in.s_addr = htonl(ip_little_end);
    char ipbuf[32] = {0};
    inet_ntop (AF_INET, &in,  ipbuf, 32);
    return std::string(ipbuf);
}
class IpWhiteList
{
public:
    static IpWhiteList& GetIpWhiteList()
    {
        static IpWhiteList ip_white_list;
        return ip_white_list;
    }
    void AddWhiteIpMap(const char* wan_ip, const char* lan_ip)
    {
        uint32_t uwan_ip = INADDR_NONE;
        uint32_t ulan_ip = INADDR_NONE;
        if (wan_ip != NULL)
        {
            uwan_ip = inet_addr(wan_ip);
        }
        if (lan_ip != NULL)
        {
            ulan_ip = inet_addr(lan_ip);
        }
        
        if (INADDR_NONE != uwan_ip)
        {
            ip_wan_lan_map_[htonl(uwan_ip)] = (INADDR_NONE == ulan_ip ? htonl(uwan_ip) :htonl(ulan_ip));
        }
        if (INADDR_NONE != ulan_ip)
        {
            ip_lan_set_.insert(htonl(ulan_ip));
        }
        
    }
    bool GetLanIp(uint32_t wan_ip, uint32_t& lan_ip)
    {
        std::map<uint32_t, uint32_t>::iterator it = ip_wan_lan_map_.find(wan_ip);
        if (it != ip_wan_lan_map_.end())
        {
            lan_ip = it->second;
            return true;
        }
        if (ip_lan_set_.find(wan_ip) != ip_lan_set_.end())
        {
            lan_ip = wan_ip;
            return true;
        }
        return false;
    }
    const std::map<uint32_t, uint32_t> &GetWanLanMap () const {
        return ip_wan_lan_map_;
    }
    
private:
    IpWhiteList(){}
    IpWhiteList(const IpWhiteList&);
    std::map<uint32_t, uint32_t> ip_wan_lan_map_;
    std::set<uint32_t> ip_lan_set_;
};
namespace TCPCommon
{
struct  DataBuffer {
    char data_[PACK_MAX_LEN];
    uint16_t data_len_;
    uint32_t ip_;
    uint16_t port_;
};

class HandleSend
{

private:
    uint16_t left_senddata_size_;
};

class HandleRecv
{

};
}

#endif //NET_COMMON_H_
