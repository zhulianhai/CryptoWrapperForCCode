#ifndef PACKDEF_H
#define PACKDEF_H
#include "TypeDef.h"

#define PACK_CMD_ID_RELAYSVR_BASE 100
#define PACK_CMD_ID_RELAYSVR_AUTH_REQ   PACK_CMD_ID_RELAYSVR_BASE + 1
#define PACK_CMD_ID_RELAYSVR_AUTH_RESP  PACK_CMD_ID_RELAYSVR_BASE + 2
#define PACK_CMD_ID_HEARTBEAT_PACKET    PACK_CMD_ID_RELAYSVR_BASE + 3
#define PACK_CMD_ID_RTP_RTCP_PACKET     PACK_CMD_ID_RELAYSVR_BASE + 4
#define PACK_CMD_ID_RELAY_CONF_REQ      PACK_CMD_ID_RELAYSVR_BASE + 5
#define PACK_CMD_ID_RELAY_IPMAP_RESP    PACK_CMD_ID_RELAYSVR_BASE + 6

//response
namespace RESPONSE {
    enum RESPONSE_AUTH {FAILED_AUTH = 0, SUCC_AUTH, NEED_REAUTH, INVALID_ARG};
    enum HEARTBEAT_TAG {HEARTBEAT_REQ, HEARTBEAT_RESP};
}

struct PACK_RELAYSVR_AUTH_REQ {
    PACK_RELAYSVR_AUTH_REQ();
    uint32_t ip_;
    uint16_t port_;
    uint16_t datalen_;
    char data_[1000];

    bool serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id );
    bool unserial ( char* in_buf, uint16_t in_buf_len );
};

struct PACK_RELAYSVR_AUTH_RESP {
    PACK_RELAYSVR_AUTH_RESP();
    uint8_t success_;
    uint64_t reserved_;

    bool serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id );
    bool unserial ( char* in_buf, uint16_t in_buf_len );
};

struct PACK_RELAYSVR_HEARTBEAT_PACKET {
    PACK_RELAYSVR_HEARTBEAT_PACKET();
    uint8_t heartbeat_tag_;
    uint64_t reserved_;

    bool serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id );
    bool unserial ( char* in_buf, uint16_t in_buf_len );
};

struct PACK_RELAYSVR_RTP_RTCP_PACKET {
    PACK_RELAYSVR_RTP_RTCP_PACKET();
    PACK_RELAYSVR_RTP_RTCP_PACKET ( uint16_t len, char *data ) :data_len_ ( len ), data_ ( data ) {}
    uint16_t data_len_;
    char *data_;

    bool serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id );
    bool unserial ( char* in_buf, uint16_t in_buf_len );
};

struct PACK_RELAYSVR_RELAY_CONF_REQ {
    PACK_RELAYSVR_RELAY_CONF_REQ();
    uint16_t req_id_;

    bool serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id );
    bool unserial ( char* in_buf, uint16_t in_buf_len );
};
struct PACK_RELAYSVR_RELAY_IPMAP_RESP {
    PACK_RELAYSVR_RELAY_IPMAP_RESP();
    uint16_t count_;
    uint32_t wan_ip_[150];
    uint32_t lan_ip_[150];

    bool serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id );
    bool unserial ( char* in_buf, uint16_t in_buf_len );
};
#endif // PACKDEF_H


