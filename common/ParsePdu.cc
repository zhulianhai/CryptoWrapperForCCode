#include "ParsePdu.h"
#include <string.h>

bool ParsePdu::isOwnProtocol ( char *in_buf, uint16_t inbuf_len )
{
    if ( NULL == in_buf ) {
        return false;
    }
    if ( inbuf_len < 8 ) {
        return false;
    }
    if ( 'S' != *in_buf ) {
        return false;
    }
    if ( 'R' != * ( in_buf + 1 ) ) {
        return false;
    }

    return true;
}

bool ParsePdu::parsePduHead ( char *in_buf, uint16_t inbuf_len, uint16_t &cmd_type, char*& data, uint16_t &data_len )
{
    if ( !isOwnProtocol ( in_buf, inbuf_len ) ) {
        return false;
    }
    if ( 0 != * ( in_buf + 2 ) ) { //ver
        return false;
    }
    data_len = * ( ( uint16_t * ) ( in_buf + 4 ) );
    if ( data_len > PACK_MAX_LEN - 8 ) {
        return false;
    }
    //only for udp
    /*if ( inbuf_len != data_len + 8 ) {
        return false;
    }*/
    cmd_type = * ( ( uint16_t * ) ( in_buf + 6 ) );
    data = in_buf + 8;
    return true;
}

int ParsePdu::packPduBuf(char* data, uint16_t data_len, uint16_t cmd_id, char* out_buf)
{
    *out_buf = 'S';
    * ( out_buf + 1 ) = 'R';
    * ( out_buf + 2 ) = 0;
    * ( ( uint16_t* ) ( out_buf+4 ) ) = data_len;
    * ( ( uint16_t* ) ( out_buf+6 ) ) = cmd_id;
    memcpy(out_buf + PDU_HEAD_LEN, data, data_len);
    return data_len + PDU_HEAD_LEN;
}
