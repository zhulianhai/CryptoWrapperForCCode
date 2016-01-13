#ifndef __TCP_PARSEPUD_H__
#define __TCP_PARSEPUD_H__

#include "ParsePdu.h"
#include "CriticalSection.h"
#include <string.h>
#define TCP_MAX_BUF_LEN 1024*1024*2

class TcpParsePdu: public ParsePdu
{
public:
    TcpParsePdu()
    {
        Reset();
    }

    void Reset()
    {
        cur_pos_ = 0;
        memset(recv_buf_, 0, sizeof(recv_buf_));

        CCriticalAutoLock guard(send_buf_lock_);
        cur_send_pos = 0;
        memset(send_buf_, 0, sizeof(send_buf_));
    }

    template<class TcpPacketHandler>
    bool ParseTcpPdu(char* in_buf, uint16_t inbuf_len, TcpPacketHandler* tcp_packet_handler)
    {
        if (cur_pos_ + inbuf_len > TCP_MAX_BUF_LEN)
        {
            //recv buf overflow
            return false;
        }

        uint16_t data_len = 0;
        uint16_t cmd_id = 0;
        memcpy(recv_buf_+cur_pos_, in_buf, inbuf_len);
        cur_pos_ += inbuf_len;
        while(true)
        {
            if (cur_pos_ < PDU_HEAD_LEN)
            {
                return true;
            }

            data_len = * ( ( uint16_t * ) ( recv_buf_ + 4 ) );

            if (cur_pos_ < uint32_t(PDU_HEAD_LEN + data_len))
            {
                return true;
            }

            cmd_id = * ( ( uint16_t * ) ( recv_buf_ + 6 ) );
            tcp_packet_handler->OnHandleTcpPdu(cmd_id, recv_buf_+PDU_HEAD_LEN , data_len);
            cur_pos_ -= (PDU_HEAD_LEN + data_len);
            if (cur_pos_ > 0)
            {
                memmove(recv_buf_, recv_buf_ + PDU_HEAD_LEN + data_len, cur_pos_);
            }
        }
    }

    template<class TcpSender>
    bool SendTcpPdu(char* in_buf, uint16_t inbuf_len, uint16_t cmd_type, TcpSender* tcp_sender)
    {
        CCriticalAutoLock guard(send_buf_lock_);
        if (cur_send_pos + PDU_HEAD_LEN + inbuf_len > TCP_MAX_BUF_LEN )
        {
            //send buf overflow
            return false;
        }

        packPduBuf(in_buf, inbuf_len, cmd_type, send_buf_ + cur_send_pos);
        cur_send_pos += PDU_HEAD_LEN + inbuf_len;

        FlushSendBuf(tcp_sender);
        return true;
    }
    template<class TcpSender>
    bool SendTcpPdu(PDUSerialBase& pdu_serial, TcpSender* tcp_sender)
    {
        CCriticalAutoLock guard(send_buf_lock_);
        if (cur_send_pos >= TCP_MAX_BUF_LEN)
        {
            return false;
        }

        uint16_t data_buf_len = TCP_MAX_BUF_LEN - cur_send_pos;
        if (!packPduBuf(pdu_serial, send_buf_ + cur_send_pos, data_buf_len))
        {
            //send buf overflow
            return false;
        }
        cur_send_pos += data_buf_len;

        FlushSendBuf(tcp_sender);
        return true;
    }
    template<class TcpSender>
    int Flush(TcpSender* tcp_sender)
    {
        CCriticalAutoLock guard(send_buf_lock_);
        return FlushSendBuf(tcp_sender);
    }

    bool isNeedSend()
    {
        CCriticalAutoLock guard(send_buf_lock_);
        return 0 < cur_send_pos && cur_send_pos <= TCP_MAX_BUF_LEN;
    }
private:
    template<class TcpSender>
    int FlushSendBuf(TcpSender* tcp_sender)
    {
		if (0 == cur_send_pos)
		{
			return 0;
		}
        if (cur_send_pos > TCP_MAX_BUF_LEN)
        {
            return -1;
        }

        uint32_t send_len = tcp_sender->Send(send_buf_, cur_send_pos);
        if (0 == send_len)
        {
            return cur_send_pos;
        }
        if (send_len > cur_send_pos)
        {
            return -1;
        }
        cur_send_pos -= send_len;
        if (cur_send_pos > 0)
        {
            memmove(send_buf_, send_buf_+send_len, cur_send_pos);
        }
        return cur_send_pos;
    }
private:
    char recv_buf_[TCP_MAX_BUF_LEN];
    uint32_t cur_pos_;

    CCriticalSection send_buf_lock_;
    char send_buf_[TCP_MAX_BUF_LEN];
    uint32_t cur_send_pos;
};
#endif // PARSEPUD_H


