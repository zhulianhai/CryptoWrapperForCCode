#include <string.h>
#include "PackDef.h"
PACK_RELAYSVR_AUTH_REQ::PACK_RELAYSVR_AUTH_REQ()
{
    memset ( this, 0, sizeof ( PACK_RELAYSVR_AUTH_REQ ) );
}

bool PACK_RELAYSVR_AUTH_REQ::serial ( char *out_buf, uint16_t out_buf_len, uint16_t &data_len, uint16_t &cmd_id )
{
    //Êä³öcmd_id
    cmd_id = PACK_CMD_ID_RELAYSVR_AUTH_REQ;
    if (1000 < datalen_) {
        return false;
    }

    data_len = 0;
    data_len += sizeof(ip_);
    data_len += sizeof(port_);
    data_len += sizeof(datalen_);
    data_len += datalen_;
    if (out_buf_len < data_len)
    {
        return false;
    }

    memcpy ( out_buf, &ip_, sizeof ( ip_ ) );
    out_buf += sizeof ( ip_ );

    memcpy ( out_buf, &port_, sizeof ( port_ ) );
    out_buf += sizeof ( port_ );

    memcpy ( out_buf, &datalen_, sizeof ( datalen_ ) );
    if ( 0 == datalen_ ) {
        return true;
    }

    out_buf += sizeof ( datalen_ );
    memcpy ( out_buf, data_, datalen_ );
    return true;

}

bool PACK_RELAYSVR_AUTH_REQ::unserial ( char *in_buf, uint16_t in_buf_len )
{
    memcpy ( &ip_, in_buf, sizeof ( ip_ ) );
    in_buf += sizeof ( ip_ );
    memcpy ( &port_, in_buf, sizeof ( port_ ) );
    in_buf += sizeof ( port_ );
    memcpy ( &datalen_, in_buf, sizeof ( datalen_ ) );
    if ( 1000 < datalen_ ) {
        return false;
    }
    if ( 0 == datalen_ ) {
        return true;
    }

    in_buf += sizeof ( datalen_ );
    memcpy ( data_, in_buf, datalen_ );
    return true;
}

PACK_RELAYSVR_AUTH_RESP::PACK_RELAYSVR_AUTH_RESP()
{
    memset ( this, 0, sizeof ( PACK_RELAYSVR_AUTH_RESP ) );
}

bool PACK_RELAYSVR_AUTH_RESP::serial ( char *out_buf, uint16_t out_buf_len, uint16_t &data_len, uint16_t &cmd_id )
{
    cmd_id = PACK_CMD_ID_RELAYSVR_AUTH_RESP;
    data_len = sizeof ( success_ ) + sizeof ( reserved_ );
    if ( out_buf_len < data_len ) {
        return false;
    }
    memcpy ( out_buf, &success_, sizeof ( success_ ) );
    out_buf += sizeof ( success_ );
    memcpy ( out_buf, &reserved_, sizeof ( reserved_ ) );
    return true;
}

bool PACK_RELAYSVR_AUTH_RESP::unserial ( char *in_buf, uint16_t in_buf_len )
{
    if ( in_buf_len < ( sizeof ( success_ ) + sizeof ( reserved_ ) ) ) {
        return false;
    }
    memcpy ( &success_, in_buf, sizeof ( success_ ) );
    in_buf += sizeof ( success_ );
    memcpy ( &reserved_, in_buf, sizeof ( reserved_ ) );
    return true;
}

PACK_RELAYSVR_HEARTBEAT_PACKET::PACK_RELAYSVR_HEARTBEAT_PACKET()
{
    memset ( this, 0, sizeof ( PACK_RELAYSVR_HEARTBEAT_PACKET ) );
}

bool PACK_RELAYSVR_HEARTBEAT_PACKET::serial ( char *out_buf, uint16_t out_buf_len, uint16_t &data_len, uint16_t &cmd_id )
{
    cmd_id = PACK_CMD_ID_HEARTBEAT_PACKET;
    data_len = sizeof ( heartbeat_tag_ ) + sizeof ( reserved_ );
    if ( out_buf_len < data_len ) {
        return false;
    }
    memcpy ( out_buf, &heartbeat_tag_, sizeof ( heartbeat_tag_ ) );
    out_buf += sizeof ( heartbeat_tag_ );
    memcpy ( out_buf, &reserved_, sizeof ( reserved_ ) );
    return true;
}

bool PACK_RELAYSVR_HEARTBEAT_PACKET::unserial ( char *in_buf, uint16_t in_buf_len )
{
    if ( in_buf_len < ( sizeof ( heartbeat_tag_ ) + sizeof ( reserved_ ) ) ) {
        return false;
    }
    memcpy ( &heartbeat_tag_, in_buf, sizeof ( heartbeat_tag_ ) );
    in_buf += sizeof ( heartbeat_tag_ );
    memcpy ( &reserved_, in_buf, sizeof ( reserved_ ) );
    return true;
}

PACK_RELAYSVR_RTP_RTCP_PACKET::PACK_RELAYSVR_RTP_RTCP_PACKET()
{
    memset ( this, 0, sizeof ( PACK_RELAYSVR_RTP_RTCP_PACKET ) );
}

bool PACK_RELAYSVR_RTP_RTCP_PACKET::serial ( char *out_buf, uint16_t out_buf_len, uint16_t &data_len, uint16_t &cmd_id )
{
    cmd_id = PACK_CMD_ID_RTP_RTCP_PACKET;
    data_len = data_len_;
    if ( out_buf_len < data_len ) {
        return false;
    }
    memcpy ( out_buf, data_, data_len );
    return true;
}

bool PACK_RELAYSVR_RTP_RTCP_PACKET::unserial ( char *in_buf, uint16_t in_buf_len )
{
    data_len_ = in_buf_len;
    data_ = in_buf;
    //if( in_buf_len > sizeof(data_) ){
    //    return false;
    //}
    //memcpy(data, in_buf, in_buf_len);
    return true;
}

PACK_RELAYSVR_RELAY_CONF_REQ::PACK_RELAYSVR_RELAY_CONF_REQ()
{
    memset ( this, 0, sizeof ( PACK_RELAYSVR_RELAY_CONF_REQ ) );
}

bool PACK_RELAYSVR_RELAY_CONF_REQ::serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id )
{
    cmd_id = PACK_CMD_ID_RELAY_CONF_REQ;
    data_len = sizeof(req_id_);
    if (out_buf_len < data_len)
    {
        return false;
    }

    memcpy ( out_buf, &req_id_, data_len );
    return true;
}

bool PACK_RELAYSVR_RELAY_CONF_REQ::unserial ( char* in_buf, uint16_t in_buf_len )
{
    if (in_buf_len < sizeof(req_id_))
    {
        return false;
    }
    memcpy(&req_id_, in_buf, sizeof(req_id_));
    return true;
}

PACK_RELAYSVR_RELAY_IPMAP_RESP::PACK_RELAYSVR_RELAY_IPMAP_RESP()
{
    memset ( this, 0, sizeof ( PACK_RELAYSVR_RELAY_IPMAP_RESP ) );
}

bool PACK_RELAYSVR_RELAY_IPMAP_RESP::serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id )
{
    cmd_id = PACK_CMD_ID_RELAY_IPMAP_RESP;
    if (count_ > 150)
    {
        return false;
    }
    data_len = sizeof(count_) + count_*sizeof(uint32_t)*2;
    if (out_buf_len < data_len)
    {
        return false;
    }

    memcpy(out_buf, &count_, sizeof(count_));

    if (0 == count_)
    {
        return true;
    }

    uint32_t array_bytes = count_*sizeof(uint32_t);
    memcpy(out_buf + sizeof(count_), wan_ip_, array_bytes);
    memcpy(out_buf + sizeof(count_) + array_bytes, lan_ip_, array_bytes);
    return true;
}

bool PACK_RELAYSVR_RELAY_IPMAP_RESP::unserial ( char* in_buf, uint16_t in_buf_len )
{
    if (in_buf_len < sizeof(count_))
    {
        return false;
    }
    memcpy(&count_, in_buf, sizeof(count_));
    if (count_ > 150)
    {
        return false;
    }
    if (0 == count_)
    {
        return true;
    }

    uint32_t array_bytes = count_*sizeof(uint32_t);
    memcpy(wan_ip_, in_buf+sizeof(count_), array_bytes);
    memcpy(lan_ip_, in_buf+sizeof(count_) + array_bytes, array_bytes);
    return true;
}


