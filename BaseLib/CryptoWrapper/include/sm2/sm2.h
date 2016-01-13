#ifndef __SM2_HEADER_2015_12_28__
#define __SM2_HEADER_2015_12_28__

#include "sm3.h"
#include "miracl.h"

#ifdef __cplusplus
extern "C"{
#endif

int sm3_e(unsigned char *userid, int userid_len, unsigned char *xa, int xa_len, unsigned char *ya, int ya_len, unsigned char *msg, int msg_len, unsigned char *e);
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

void sm2_keygen(unsigned char *wx,int *wxlen, unsigned char *wy,int *wylen,unsigned char *privkey, int *privkeylen);
/*
功能：生成SM2公私钥对
[输出] wx：   公钥的X坐标，不足32字节在前面加0x00
[输出] wxlen: wx的字节数，32
[输出] wy：   公钥的Y坐标，不足32字节在前面加0x00
[输出] wylen: wy的字节数，32
[输出] privkey：私钥，不足32字节在前面加0x00
[输出] privkeylen： privkey的字节数，32
*/

void sm2_sign(unsigned char *hash,int hashlen,unsigned char *privkey,int privkeylen,unsigned char *cr,int *rlen,unsigned char *cs,int *slen);
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

int  sm2_verify(unsigned char *hash,int hashlen,unsigned char  *cr,int rlen,unsigned char *cs,int slen, unsigned char *wx,int wxlen, unsigned char *wy,int wylen);
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

int  sm2_encrypt(unsigned char *msg,int msglen, unsigned char *wx,int wxlen, unsigned char *wy,int wylen, unsigned char *outmsg);
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

int  sm2_decrypt(unsigned char *msg,int msglen, unsigned char *privkey, int privkeylen, unsigned char *outmsg);
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

void sm2_keyagreement_a1_3(unsigned char *kx1, int *kx1len, 
						   unsigned char *ky1, int *ky1len,
						   unsigned char *ra,  int *ralen
						   );

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
						  );

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
						  );

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
[输出] s1和sa是发起方产生的32字节的HASH值，s1自己保存（应等于sb），sa要传送给接收方，用于验证协商的正确性。如果s1和sa为NULL，则不输出。


返回值：0－失败  1－成功
	
*/

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
						 );
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
#ifdef __cplusplus
}
#endif


#endif
