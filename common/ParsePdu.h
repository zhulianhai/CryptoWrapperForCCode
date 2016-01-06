#ifndef PARSEPUD_H
#define PARSEPUD_H
#include "TypeDef.h"

#define PDU_HEAD_LEN    8
class PduHandler
{
public:
    virtual void OnHandlePdu(uint16_t cmd_type, char* packet, uint16_t packet_len ,
        uint32_t remote_ip, uint16_t remote_port) = 0;
    virtual void OnSendPdu() = 0;
};

class ParsePdu
{
public:
    bool isOwnProtocol ( char* in_buf, uint16_t inbuf_len );
    bool parsePduHead ( char* in_buf, uint16_t inbuf_len, uint16_t& cmd_type, char*& data, uint16_t& data_len );

    template<class T>
    bool packPduBuf ( T& data, char* out_buf, uint16_t& outbuf_len ) {
        /*if ( outbuf_len < PACK_MAX_LEN ) {
            return false;
        }*/

        uint16_t data_len = 0;
        uint16_t cmd_id = 0;
        if ( !data.serial ( out_buf+8, outbuf_len - 8, data_len, cmd_id ) ) {
            
            return false;
        }
        outbuf_len  = data_len + 8;
        *out_buf = 'S';
        * ( out_buf + 1 ) = 'R';
        * ( out_buf + 2 ) = 0;
        * ( ( uint16_t* ) ( out_buf+4 ) ) = data_len;
        * ( ( uint16_t* ) ( out_buf+6 ) ) = cmd_id;
        return true;
    }
    int packPduBuf(char* data, uint16_t data_len, uint16_t cmd_type, char* out_buf);
};

class PDUSerialBase
{
public:
    virtual bool serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id ) = 0;
};

template<class Pdu>
class PDUSerial: public PDUSerialBase
{
public:
    explicit PDUSerial(Pdu& pdu):pdu_(pdu){}
    virtual bool serial ( char* out_buf, uint16_t out_buf_len, uint16_t& data_len, uint16_t& cmd_id )
    {
        return pdu_.serial ( out_buf, out_buf_len, data_len, cmd_id );
    }
private:
    Pdu& pdu_;
};

#endif // PARSEPUD_H


