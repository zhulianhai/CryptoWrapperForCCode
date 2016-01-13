#include "SessionManager.h"
#include "Accept.h"
#include <signal.h>
#include "TraceLog.h"
#include "LpConfig.h" //for config file
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static bool main_loop_start_ = false;
struct _LpConfig *m_pConfig; //for config file

/*static bool openConfig() // for config file
{
    m_pConfig =  lp_config_new ( "relayserver.cfg" );
    if ( !m_pConfig ) {
        return false;
    }
    return true;
}*/

class TimerCallbackProcess: public TimerCallback
{
public:
    TimerCallbackProcess() {
        managers_.clear();
    }
    ~TimerCallbackProcess() {
        managers_.clear();
    }

    void onTimer ( uint64_t sec ) {
        std::vector<SessionManager *>::const_iterator it = managers_.begin();
        for ( ; it != managers_.end(); it++ ) {
            ( *it )->onTimer ( sec );
        }
    }

    void RegisterSessionManager ( SessionManager *sess ) {
        managers_.push_back ( sess );
    }

private:
    std::vector<SessionManager *> managers_;
};

typedef void ( *MAINLOOPFUNC ) ( void *obj );
void mainLoop ( MAINLOOPFUNC func, void *obj )
{
    while ( main_loop_start_ ) {
        func ( obj );
    }
}

static bool daemonize()   //dor daemon
{
    pid_t pid;
    int fd;
    pid = fork();
    if ( pid < 0 ) {
        return false;
    }
    if ( pid > 0 ) {
        exit ( 0 );
    }
    fd = open ( "/dev/null", O_RDWR | O_NOCTTY | O_CLOEXEC );
    if ( fd < 0 ) {
        LogWARNING ( "daemonize open()" );
    } else {
        dup2 ( fd, 0 );
        dup2 ( fd, 1 );
        dup2 ( fd, 2 );
        if ( fd > 2 ) {
            close ( fd );
        }
    }
    setsid();
    umask ( 0 );

    return true;
}

int main ( int argc, char **argv )
{
    ::signal ( SIGPIPE, SIG_IGN );

#if 1
//for config file
    m_pConfig =  lp_config_new ( "relayserver.cfg" );
    if ( !m_pConfig ) {

        printf ( "open config file failed....\n" );
        return false;
    }
#endif

    uint16_t use_log = lp_config_get_int ( m_pConfig, "debug", "log", 0);
    if ( !TraceLog::log().openLog ( use_log > 0 ? "relayserver.log" : NULL, TL_DEBUG, false ) ) {
        return -1;
    }
    LogINFO ( "relay server start...." );

    uint16_t listen_port = lp_config_get_int ( m_pConfig, "relay", "port", 8080 );
    const char *listen_ip = lp_config_get_string ( m_pConfig, "relay", "ip", "0.0.0.0" );
    int kDaemon = lp_config_get_int ( m_pConfig, "daemon", "is_daemon", 0 );

    LogINFO("local ip:%s, local port:%d, daemon:%d", listen_ip, listen_port, kDaemon);


    int kThread_num =  lp_config_get_int ( m_pConfig, "thread", "thread_num", 8 ); //read from cfg???
    if ( kThread_num < 2 ) {
        kThread_num = 2;
    }

    //load ip white list
    int ip_count = lp_config_get_int ( m_pConfig, "white_ip", "count", 0);
    char item_ip[20] = {0};
    
    std::string str_lip;
    int index = 1;
    for(; index <= ip_count; index++)
    {
        sprintf(item_ip, "wan_%d", index);
        std::string str_wip = lp_config_get_string ( m_pConfig, "white_ip", item_ip, "");

        sprintf(item_ip, "lan_%d", index);
        std::string str_lip = lp_config_get_string ( m_pConfig, "white_ip", item_ip, "");
        IpWhiteList::GetIpWhiteList().AddWhiteIpMap(str_wip.c_str(), str_lip.c_str());
        LogINFO("white ip, wan:%s, lan:%s", str_wip.c_str(), str_lip.c_str());
    }

    if ( kDaemon ) {
        daemonize();
    }
    UDPSessionManager udp_sess_manager;
    TCPSessionManager tcp_sess_manager;
    udp_sess_manager.startEventloopThread ( kThread_num / 2 );
    tcp_sess_manager.startEventloopThread ( kThread_num / 2 );

    InetAddress listen_addr;
    listen_addr.ip = inet_addr ( listen_ip );
    listen_addr.port = listen_port;
    Accept *udp_accept = Accept::createAccept ( listen_addr, &udp_sess_manager, Accept::T_UDP );
    udp_sess_manager.setAccept ( udp_accept );

    Accept *tcp_accept = Accept::createAccept ( listen_addr, &tcp_sess_manager, Accept::T_TCP );
    tcp_sess_manager.setAccept ( tcp_accept );

    main_loop_start_ = true;
    //thread::loop(); write log???
    TimerCallbackProcess t_proc;
    t_proc.RegisterSessionManager ( &udp_sess_manager );
    t_proc.RegisterSessionManager ( &tcp_sess_manager );
    Timer timer ( true );
    timer.setTimerCallback ( &t_proc );

    //sleep(2);
    udp_accept->start();
    tcp_accept->start();

    mainLoop ( &Timer::process, &timer );

    Accept::deleteAccept ( udp_accept );
    Accept::deleteAccept ( tcp_accept );
    TraceLog::log().closeLog();
    lp_config_destroy ( m_pConfig );
    return 0;
}
