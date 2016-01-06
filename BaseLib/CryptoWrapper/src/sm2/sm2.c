#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include "sm2.h"

#define SM2_PAD_ZERO TRUE
//#define SM2_PAD_ZERO FALSE

#define SM2_DEBUG   0
/*
#define QBITS 256
#define PBITS 3072
#define MAX_ECC_KEY_LEN		256
#define MAX_ECC_KEY_SIZE	(MAX_ECC_KEY_LEN/8)
*/

struct FPECC{
char *p;
char *a;
char *b;
char *n;
char *x;
char *y;
};

#if SM2_DEBUG

void PrintBuf(unsigned char *buf, int	buflen) 
{
	int i;
	printf("\n");
	printf("len = %d\n", buflen);
	for(i=0; i<buflen; i++) {
  	if (i % 32 != 31)
  	  printf("%02x", buf[i]);
  	  else
  	  printf("%02x\n", buf[i]);
  }
  printf("\n");
  return;
}
void PrintBig(big data) 
{
 int len=0;
 unsigned char buf[10240];
    
 len=big_to_bytes(0,data,(char *)buf,0);
 PrintBuf(buf,len);
}

unsigned char radom[]  = {0x6C,0xB2,0x8D,0x99,0x38,0x5C,0x17,0x5C,0x94,0xF9,0x4E,0x93,0x48,0x17,0x66,0x3F,0xC1,0x76,0xD9,0x25,0xDD,0x72,0xB7,0x27,0x26,0x0D,0xBA,0xAE,0x1F,0xB2,0xF9,0x6F};
unsigned char radom1[] = {0x4C,0x62,0xEE,0xFD,0x6E,0xCF,0xC2,0xB9,0x5B,0x92,0xFD,0x6C,0x3D,0x95,0x75,0x14,0x8A,0xFA,0x17,0x42,0x55,0x46,0xD4,0x90,0x18,0xE5,0x38,0x8D,0x49,0xDD,0x7B,0x4F};
unsigned char randkey[] = {0x83,0xA2,0xC9,0xC8,0xB9,0x6E,0x5A,0xF7,0x0B,0xD4,0x80,0xB4,0x72,0x40,0x9A,0x9A,0x32,0x72,0x57,0xF1,0xEB,0xB7,0x3F,0x5B,0x07,0x33,0x54,0xB2,0x48,0x66,0x85,0x63};
unsigned char randkeyb[]= {0x33,0xFE,0x21,0x94,0x03,0x42,0x16,0x1C,0x55,0x61,0x9C,0x4A,0x0C,0x06,0x02,0x93,0xD5,0x43,0xC8,0x0A,0xF1,0x97,0x48,0xCE,0x17,0x6D,0x83,0x47,0x7D,0xE7,0x1C,0x80};

struct FPECC Ecc256={
"8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3",
"787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498",
"63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A",
"8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7",
"421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D",
"0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2",
};
unsigned char sm2_par_dig[128] = {
0x78,0x79,0x68,0xB4,0xFA,0x32,0xC3,0xFD,0x24,0x17,0x84,0x2E,0x73,0xBB,0xFE,0xFF,
0x2F,0x3C,0x84,0x8B,0x68,0x31,0xD7,0xE0,0xEC,0x65,0x22,0x8B,0x39,0x37,0xE4,0x98,
0x63,0xE4,0xC6,0xD3,0xB2,0x3B,0x0C,0x84,0x9C,0xF8,0x42,0x41,0x48,0x4B,0xFE,0x48,
0xF6,0x1D,0x59,0xA5,0xB1,0x6B,0xA0,0x6E,0x6E,0x12,0xD1,0xDA,0x27,0xC5,0x24,0x9A,
0x42,0x1D,0xEB,0xD6,0x1B,0x62,0xEA,0xB6,0x74,0x64,0x34,0xEB,0xC3,0xCC,0x31,0x5E,
0x32,0x22,0x0B,0x3B,0xAD,0xD5,0x0B,0xDC,0x4C,0x4E,0x6C,0x14,0x7F,0xED,0xD4,0x3D,
0x06,0x80,0x51,0x2B,0xCB,0xB4,0x2C,0x07,0xD4,0x73,0x49,0xD2,0x15,0x3B,0x70,0xC4,
0xE5,0xD7,0xFD,0xFC,0xBF,0xA3,0x6E,0xA1,0xA8,0x58,0x41,0xB9,0xE4,0x6E,0x09,0xA2,
};

#else
/*SM2*/
struct FPECC Ecc256={
"FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF",
"FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC",
"28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93",
"FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123",
"32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7",
"BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0",
};

unsigned char sm2_par_dig[128] = {
0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,
0x28,0xE9,0xFA,0x9E,0x9D,0x9F,0x5E,0x34,0x4D,0x5A,0x9E,0x4B,0xCF,0x65,0x09,0xA7,
0xF3,0x97,0x89,0xF5,0x15,0xAB,0x8F,0x92,0xDD,0xBC,0xBD,0x41,0x4D,0x94,0x0E,0x93,
0x32,0xC4,0xAE,0x2C,0x1F,0x19,0x81,0x19,0x5F,0x99,0x04,0x46,0x6A,0x39,0xC9,0x94,
0x8F,0xE3,0x0B,0xBF,0xF2,0x66,0x0B,0xE1,0x71,0x5A,0x45,0x89,0x33,0x4C,0x74,0xC7,
0xBC,0x37,0x36,0xA2,0xF4,0xF6,0x77,0x9C,0x59,0xBD,0xCE,0xE3,0x6B,0x69,0x21,0x53,
0xD0,0xA9,0x87,0x7C,0xC6,0x2A,0x47,0x40,0x02,0xDF,0x32,0xE5,0x21,0x39,0xF0,0xA0,
};

unsigned char enkey[32] = {
0xB1,0x6B,0xA0,0xDA,0x27,0xC5,0x24,0x9A,0xF6,0x1D,0x6E,0x6E,0x12,0xD1,0x59,0xA5,
0xB6,0x74,0x64,0x34,0xEB,0xD6,0x1B,0x62,0xEA,0xEB,0xC3,0xCC,0x31,0x5E,0x42,0x1D,
};
#endif


#define SEED_CONST 0x1BD8C95A

int sm3_e(unsigned char *userid, int userid_len, unsigned char *xa, int xa_len, unsigned char *ya, int ya_len, unsigned char *msg, int msg_len, unsigned char *e)
{
/*
功能：根据用户ID及公钥，求用于签名或验签的消息HASH值
[输入] userid： 用户ID
[输入] userid_len： userid的字节数
[输入] xa： 公钥的X坐标
[输入] xa_len: xa的字节数
[输入] ya： 公钥的Y坐标
[输入] ya_len: ya的字节数
[输入] msg：要签名的消息
[输入] msg_len： msg的字节数
[输出] e：32字节，用于签名或验签

返回值：
		－1：内存不足
		  0：成功
*/
	unsigned char *buf;
	int userid_bitlen;

	if((xa_len > 32) || (ya_len > 32))
		return -1;

	buf = malloc(2+userid_len+128+32+32);
	if(buf == NULL)
		return -1;

	userid_bitlen = userid_len << 3;
	buf[0] = (userid_bitlen >> 8) & 0xFF;
	buf[1] = userid_bitlen & 0xFF;

	memcpy(buf+2, userid, userid_len);
	memcpy(buf+2+userid_len, sm2_par_dig, 128);

	memset(buf+2+userid_len+128, 0, 64);
	memcpy(buf+2+userid_len+128+32-xa_len, xa, 32);
	memcpy(buf+2+userid_len+128+32+32-ya_len, ya, 32);

	sm3(buf, 2+userid_len+128+32+32, e);
	free(buf);

#if SM2_DEBUG
	printf("sm3_e: ");
	PrintBuf(e, 32);
#endif

	buf = malloc(msg_len+32);
	if(buf == NULL)
		return -1;

	memcpy(buf, e, 32);
	memcpy(buf+32, msg, msg_len);
	sm3(buf, 32+msg_len, e);

	free(buf);

	return 0;

}


/********************************************************/
//               以下是P域上的ECC函数                   //
/*******************************************************/
void sm2_keygen(unsigned char *wx, int *wxlen, unsigned char *wy, int *wylen,unsigned char *privkey, int *privkeylen)
{
/*
功能：生成SM2公私钥对
[输出] wx：   公钥的X坐标，不足32字节在前面加0x00
[输出] wxlen: wx的字节数，32
[输出] wy：   公钥的Y坐标，不足32字节在前面加0x00
[输出] wylen: wy的字节数，32
[输出] privkey：私钥，不足32字节在前面加0x00
[输出] privkeylen： privkey的字节数，32
*/
	struct FPECC *cfig = &Ecc256;
	epoint *g;
    big a,b,p,n,x,y,key1;
    miracl *mip = mirsys(20,0);

    mip->IOBASE = 16;

    p = mirvar(0);
	a = mirvar(0);
    b = mirvar(0);
    n = mirvar(0);
    x = mirvar(0);
    y = mirvar(0);

    key1 = mirvar(0);

    cinstr(p,cfig->p);
	cinstr(a,cfig->a);
    cinstr(b,cfig->b);
	cinstr(n,cfig->n);
	cinstr(x,cfig->x);
    cinstr(y,cfig->y);

	ecurve_init(a,b,p,MR_PROJECTIVE);
    g = epoint_init();
    epoint_set(x,y,0,g);

    irand(time(NULL)+SEED_CONST);
    bigrand(n,key1);
    ecurve_mult(key1,g,g);
    epoint_get(g,x,y);

#if SM2_PAD_ZERO
    *wxlen = big_to_bytes(32, x, (char *)wx, TRUE);
   	*wylen = big_to_bytes(32, y, (char *)wy, TRUE);
	*privkeylen = big_to_bytes(32, key1, (char *)privkey, TRUE);
#else
    *wxlen = big_to_bytes(0, x, (char *)wx, FALSE);
   	*wylen = big_to_bytes(0, y, (char *)wy, FALSE);
	*privkeylen = big_to_bytes(0, key1, (char *)privkey, FALSE);
#endif
	mirkill(key1);
	mirkill(p);
	mirkill(a);
	mirkill(b);
	mirkill(n);
	mirkill(x);
	mirkill(y);
	epoint_free(g);
	mirexit();
}


void sm2_sign(unsigned char *hash,int hashlen,unsigned char *privkey,int privkeylen,unsigned char *cr,int *rlen,unsigned char *cs,int *slen)
{
/*
功能：SM2签名
[输入] hash：    sm3_e()的结果
[输入] hashlen： hash的字节数，应为32
[输入] privkey： 私钥
[输入] privkeylen： privkeylen的字节数
 
[输出] cr：  签名结果的第一部分，不足32字节在前面加0x00。
[输出] rlen：cr的字节数，32
[输出] cs：  签名结果的第二部分，不足32字节在前面加0x00。
[输出] slen：cs的字节数，32
*/
	struct FPECC *cfig = &Ecc256;
    big e, r, s, k;
    big a,b,p,n,x,y,key1;
    epoint *g;
    miracl *mip = mirsys(20, 0);

	mip->IOBASE = 16;
	e = mirvar(0); 
	r = mirvar(0); 
	s = mirvar(0);
	k = mirvar(0);

    p = mirvar(0);
	a = mirvar(0);
    b = mirvar(0);
    n = mirvar(0);
    x = mirvar(0);
    y = mirvar(0);

    key1 = mirvar(0);
    bytes_to_big(privkeylen,(char *)privkey,key1);

    cinstr(p,cfig->p);
	cinstr(a,cfig->a);
    cinstr(b,cfig->b);
	cinstr(n,cfig->n);
	cinstr(x,cfig->x);
    cinstr(y,cfig->y);
	ecurve_init(a,b,p,MR_PROJECTIVE);
    g = epoint_init();
    epoint_set(x,y,0,g);
    
	bytes_to_big(hashlen, (char *)hash, e);
    irand(time(NULL)+SEED_CONST);

sm2_sign_again:
#if SM2_DEBUG
	bytes_to_big(32, (char *)radom, k);
#else
	do
	{
		bigrand(n, k);
	}
	while(k->len == 0);
#endif

	ecurve_mult(k, g, g);
	epoint_get(g, r, r);

#if SM2_DEBUG
	PrintBig(r);	
#endif	
	
	add(e, r, r);
	divide(r, n, n);

#if SM2_DEBUG
	PrintBig(r);	
#endif	

	if(r->len == 0) 
		goto sm2_sign_again;

	add(r, k, a);
	if (compare(a, n) == 0) 
		goto sm2_sign_again;
	
	incr(key1, 1, b);
	xgcd(b,n,b,b,b);

#if SM2_DEBUG
	PrintBig(b);	
#endif	
	
	multiply(r, key1, a);
	divide(a, n, n);

	if(compare(k, a) >= 0)
	{
		subtract(k, a, a);
	}
	else
	{
		subtract(n, a, a);
		add(k, a, a);
	}
	
	mad(a,b,a,n,n,s);
#if SM2_DEBUG
	PrintBig(s);	
#endif	
	
    if(s->len == 0) 
		goto sm2_sign_again;

#if SM2_PAD_ZERO
	*rlen = big_to_bytes(32, r, (char *)cr, TRUE);
	*slen = big_to_bytes(32, s, (char *)cs, TRUE);
#else
	*rlen = big_to_bytes(0, r, (char *)cr, FALSE);
	*slen = big_to_bytes(0, s, (char *)cs, FALSE);
#endif	
	mirkill(e);
	mirkill(r); 
	mirkill(s); 
	mirkill(k); 
	mirkill(p);
	mirkill(a); 
	mirkill(b); 
	mirkill(n); 
	mirkill(x); 
	mirkill(y); 
	mirkill(key1);
    epoint_free(g);
	mirexit();
}

int sm2_verify(unsigned char *hash,int hashlen,unsigned char  *cr,int rlen,unsigned char *cs,int slen, unsigned char *wx,int wxlen, unsigned char *wy,int wylen)
{
/*
功能：验证SM2签名
[输入] hash：    sm3_e()的结果
[输入] hashlen： hash的字节数，应为32
[输入] cr：  签名结果的第一部分
[输入] rlen：cr的字节数
[输入] cs：  签名结果的第二部分。
[输入] slen：cs的字节数
[输入] wx：   公钥的X坐标
[输入] wxlen: wx的字节数，不超过32字节
[输入] wy：   公钥的Y坐标
[输入] wylen: wy的字节数，不超过32字节

返回值：
		0：验证失败
		1：验证通过
*/

	struct FPECC *cfig = &Ecc256;
    big e, r,s,v;
    big a,b,p,n,x,y;
    epoint *g, *w;
	int ret = 0;
    miracl *mip=mirsys(20, 0);

	mip->IOBASE=16;
    p=mirvar(0);
	a=mirvar(0);
    b=mirvar(0);
    n=mirvar(0);
    x=mirvar(0);
    y=mirvar(0);

	e=mirvar(0); 
	r=mirvar(0); 
	s=mirvar(0); 
	v=mirvar(0);

    cinstr(p,cfig->p);
	cinstr(a,cfig->a);
    cinstr(b,cfig->b);
	cinstr(n,cfig->n);
	cinstr(x,cfig->x);
    cinstr(y,cfig->y);

	ecurve_init(a,b,p,MR_PROJECTIVE);
    g=epoint_init();
	w=epoint_init();
    epoint_set(x,y,0,g);
	bytes_to_big(wxlen,(char *)wx,x);
	bytes_to_big(wylen,(char *)wy,y);
	if(!epoint_set(x,y,0,w))
		goto exit_sm2_verify;

    bytes_to_big(hashlen,(char *)hash,e);
	bytes_to_big(rlen,(char *)cr,r);
    bytes_to_big(slen,(char *)cs,s);

	if ((compare(r, n) >= 0)  || (r->len == 0))
		goto exit_sm2_verify;

	if ((compare(s, n) >= 0) || (s->len == 0))
		goto exit_sm2_verify;


	add(s, r, a);
	divide(a, n, n);
	if (a->len == 0)
		goto exit_sm2_verify;

#if SM2_DEBUG
	PrintBig(a);	
#endif	

    ecurve_mult2(s, g, a, w, g);
    epoint_get(g, v, v);
#if SM2_DEBUG
	PrintBig(v);	
#endif	
	

	add(v, e, v);
	divide(v, n, n);
#if SM2_DEBUG
	PrintBig(v);	
#endif	

	if (compare(v, r) == 0) 
		ret = 1;

exit_sm2_verify:

	mirkill(r);  
	mirkill(s);
	mirkill(v);  
	mirkill(e);  
	mirkill(a);  
	mirkill(b);
    mirkill(p);  
	mirkill(n);  
	mirkill(x);
	mirkill(y);
    epoint_free(g); 
	epoint_free(w);
	mirexit();

	return ret;

}

int kdf(unsigned char *zl, unsigned char *zr, int klen, unsigned char *kbuf)
{
/*
return 0: kbuf is 0, unusable
       1: kbuf is OK
*/
	unsigned char buf[70];
	unsigned char digest[32];
	unsigned int ct = 0x00000001;
	int i, m, n;
	unsigned char *p;


	memcpy(buf, zl, 32);
	memcpy(buf+32, zr, 32);

	m = klen / 32;
	n = klen % 32;
	p = kbuf;

	for(i = 0; i < m; i++)
	{
		buf[64] = (ct >> 24) & 0xFF;
		buf[65] = (ct >> 16) & 0xFF;
		buf[66] = (ct >> 8) & 0xFF;
		buf[67] = ct & 0xFF;
		sm3(buf, 68, p);
		p += 32;
		ct++;
	}

	if(n != 0)
	{
		buf[64] = (ct >> 24) & 0xFF;
		buf[65] = (ct >> 16) & 0xFF;
		buf[66] = (ct >> 8) & 0xFF;
		buf[67] = ct & 0xFF;
		sm3(buf, 68, digest);
	}

	memcpy(p, digest, n);

	for(i = 0; i < klen; i++)
	{
		if(kbuf[i] != 0)
			break;
	}

	if(i < klen)
		return 1;
	else
		return 0;

}

int sm2_encrypt(unsigned char *msg,int msglen, unsigned char *wx,int wxlen, unsigned char *wy,int wylen, unsigned char *outmsg)
{
/*
功能：用SM2公钥加密数据。加密结果比输入数据多96字节！
[输入] msg     要加密的数据
[输入] msglen：msg的字节数
[输入] wx：    公钥的X坐标
[输入] wxlen:  wx的字节数，不超过32字节
[输入] wy：    公钥的Y坐标
[输入] wylen:  wy的字节数，不超过32字节

[输出] outmsg: 加密结果，比输入数据多96字节！，C1（64字节）和C3（32字节）保留前导0x00

返回值：
		-1：        加密失败
		msglen+96： 加密成功
*/
	struct FPECC *cfig = &Ecc256;
    big x2, y2, c1, c2, k;
    big a,b,p,n,x,y;
    epoint *g, *w;
	int ret = -1;
	int i;
	unsigned char zl[32], zr[32];
	unsigned char *tmp;
    miracl *mip;


	tmp = malloc(msglen+64);
	if(tmp == NULL)
		return -1;

	mip = mirsys(20, 0);
	mip->IOBASE = 16;

    p=mirvar(0);
	a=mirvar(0);
    b=mirvar(0);
    n=mirvar(0);
    x=mirvar(0);
    y=mirvar(0);

	k=mirvar(0);
	x2=mirvar(0); 
	y2=mirvar(0); 
	c1=mirvar(0); 
	c2=mirvar(0); 

    cinstr(p,cfig->p);
	cinstr(a,cfig->a);
    cinstr(b,cfig->b);
	cinstr(n,cfig->n);
	cinstr(x,cfig->x);
    cinstr(y,cfig->y);

	ecurve_init(a,b,p,MR_PROJECTIVE);
    g=epoint_init();
	w=epoint_init();
    epoint_set(x,y,0,g);
	bytes_to_big(wxlen,(char *)wx,x);
	bytes_to_big(wylen,(char *)wy,y);
	epoint_set(x,y,0,w);

    irand(time(NULL)+SEED_CONST);

sm2_encrypt_again:

#if SM2_DEBUG
	bytes_to_big(32,(char *)radom1,k);
#else
	do
	{
		bigrand(n, k);
	}
	while(k->len == 0);
#endif

	ecurve_mult(k, g, g);
	epoint_get(g, c1, c2);
	big_to_bytes(32, c1, (char *)outmsg, TRUE);
	big_to_bytes(32, c2, (char *)outmsg+32, TRUE);

#if SM2_DEBUG
	PrintBig(c1);	
	PrintBig(c2);	
#endif	

	if(point_at_infinity(w))
		goto exit_sm2_encrypt;

	ecurve_mult(k, w, w);
	epoint_get(w, x2, y2);

#if SM2_DEBUG
	PrintBig(x2);	
	PrintBig(y2);	
#endif	

	big_to_bytes(32, x2, (char *)zl, TRUE);
	big_to_bytes(32, y2, (char *)zr, TRUE);

	if (kdf(zl, zr, msglen, outmsg+64) == 0)
		goto sm2_encrypt_again;

	for(i = 0; i < msglen; i++)
	{
		outmsg[64+i] ^= msg[i];
	}


	memcpy(tmp, zl, 32);
	memcpy(tmp+32, msg, msglen);
	memcpy(tmp+32+msglen, zr, 32);

	sm3(tmp, 64+msglen, &outmsg[64+msglen]);

	ret = msglen+64+32;

exit_sm2_encrypt:

	mirkill(x2);  
	mirkill(y2);  
	mirkill(c1);  
	mirkill(c2);  
	mirkill(k);
	mirkill(a);  
	mirkill(b);
    mirkill(p);  
	mirkill(n);  
	mirkill(x);
	mirkill(y);
    epoint_free(g); 
	epoint_free(w);
	mirexit();
	free(tmp);


	return ret;

}


int sm2_decrypt(unsigned char *msg,int msglen, unsigned char *privkey, int privkeylen, unsigned char *outmsg)
{
/*
功能：用SM2私钥解密数据。解密结果比输入数据少96字节！
[输入] msg     要解密的数据，是sm2_encrypt()加密的结果，不少于96字节。
[输入] msglen：msg的字节数
[输入] privkey： 私钥
[输入] privkeylen： privkeylen的字节数

[输出] outmsg: 解密结果，比输入数据少96字节！

返回值：
		-1：        解密失败
		msglen-96： 解密成功
*/
	struct FPECC *cfig = &Ecc256;
    big x2, y2, c, k;
    big a,b,p,n,x,y,key1;
    epoint *g;

	unsigned char c3[32];
	unsigned char zl[32], zr[32];
	int i, ret = -1;
	unsigned char *tmp;

    miracl *mip;

	if(msglen < 96)
		return 0;

	msglen -= 96;

	tmp = malloc(msglen+64);
	if(tmp == NULL)
		return 0;

	mip = mirsys(20, 0);
	mip->IOBASE = 16;

	x2=mirvar(0); 
	y2=mirvar(0); 
	c=mirvar(0); 
	k = mirvar(0);

    p = mirvar(0);
	a = mirvar(0);
    b = mirvar(0);
    n = mirvar(0);
    x = mirvar(0);
    y = mirvar(0);
    key1 = mirvar(0);

    bytes_to_big(privkeylen,(char *)privkey,key1);
    cinstr(p,cfig->p);
	cinstr(a,cfig->a);
    cinstr(b,cfig->b);
	cinstr(n,cfig->n);
	cinstr(x,cfig->x);
    cinstr(y,cfig->y);
	ecurve_init(a,b,p,MR_PROJECTIVE);
    g = epoint_init();
    
	bytes_to_big(32, (char *)msg, x);
	bytes_to_big(32, (char *)msg+32, y);
    if(!epoint_set(x,y,0,g))
		goto exit_sm2_decrypt;

	if(point_at_infinity(g))
		goto exit_sm2_decrypt;

	ecurve_mult(key1, g, g);
	epoint_get(g, x2, y2);

#if SM2_DEBUG
	PrintBig(x2);	
	PrintBig(y2);	
#endif		
	
	big_to_bytes(32, x2, (char *)zl, TRUE);
	big_to_bytes(32, y2, (char *)zr, TRUE);

	if (kdf(zl, zr, msglen, outmsg) == 0)
		goto exit_sm2_decrypt;

	for(i = 0; i < msglen; i++)
	{
		outmsg[i] ^= msg[i+64];
	}

	memcpy(tmp, zl, 32);
	memcpy(tmp+32, outmsg, msglen);
	memcpy(tmp+32+msglen, zr, 32);

	sm3(tmp, 64+msglen, c3);
#if SM2_DEBUG
	PrintBuf(c3, 32);
#endif		

	if(memcmp(c3, msg+64+msglen, 32) != 0)
		goto exit_sm2_decrypt;

	ret =  msglen;

exit_sm2_decrypt:

	mirkill(x2);  
	mirkill(y2);  
	mirkill(c);  
	mirkill(k);
	mirkill(p);
	mirkill(a); 
	mirkill(b); 
	mirkill(n); 
	mirkill(x); 
	mirkill(y); 
	mirkill(key1);
    epoint_free(g);
	mirexit();
	free(tmp);

	return ret;
}


#if SM2_DEBUG   
#else
/*void enrand(unsigned char *r, int rlen)
{
	int i, k;
	for(i = 0, k =0; i < rlen; i++) 
	{
		r[i] ^= enkey[k];
		k++;
		k %= 32;
	}
}
*/

int enrand(unsigned char *r, int rlen)
{ 
    aes a;
    char key[16];
    char iv[16];
    int  i;


	memcpy(key, enkey, 16);
	memcpy(iv, enkey, 16);

    if (!aes_init(&a,MR_ECB,16,key,iv))
    {
        return 0;
    }

    for(i = 0; i < rlen; i += 16)
    {
        aes_encrypt(&a, (char *)r+i);
    }

    aes_end(&a);
    return 0;
}

int derand(unsigned char *r, int rlen)
{ 
    aes a;
    char key[16];
    char iv[16];
    int i;

	memcpy(key, enkey, 16);
	memcpy(iv, enkey, 16);

    if (!aes_init(&a,MR_ECB,16,key,iv))
    {
        return 0;
    }

    for(i = 0; i < rlen; i += 16)
    {
        aes_decrypt(&a,(char *)r+i);
    }

    aes_end(&a);
    return 0;
}




#endif

void sm2_keyagreement_a1_3(unsigned char *kx1, int *kx1len, 
						   unsigned char *ky1, int *ky1len,
						   unsigned char *ra,  int *ralen
						   )
{
/*
功能：密钥协商的发起方调用此函数产生一对临时公钥(kx1, ky1)和相应的随机数。公钥发送给对方，随机数ra自己保存。
[输出] kx1：   公钥的X坐标，不足32字节在前面加0x00
[输出] kx1len：kx1的字节数，32
[输出] ky1：   公钥的Y坐标，不足32字节在前面加0x00
[输出] ky1len：ky1的字节数，32
[输出] ra:     随机数，不足32字节在前面加0x00
[输出] ralen： ra的字节数，32

返回值：无
	
*/
	struct FPECC *cfig = &Ecc256;
    big k, x1, y1;
    big a,b,p,n,x,y;
    epoint *g;
    miracl *mip = mirsys(20, 0);

	mip->IOBASE = 16;
	k  = mirvar(0); 
	x1 = mirvar(0); 
	y1 = mirvar(0); 

    p = mirvar(0);
	a = mirvar(0);
    b = mirvar(0);
    n = mirvar(0);
    x = mirvar(0);
    y = mirvar(0);


    cinstr(p,cfig->p);
	cinstr(a,cfig->a);
    cinstr(b,cfig->b);
	cinstr(n,cfig->n);
	cinstr(x,cfig->x);
    cinstr(y,cfig->y);
	ecurve_init(a,b,p,MR_PROJECTIVE);
    g = epoint_init();
    epoint_set(x,y,0,g);
    
    irand(time(NULL)+SEED_CONST);

#if SM2_DEBUG
	bytes_to_big(32, (char *)randkey, k);
#else
	do
	{
		bigrand(n, k);
	}
	while(k->len == 0);
#endif


	ecurve_mult(k, g, g);
	epoint_get(g, x1, y1);

#if SM2_PAD_ZERO
	*kx1len = big_to_bytes(32, x1, (char *)kx1, TRUE);
	*ky1len = big_to_bytes(32, y1, (char *)ky1, TRUE);
	*ralen  = big_to_bytes(32, k,  (char *)ra,  TRUE);
#else
	*kx1len = big_to_bytes(0, x1, (char *)kx1, FALSE);
	*ky1len = big_to_bytes(0, y1, (char *)ky1, FALSE);
	*ralen =  big_to_bytes(0, k,  (char *)ra,  FALSE);
#endif	

#if SM2_DEBUG
#else
	enrand(ra, *ralen);
#endif

	mirkill(k);
	mirkill(x1); 
	mirkill(y1); 
	mirkill(p);
	mirkill(a); 
	mirkill(b); 
	mirkill(n); 
	mirkill(x); 
	mirkill(y); 
    epoint_free(g);
	mirexit();
}

int sm3_z(unsigned char *userid, int userid_len, unsigned char *xa, int xa_len, unsigned char *ya, int ya_len, unsigned char *z)
{
/*
功能：根据用户ID及公钥，求Z值
[输入] userid： 用户ID
[输入] userid_len： userid的字节数
[输入] xa： 公钥的X坐标
[输入] xa_len: xa的字节数
[输入] ya： 公钥的Y坐标
[输入] ya_len: ya的字节数
[输出] z：32字节

返回值：
		－1：内存不足
		  0：成功
*/
	unsigned char *buf;
	int userid_bitlen;

	if((xa_len > 32) || (ya_len > 32))
		return -1;

	buf = malloc(2+userid_len+128+32+32);
	if(buf == NULL)
		return -1;

	userid_bitlen = userid_len << 3;
	buf[0] = (userid_bitlen >> 8) & 0xFF;
	buf[1] = userid_bitlen & 0xFF;

	memcpy(buf+2, userid, userid_len);
	memcpy(buf+2+userid_len, sm2_par_dig, 128);

	memset(buf+2+userid_len+128, 0, 64);
	memcpy(buf+2+userid_len+128+32-xa_len, xa, 32);
	memcpy(buf+2+userid_len+128+32+32-ya_len, ya, 32);

	sm3(buf, 2+userid_len+128+32+32, z);
	free(buf);

#if SM2_DEBUG
	printf("sm3_z: ");
	PrintBuf(z, 32);
#endif

	return 0;

}


int kdf_key(unsigned char *z, int zlen, int klen, unsigned char *kbuf)
{
/*
return 0: kbuf is 0, unusable
       1: kbuf is OK
*/
	unsigned char *buf;
	unsigned char digest[32];
	unsigned int ct = 0x00000001;
	int i, m, n;
	unsigned char *p;

	buf = malloc(zlen+4);
	if(buf == NULL)
		return 0;

	memcpy(buf, z, zlen);

	m = klen / 32;
	n = klen % 32;
	p = kbuf;

	for(i = 0; i < m; i++)
	{
		buf[zlen] = (ct >> 24) & 0xFF;
		buf[zlen+1] = (ct >> 16) & 0xFF;
		buf[zlen+2] = (ct >> 8) & 0xFF;
		buf[zlen+3] = ct & 0xFF;
		sm3(buf, zlen+4, p);
		p += 32;
		ct++;
	}

	if(n != 0)
	{
		buf[zlen] = (ct >> 24) & 0xFF;
		buf[zlen+1] = (ct >> 16) & 0xFF;
		buf[zlen+2] = (ct >> 8) & 0xFF;
		buf[zlen+3] = ct & 0xFF;
		sm3(buf, zlen+4, digest);
	}

	memcpy(p, digest, n);

	free(buf);

	return 1;

}
int sm2_keyagreement_b1_9( 
						  unsigned char *kx1, int kx1len,
						  unsigned char *ky1, int ky1len,
						  unsigned char *pax, int paxlen,
						  unsigned char *pay, int paylen,
						  unsigned char *private_b,   int private_b_len,
						  unsigned char *pbx, int pbxlen,
						  unsigned char *pby, int pbylen,
						  unsigned char *ida, int idalen,
						  unsigned char *idb, int idblen,
						  unsigned int  kblen,
						  unsigned char *kbbuf,
						  unsigned char *kx2, int *kx2len,
						  unsigned char *ky2, int *ky2len,
						  unsigned char *xv,  int *xvlen,
						  unsigned char *yv,  int *yvlen,
						  unsigned char *sb
						  )
{
/*

功能：密钥协商的接收方调用此函数协商出密钥kbbuf，同时产生一对临时公钥(kx2, ky2)以及(xv, yv)、sb。(kx2, ky2)和sb发送给对方，kbbuf和(xv, yv)自己保存。
说明：
[输入] (kx1, ky1)是发起方产生的临时公钥
[输入] (pax, pay)是发起方的公钥
[输入] private_b是接收方的私钥
[输入] (pbx, pby)是接收方的公钥
[输入] ida是发起方的用户标识
[输入] idb是接收方的用户标识
[输入] kblen是要约定的密钥字节数

[输出] kbbuf是协商密钥输出缓冲区
[输出] (kx2, ky2)是接收方产生的临时公钥，不足32字节在前面加0x00
[输出] (xv, yv)是接收方产生的中间结果，自己保存，用于验证协商的正确性。，不足32字节在前面加0x00。如果(xv, yv)=NULL，则不输出。
[输出] sb是接收方产生的32字节的HASH值，要传送给发起方，用于验证协商的正确性。如果为sb=NULL，则不输出。


返回值：0－失败  1－成功
	
*/
	struct FPECC *cfig = &Ecc256;
    big k, x1, y1, x2, y2, _x1, _x2, db, tb;
    big a,b,p,n,x,y;
    epoint *g, *w;
	int ret = 0;
	unsigned char kx1buf[32], ky1buf[32];
	unsigned char kx2buf[32], ky2buf[32];
	unsigned char xvbuf[32];
	unsigned char yvbuf[32];
	unsigned char paxbuf[32];
	unsigned char paybuf[32];
	unsigned char buf[256];

	unsigned char za[32];
	unsigned char zb[32];




    miracl *mip = mirsys(20, 0);

	mip->IOBASE = 16;
	k  = mirvar(0); 
	x1 = mirvar(0); 
	y1 = mirvar(0); 
	x2 = mirvar(0); 
	y2 = mirvar(0); 
	_x1 = mirvar(0); 
	_x2 = mirvar(0); 
	tb = mirvar(0); 
	db = mirvar(0); 

    p = mirvar(0);
	a = mirvar(0);
    b = mirvar(0);
    n = mirvar(0);
    x = mirvar(0);
    y = mirvar(0);


    cinstr(p,cfig->p);
	cinstr(a,cfig->a);
    cinstr(b,cfig->b);
	cinstr(n,cfig->n);
	cinstr(x,cfig->x);
    cinstr(y,cfig->y);
	ecurve_init(a,b,p,MR_PROJECTIVE);
	g = epoint_init();
 	w = epoint_init();
	epoint_set(x,y,0,g);
    

	sm3_z(ida, idalen, pax, paxlen, pay, paylen, za);
	sm3_z(idb, idblen, pbx, pbxlen, pby, pbylen, zb);

#if SM2_DEBUG
	printf("za & zb: ");
	PrintBuf(za, 32);
	PrintBuf(zb, 32);
#endif

    irand(time(NULL)+SEED_CONST);

#if SM2_DEBUG
	bytes_to_big(32, (char *)randkeyb, k);
#else
	do
	{
		bigrand(n, k);
	}
	while(k->len == 0);
#endif


	ecurve_mult(k, g, g);
	epoint_get(g, x2, y2);

	big_to_bytes(32, x2, (char *)kx2buf, TRUE);
	big_to_bytes(32, y2, (char *)ky2buf, TRUE);
#if SM2_DEBUG
	printf("RB(x2, y2): ");
	PrintBuf(kx2buf, 32);
	PrintBuf(ky2buf, 32);
#endif

	memcpy(kx2, kx2buf, 32);
	memcpy(ky2, ky2buf, 32);
	*kx2len = 32;
	*ky2len = 32;


	memcpy(buf, kx2buf+16, 16);
	buf[0] |= 0x80;
	bytes_to_big(16, (char *)buf, _x2);
	bytes_to_big(private_b_len, (char *)private_b, db);

#if SM2_DEBUG
	PrintBig(_x2); 
#endif

	mad(_x2, k, db, n, n, tb);
#if SM2_DEBUG
	PrintBig(tb); 
#endif

	bytes_to_big(kx1len, (char *)kx1, x1);
	bytes_to_big(ky1len, (char *)ky1, y1);

    if(!epoint_set(x1,y1,0,g))
		goto exit_sm2_keyagreement_b19;


	big_to_bytes(32, x1, (char *)kx1buf, TRUE);
	big_to_bytes(32, y1, (char *)ky1buf, TRUE);
	memcpy(buf, kx1buf+16, 16);
	buf[0] |= 0x80;
	bytes_to_big(16, (char *)buf, _x1);
#if SM2_DEBUG
	PrintBig(_x1); 
#endif

	bytes_to_big(paxlen, (char *)pax, x);
	bytes_to_big(paylen, (char *)pay, y);
	big_to_bytes(32, x, (char *)paxbuf, TRUE);
	big_to_bytes(32, y, (char *)paybuf, TRUE);

    if(!epoint_set(x, y, 0, w))
		goto exit_sm2_keyagreement_b19;

	ecurve_mult(_x1, g, g);
	ecurve_add(w, g);
	ecurve_mult(tb, g, g);
	if(point_at_infinity(g))
		goto exit_sm2_keyagreement_b19;

	epoint_get(g, x, y);
	big_to_bytes(32, x, (char *)xvbuf, TRUE);
	big_to_bytes(32, y, (char *)yvbuf, TRUE);
#if SM2_DEBUG
	printf("xv & yv: ");
	PrintBuf(xvbuf, 32);
	PrintBuf(yvbuf, 32);
#endif


	memcpy(buf, xvbuf, 32);
	memcpy(buf+32, yvbuf, 32);
	memcpy(buf+64, za, 32);
	memcpy(buf+96, zb, 32);
	kdf_key(buf, 128, kblen, kbbuf);
#if SM2_DEBUG
	printf("Kb: ");
	PrintBuf(kbbuf, kblen);
#endif


	if(sb != NULL)
	{
		memcpy(buf, xvbuf, 32);
		memcpy(buf+32, za, 32);
		memcpy(buf+64, zb, 32);
		memcpy(buf+96, kx1buf, 32);
		memcpy(buf+128, ky1buf, 32);
		memcpy(buf+160, kx2buf, 32);
		memcpy(buf+192, ky2buf, 32);
		sm3(buf, 32*7, sb);
		buf[0] = 0x02;
		memcpy(buf+1, yvbuf, 32);
		memcpy(buf+33, sb, 32);
		sm3(buf, 65, sb);
	}

	if(xv != NULL)
	{
		memcpy(xv, xvbuf, 32);
		*xvlen = 32;
#if SM2_DEBUG
#else
		enrand(xv, *xvlen);
#endif

	}

	if(yv != NULL)
	{
		memcpy(yv, yvbuf, 32);
		*yvlen = 32;
#if SM2_DEBUG
#else
		enrand(yv, *yvlen);
#endif
	}

#if SM2_DEBUG
	printf("Sb: ");
	PrintBuf(sb, 32);
#endif

	ret = 1;

exit_sm2_keyagreement_b19:

	mirkill(k);
	mirkill(x1); 
	mirkill(y1); 
	mirkill(x2); 
	mirkill(y2); 
	mirkill(_x1); 
	mirkill(_x2); 
	mirkill(tb); 
	mirkill(db); 
	mirkill(p);
	mirkill(a); 
	mirkill(b); 
	mirkill(n); 
	mirkill(x); 
	mirkill(y); 
    epoint_free(g);
    epoint_free(w);
	mirexit();

	return ret;
}


int sm2_keyagreement_a4_10( 
						  unsigned char *kx1, int kx1len,
						  unsigned char *ky1, int ky1len,
						  unsigned char *pax, int paxlen,
						  unsigned char *pay, int paylen,
						  unsigned char *private_a,   int private_a_len,
						  unsigned char *pbx, int pbxlen,
						  unsigned char *pby, int pbylen,
						  unsigned char *ida, int idalen,
						  unsigned char *idb, int idblen,
						  unsigned char *kx2, int kx2len,
						  unsigned char *ky2, int ky2len,
						  unsigned char *ra,  int ralen,
						  unsigned int  kalen,
						  unsigned char *kabuf,
						  unsigned char *s1,
						  unsigned char *sa
						  )
{
/*

功能：密钥协商的发起方调用此函数协商出密钥kabuf，同时产生s1和sa。s1和kabuf自己保存，sa发送给接收方，用于确认协商过程的正确性。
说明：
[输入] (kx1, ky1)是发起方产生的临时公钥
[输入] (pax, pay)是发起方的公钥
[输入] private_a是发起方的私钥
[输入] (pbx, pby)是接收方的公钥
[输入] ida是发起方的用户标识
[输入] idb是接收方的用户标识
[输入] (kx2, ky2)是接收方产生的临时公钥
[输入] ra是发起方调用sm2_keyagreement_a1_3产生的随机数
[输入] kalen是要约定的密钥字节数

[输出] kabuf是协商密钥输出缓冲区
[输出] s1和sa是发起方产生的32字节的HASH值，s1自己保存（应等于sb），sa要传送给接收方，用于验证协商的正确性


返回值：0－失败  1－成功
	
*/

	struct FPECC *cfig = &Ecc256;
    big k, x1, y1, x2, y2, _x1, _x2, da, ta;
    big a,b,p,n,x,y;
    epoint *g, *w;
	int ret = 0;
	unsigned char kx1buf[32], ky1buf[32];
	unsigned char kx2buf[32], ky2buf[32];
	unsigned char xubuf[32];
	unsigned char yubuf[32];
	unsigned char buf[256];

	unsigned char za[32];
	unsigned char zb[32];
	unsigned char hash[32];




    miracl *mip = mirsys(20, 0);

	mip->IOBASE = 16;
	k  = mirvar(0); 
	x1 = mirvar(0); 
	y1 = mirvar(0); 
	x2 = mirvar(0); 
	y2 = mirvar(0); 
	_x1 = mirvar(0); 
	_x2 = mirvar(0); 
	ta = mirvar(0); 
	da = mirvar(0); 

    p = mirvar(0);
	a = mirvar(0);
    b = mirvar(0);
    n = mirvar(0);
    x = mirvar(0);
    y = mirvar(0);


    cinstr(p,cfig->p);
	cinstr(a,cfig->a);
    cinstr(b,cfig->b);
	cinstr(n,cfig->n);
	cinstr(x,cfig->x);
    cinstr(y,cfig->y);
	ecurve_init(a,b,p,MR_PROJECTIVE);
	g = epoint_init();
 	w = epoint_init();
    

	sm3_z(ida, idalen, pax, paxlen, pay, paylen, za);
	sm3_z(idb, idblen, pbx, pbxlen, pby, pbylen, zb);


	bytes_to_big(kx1len, (char *)kx1, x1);
	bytes_to_big(ky1len, (char *)ky1, y1);

    if(!epoint_set(x1,y1,0,g))
		goto exit_sm2_keyagreement_a4_10;

	big_to_bytes(32, x1, (char *)kx1buf, TRUE);
	big_to_bytes(32, y1, (char *)ky1buf, TRUE);
	memcpy(buf, kx1buf+16, 16);
	buf[0] |= 0x80;
	bytes_to_big(16, (char *)buf, _x1);
#if SM2_DEBUG
	PrintBig(_x1); 
#endif


	bytes_to_big(private_a_len, (char *)private_a, da);

#if SM2_DEBUG
	bytes_to_big(ralen, (char *)ra, k);
#else
	memcpy(buf, ra, ralen);
	derand(buf, ralen);
	bytes_to_big(ralen, (char *)buf, k);
#endif
	

	mad(_x1, k, da, n, n, ta);
#if SM2_DEBUG
	PrintBig(ta); 
#endif


	bytes_to_big(kx2len, (char *)kx2, x2);
	bytes_to_big(ky2len, (char *)ky2, y2);
    if(!epoint_set(x2,y2,0,g))
		goto exit_sm2_keyagreement_a4_10;

	big_to_bytes(32, x2, (char *)kx2buf, TRUE);
	big_to_bytes(32, y2, (char *)ky2buf, TRUE);
	memcpy(buf, kx2buf+16, 16);
	buf[0] |= 0x80;
	bytes_to_big(16, (char *)buf, _x2);
#if SM2_DEBUG
	PrintBig(_x2); 
#endif

	bytes_to_big(pbxlen, (char *)pbx, x);
	bytes_to_big(pbylen, (char *)pby, y);
    if(!epoint_set(x, y, 0, w))
		goto exit_sm2_keyagreement_a4_10;

	ecurve_mult(_x2, g, g);
	ecurve_add(w, g);
	ecurve_mult(ta, g, g);
	if(point_at_infinity(g))
		goto exit_sm2_keyagreement_a4_10;


	epoint_get(g, x, y);
	big_to_bytes(32, x, (char *)xubuf, TRUE);
	big_to_bytes(32, y, (char *)yubuf, TRUE);
#if SM2_DEBUG
	printf("xu & yu: ");
	PrintBuf(xubuf, 32);
	PrintBuf(yubuf, 32);
#endif


	memcpy(buf, xubuf, 32);
	memcpy(buf+32, yubuf, 32);
	memcpy(buf+64, za, 32);
	memcpy(buf+96, zb, 32);
	kdf_key(buf, 128, kalen, kabuf);
#if SM2_DEBUG
	printf("Ka: ");
	PrintBuf(kabuf, kalen);
#endif

	if((s1 != NULL) || (sa != NULL))
	{
		memcpy(buf, xubuf, 32);
		memcpy(buf+32, za, 32);
		memcpy(buf+64, zb, 32);
		memcpy(buf+96, kx1buf, 32);
		memcpy(buf+128, ky1buf, 32);
		memcpy(buf+160, kx2buf, 32);
		memcpy(buf+192, ky2buf, 32);
		sm3(buf, 32*7, hash);
	}

	if(s1 != NULL)
	{
		buf[0] = 0x02;
		memcpy(buf+1, yubuf, 32);
		memcpy(buf+33, hash, 32);
		sm3(buf, 65, s1);
#if SM2_DEBUG
		printf("S1: ");
		PrintBuf(s1, 32);
#endif
	}

	if(sa != NULL)
	{
		buf[0] = 0x03;
		memcpy(buf+1, yubuf, 32);
		memcpy(buf+33, hash, 32);
		sm3(buf, 65, sa);
#if SM2_DEBUG
		printf("Sa: ");
		PrintBuf(sa, 32);
#endif
	}

	ret = 1;

exit_sm2_keyagreement_a4_10:

	mirkill(k);
	mirkill(x1); 
	mirkill(y1); 
	mirkill(x2); 
	mirkill(y2); 
	mirkill(_x1); 
	mirkill(_x2); 
	mirkill(ta); 
	mirkill(da); 
	mirkill(p);
	mirkill(a); 
	mirkill(b); 
	mirkill(n); 
	mirkill(x); 
	mirkill(y); 
    epoint_free(g);
    epoint_free(w);
	mirexit();

	return ret;
}


void sm2_keyagreement_b10( 
						  unsigned char *pax, int paxlen,
						  unsigned char *pay, int paylen,
						  unsigned char *pbx, int pbxlen,
						  unsigned char *pby, int pbylen,
						  unsigned char *kx1, int kx1len,
						  unsigned char *ky1, int ky1len,
						  unsigned char *kx2, int kx2len,
						  unsigned char *ky2, int ky2len,
						  unsigned char *xv, int xvlen,
						  unsigned char *yv, int yvlen,
						  unsigned char *ida, int idalen,
						  unsigned char *idb, int idblen,
						  unsigned char *s2
						 )
{
/*

功能：密钥协商的接收方调用此函数产生s2，用于验证协商过程的正确性。
说明：
[输入] (pax, pay)是发起方的公钥
[输入] (pbx, pby)是接收方的公钥
[输入] (kx1, ky1)是发起方产生的临时公钥
[输入] (kx2, ky2)是接收方产生的临时公钥
[输入] (xv, yv)是接收方产生的中间结果
[输入] ida是发起方的用户标识
[输入] idb是接收方的用户标识

[输出] s2是接收方产生的32字节的HASH值，应等于sa。


返回值：无
	
*/

/*
	S2=Hash(0x03∥ yV ∥Hash(xV ∥ ZA ∥ ZB ∥ x1 ∥ y1 ∥ x2 ∥ y2))：
*/
    big x1, y1, x2, y2, x3, y3;
	unsigned char buf[256];
	unsigned char za[32];
	unsigned char zb[32];
    miracl *mip = mirsys(20, 0);

	mip->IOBASE = 16;
	x1 = mirvar(0); 
	y1 = mirvar(0); 
	x2 = mirvar(0); 
	y2 = mirvar(0); 
	x3 = mirvar(0); 
	y3 = mirvar(0); 

	sm3_z(ida, idalen, pax, paxlen, pay, paylen, za);
	sm3_z(idb, idblen, pbx, pbxlen, pby, pbylen, zb);

	bytes_to_big(kx1len, (char *)kx1, x1);
	bytes_to_big(ky1len, (char *)ky1, y1);
	bytes_to_big(kx2len, (char *)kx2, x2);
	bytes_to_big(ky2len, (char *)ky2, y2);
#if SM2_DEBUG
	bytes_to_big(xvlen, (char *)xv, x3);
	bytes_to_big(yvlen, (char *)yv, y3);
#else
	memcpy(buf, xv, xvlen);
	derand(buf, xvlen);
	bytes_to_big(xvlen, (char *)buf, x3);
	memcpy(buf, yv, yvlen);
	derand(buf, yvlen);
	bytes_to_big(yvlen, (char *)buf, y3);
#endif


	big_to_bytes(32, x3, (char *)buf, TRUE);
	memcpy(buf+32, za, 32);
	memcpy(buf+32+32, zb, 32);
	big_to_bytes(32, x1, (char *)buf+32+32+32, TRUE);
	big_to_bytes(32, y1, (char *)buf+32+32+32+32, TRUE);
	big_to_bytes(32, x2, (char *)buf+32+32+32+32+32, TRUE);
	big_to_bytes(32, y2, (char *)buf+32+32+32+32+32+32, TRUE);
	sm3(buf, 32*7, s2);

	buf[0] = 0x03;
	big_to_bytes(32, y3, (char *)buf+1, TRUE);
	memcpy(buf+1+32, s2, 32);
	sm3(buf, 65, s2);
#if SM2_DEBUG
	printf("s2: ");
	PrintBuf(s2, 32);
#endif


	mirkill(x1); 
	mirkill(y1); 
	mirkill(x2); 
	mirkill(y2); 
	mirkill(x3); 
	mirkill(y3); 
	mirexit();


}


#if 0
######################################
x86: CPU 2.8G
sm2_sign speed:147.058824/s
sm2_verify speed:116.414435/s
sm2_verify OK!===1
sm2_encrypt speed:74.404762/s
sm2_decrypt speed:150.602410/s
sm2_decrypt OK!


#endif

#if 0

void key_test()
{
	unsigned char ida[19] = "ALICE123@YAHOO.COM";
	unsigned char idb[18] = "BILL456@YAHOO.COM";
	unsigned char da[] = {0x6F,0xCB,0xA2,0xEF,0x9A,0xE0,0xAB,0x90,0x2B,0xC3,0xBD,0xE3,0xFF,0x91,0x5D,0x44,0xBA,0x4C,0xC7,0x8F,0x88,0xE2,0xF8,0xE7,0xF8,0x99,0x6D,0x3B,0x8C,0xCE,0xED,0xEE}; 
	unsigned char xa[] = {0x30,0x99,0x09,0x3B,0xF3,0xC1,0x37,0xD8,0xFC,0xBB,0xCD,0xF4,0xA2,0xAE,0x50,0xF3,0xB0,0xF2,0x16,0xC3,0x12,0x2D,0x79,0x42,0x5F,0xE0,0x3A,0x45,0xDB,0xFE,0x16,0x55};
	unsigned char ya[] = {0x3D,0xF7,0x9E,0x8D,0xAC,0x1C,0xF0,0xEC,0xBA,0xA2,0xF2,0xB4,0x9D,0x51,0xA4,0xB3,0x87,0xF2,0xEF,0xAF,0x48,0x23,0x39,0x08,0x6A,0x27,0xA8,0xE0,0x5B,0xAE,0xD9,0x8B};
	unsigned char db[] = {0x5E,0x35,0xD7,0xD3,0xF3,0xC5,0x4D,0xBA,0xC7,0x2E,0x61,0x81,0x9E,0x73,0x0B,0x01,0x9A,0x84,0x20,0x8C,0xA3,0xA3,0x5E,0x4C,0x2E,0x35,0x3D,0xFC,0xCB,0x2A,0x3B,0x53}; 
	unsigned char xb[] = {0x24,0x54,0x93,0xD4,0x46,0xC3,0x8D,0x8C,0xC0,0xF1,0x18,0x37,0x46,0x90,0xE7,0xDF,0x63,0x3A,0x8A,0x4B,0xFB,0x33,0x29,0xB5,0xEC,0xE6,0x04,0xB2,0xB4,0xF3,0x7F,0x43};
	unsigned char yb[] = {0x53,0xC0,0x86,0x9F,0x4B,0x9E,0x17,0x77,0x3D,0xE6,0x8F,0xEC,0x45,0xE1,0x49,0x04,0xE0,0xDE,0xA4,0x5B,0xF6,0xCE,0xCF,0x99,0x18,0xC8,0x5E,0xA0,0x47,0xC6,0x0A,0x4C};

	unsigned char kabuf[32], sa[32];
	unsigned char kbbuf[32], sb[32], s1[32], s2[32];
	unsigned char kx2[32], ky2[32];
	int kx2len, ky2len;
	unsigned char kx1[256], ky1[256], ra[256], xv[32], yv[32];
	int kx1len,ky1len, ralen, xvlen, yvlen;

#if SM2_DEBUG
#else
	sm2_keygen(xa, &kx1len, ya, &ky1len, da, &ralen);
	sm2_keygen(xb, &kx1len, yb, &ky1len, db, &ralen);
#endif

	sm2_keyagreement_a1_3(kx1, &kx1len, ky1, &ky1len, ra, &ralen);

	sm2_keyagreement_b1_9(
						  kx1, kx1len,
						  ky1, ky1len,
						  xa, 32,
						  ya, 32,
						  db, 32,
						  xb, 32,
						  yb, 32,
						  ida, 18,
						  idb, 17,
						  16,
						  kbbuf,
						  kx2, &kx2len,
						  ky2, &ky2len,
						  xv, &xvlen,
						  yv, &yvlen,
						  sb
						  );


	sm2_keyagreement_a4_10(
						  kx1, kx1len,
						  ky1, ky1len,
						  xa, 32,
						  ya, 32,
						  da, 32,
						  xb, 32,
						  yb, 32,
						  ida, 18,
						  idb, 17,
						  kx2, kx2len,
						  ky2, ky2len,
						  ra, ralen,
						  16,
						  kabuf,
						  s1,
						  sa
						  );

	sm2_keyagreement_b10(
						  xa, 32,
						  ya, 32,
						  xb, 32,
						  yb, 32,
						  kx1, 32,
						  ky1, 32,
						  kx2, 32,
						  ky2, 32,
						  xv, xvlen,
						  yv, yvlen,
						  ida, 18,
						  idb, 17,
						  s2
		);

	if(memcmp(s1, sb, 32) != 0)
	{
		printf("key_test error ! \n");
		return;
	}
	if(memcmp(kabuf, kbbuf, 16) != 0)
	{
		printf("key_test error ! \n");
		return;
	}

	if(memcmp(s2, sa, 32) != 0)
	{
		printf("key_test error ! \n");
		return;
	}

	printf("key_test OK ! \n");


}

int main()
{
	int rlen, slen, rv;
	unsigned char digest_sm2[] = {0xB5,0x24,0xF5,0x52,0xCD,0x82,0xB8,0xB0,0x28,0x47,0x6E,0x00,0x5C,0x37,0x7F,0xB1,0x9A,0x87,0xE6,0xFC,0x68,0x2D,0x48,0xBB,0x5D,0x42,0xE3,0xD9,0xB9,0xEF,0xFE,0x76};
	unsigned char kA_sm2[] = {0x12,0x8B,0x2F,0xA8,0xBD,0x43,0x3C,0x6C,0x06,0x8C,0x8D,0x80,0x3D,0xFF,0x79,0x79,0x2A,0x51,0x9A,0x55,0x17,0x1B,0x1B,0x65,0x0C,0x23,0x66,0x1D,0x15,0x89,0x72,0x63}; 
	unsigned char kAx_sm2[] = {0x0A,0xE4,0xC7,0x79,0x8A,0xA0,0xF1,0x19,0x47,0x1B,0xEE,0x11,0x82,0x5B,0xE4,0x62,0x02,0xBB,0x79,0xE2,0xA5,0x84,0x44,0x95,0xE9,0x7C,0x04,0xFF,0x4D,0xF2,0x54,0x8A};
	unsigned char kAy_sm2[] = {0x7C,0x02,0x40,0xF8,0x8F,0x1C,0xD4,0xE1,0x63,0x52,0xA7,0x3C,0x17,0xB7,0xF1,0x6F,0x07,0x35,0x3E,0x53,0xA1,0x76,0xD6,0x84,0xA9,0xFE,0x0C,0x6B,0xB7,0x98,0xE8,0x57};
	unsigned char r_sm2[] = {0x40,0xF1,0xEC,0x59,0xF7,0x93,0xD9,0xF4,0x9E,0x09,0xDC,0xEF,0x49,0x13,0x0D,0x41,0x94,0xF7,0x9F,0xB1,0xEE,0xD2,0xCA,0xA5,0x5B,0xAC,0xDB,0x49,0xC4,0xE7,0x55,0xD1};
	unsigned char s_sm2[] = {0x6F,0xC6,0xDA,0xC3,0x2C,0x5D,0x5C,0xF1,0x0C,0x77,0xDF,0xB2,0x0F,0x7C,0x2E,0xB6,0x67,0xA4,0x57,0x87,0x2F,0xB0,0x9E,0xC5,0x63,0x27,0xA6,0x7E,0xC7,0xDE,0xEB,0xE7};
	
	
	unsigned char dB[] = {0x16,0x49,0xAB,0x77,0xA0,0x06,0x37,0xBD,0x5E,0x2E,0xFE,0x28,0x3F,0xBF,0x35,0x35,0x34,0xAA,0x7F,0x7C,0xB8,0x94,0x63,0xF2,0x08,0xDD,0xBC,0x29,0x20,0xBB,0x0D,0xA0}; 
	unsigned char xB[] = {0x43,0x5B,0x39,0xCC,0xA8,0xF3,0xB5,0x08,0xC1,0x48,0x8A,0xFC,0x67,0xBE,0x49,0x1A,0x0F,0x7B,0xA0,0x7E,0x58,0x1A,0x0E,0x48,0x49,0xA5,0xCF,0x70,0x62,0x8A,0x7E,0x0A};
	unsigned char yB[] = {0x75,0xDD,0xBA,0x78,0xF1,0x5F,0xEE,0xCB,0x4C,0x78,0x95,0xE2,0xC1,0xCD,0xF5,0xFE,0x01,0xDE,0xBB,0x2C,0xDB,0xAD,0xF4,0x53,0x99,0xCC,0xF7,0x7B,0xBA,0x07,0x6A,0x42};
	unsigned char tx[] = "encryption standard";
	unsigned char etx[256] ;
	unsigned char mtx[256] ;
	
	clock_t start,end;
	double tt;
	int j;

	unsigned char userid[] = "ALICE123@YAHOO.COM";
	unsigned char msg[] = "message digest";



#if SM2_DEBUG
	int loop = 1;
	sm3_e(userid, 18, kAx_sm2, 32, kAy_sm2, 32, msg, 14, etx);
	printf("sm3_e: ");
	PrintBuf(etx, 32);

#else
	int loop = 200;
	int wxlen, wylen, privkeylen;

	sm2_keygen(kAx_sm2, &wxlen, kAy_sm2, &wylen, kA_sm2, &privkeylen);
	sm2_keygen(xB, &wxlen, yB, &wylen, dB, &privkeylen);
#endif


	key_test();
	return 0;


	start = clock();

	for(j=0;j<loop;j++)
		sm2_sign(digest_sm2,32,kA_sm2,32,r_sm2,&rlen,s_sm2,&slen);

	end = clock();

	tt = (double)(end-start)/CLOCKS_PER_SEC;
	printf("sm2_sign speed:%lf/s\n", (double)j/tt);

	start = clock();

	for(j=0;j<loop;j++)
		rv = sm2_verify(digest_sm2,32,r_sm2,32,s_sm2,32,kAx_sm2,32,kAy_sm2,32);

	end = clock();
	tt = (double)(end-start)/CLOCKS_PER_SEC;
	printf("sm2_verify speed:%lf/s\n", (double)j/tt);

	if(rv != 1) 
		printf("sm2_verify error!===%d\n",rv);
	else
		printf("sm2_verify OK!===%d\n",rv);
	
	
	start = clock();

	for(j=0;j<loop;j++)
		sm2_encrypt(tx,19,xB,32,yB,32,etx);

	end = clock();
	tt = (double)(end-start)/CLOCKS_PER_SEC;
	printf("sm2_encrypt speed:%lf/s\n", (double)j/tt);


#if SM2_DEBUG
/*
245C26 FB68B1DD DDB12C4B 6BF9F2B6 D5FE60A3 83B0D18D 1C4144AB F17F6252E7
76CB92 64C2A7E8 8E52B199 03FDC473 78F605E3 6811F5C0 7423A24B 84400F01B8
650053 A89B41C4 18B0C3AA D00D886C 00286467 9C3D7360 C30156FA B7C80A0276
712DA9 D8094A63 4B766D3A 285E0748 0653426D
*/	
	printf("Encrypted: ");
	PrintBuf(etx, 64+19+32);
#endif

	start = clock();
	for(j=0;j<loop;j++)
		sm2_decrypt(etx,64+19+32,dB,32,mtx);
	end = clock();
	tt = (double)(end-start)/CLOCKS_PER_SEC;
	printf("sm2_decrypt speed:%lf/s\n", (double)j/tt);

	if(sm2_decrypt(etx,64+19+32,dB,32,mtx) < 0)
		printf("sm2_decrypt error!\n");
	else
		printf("sm2_decrypt OK!\n");
	
#if SM2_DEBUG
	PrintBuf(mtx, 19);
#endif

	return 0;
}

#endif

