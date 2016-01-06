/* ============================================================================
 * Copyright (c) 2010-2015.  All rights reserved.
 * SM3 Hash Cipher Algorithm: Digest length is 256-bit
 * ============================================================================
 */

#ifndef __SM3_HEADER__
#define __SM3_HEADER__

#ifdef __cplusplus
extern "C"{
#endif


#define  SM3_LBLOCK         16
#define  SM3_CBLOCK         64
#define  SM3_DIGEST_LENGTH  32
#define  SM3_LAST_BLOCK     56

typedef struct SM3state_st
{
	unsigned long h[8];
	unsigned long Nl,Nh;
	unsigned long data[SM3_LBLOCK];
	unsigned int  num;
} SM3_CTX;

void SM3_Init (SM3_CTX *ctx);
void SM3_Update(SM3_CTX *ctx, const void *data, unsigned int len);
void SM3_Final(unsigned char *md, SM3_CTX *ctx);
unsigned char *sm3(const unsigned char *d, unsigned int n, unsigned char *md);
/*
d:  data
n:  byte length
md: 32 bytes digest
*/

#ifdef __cplusplus
}
#endif

#endif
