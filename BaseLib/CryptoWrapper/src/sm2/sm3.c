/* ============================================================================
 * Copyright (c) 2010-2015.  All rights reserved.
 * SM3 Hash Cipher Algorithm: Digest length is 256-bit
 * ============================================================================
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "sm3.h"

#define nl2c(l,c)	(*((c)++) = (unsigned char)(((l) >> 24) & 0xff), \
					 *((c)++) = (unsigned char)(((l) >> 16) & 0xff), \
					 *((c)++) = (unsigned char)(((l) >> 8)  & 0xff), \
					 *((c)++) = (unsigned char)(((l)    )   & 0xff))

#define c_2_nl(c)	((*(c) << 24) | (*(c+1) << 16) | (*(c+2) << 8) | *(c+3))
#define ROTATE(X, C) (((X) << (C)) | ((X) >> (32 - (C))))

#define TH 0x79cc4519
#define TL 0x7a879d8a
#define FFH(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define FFL(X, Y, Z) (((X) & (Y)) | ((X) & (Z)) | ((Y) & (Z)))
#define GGH(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define GGL(X, Y, Z) (((X) & (Y)) | ((~X) & (Z)))
#define P0(X)  ((X) ^ (((X) << 9) | ((X) >> 23)) ^ (((X) << 17) | ((X) >> 15)))
#define P1(X)  ((X) ^ (((X) << 15) | ((X) >> 17)) ^ (((X) << 23) | ((X) >> 9)))

#define DEBUG_SM3 0

#if DEBUG_SM3
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
#endif

void sm3_block(SM3_CTX *ctx)
{
	register int j, k;
	register unsigned long t;
	register unsigned long ss1, ss2, tt1, tt2;
	register unsigned long a, b, c, d, e, f, g, h;
	unsigned long w[132];


	for(j = 0; j < 16; j++)
		w[j] = ctx->data[j];

	for(j = 16; j < 68; j++)
	{
		t = w[j-16] ^ w[j-9] ^ ROTATE(w[j-3], 15);
		w[j] = P1(t) ^ ROTATE(w[j-13], 7) ^ w[j-6];
	}


	for(j = 0, k = 68; j < 64; j++, k++)
	{
		w[k] = w[j] ^ w[j+4];
	}


	a = ctx->h[0];
	b = ctx->h[1];
	c = ctx->h[2];
	d = ctx->h[3];
	e = ctx->h[4];
	f = ctx->h[5];
	g = ctx->h[6];
	h = ctx->h[7];

	for(j = 0; j < 16; j++)
	{
		ss1 = ROTATE(ROTATE(a, 12) +  e + ROTATE(TH, j), 7);
		ss2 = ss1 ^ ROTATE(a, 12);
		tt1 = FFH(a, b, c) + d + ss2 + w[68 + j];
		tt2 = GGH(e, f, g) + h + ss1 + w[j];

		d = c; 
		c = ROTATE(b, 9);
		b = a;
		a = tt1;

		h = g;
		g = ROTATE(f, 19);
		f = e;
		e = P0(tt2);
	}


	for(j = 16; j < 33; j++)
	{
		ss1 = ROTATE(ROTATE(a, 12) +  e + ROTATE(TL, j), 7);
		ss2 = ss1 ^ ROTATE(a, 12);
		tt1 = FFL(a, b, c) + d + ss2 + w[68 + j];
		tt2 = GGL(e, f, g) + h + ss1 + w[j];

		d = c;
		c = ROTATE(b, 9);
		b = a;
		a = tt1;

		h = g;
		g = ROTATE(f, 19);
		f = e;
		e = P0(tt2);
	}

	for(j = 33; j < 64; j++)
	{
		ss1 = ROTATE(ROTATE(a, 12) +  e + ROTATE(TL, (j-32)), 7);
		ss2 = ss1 ^ ROTATE(a, 12);
		tt1 = FFL(a, b, c) + d + ss2 + w[68 + j];
		tt2 = GGL(e, f, g) + h + ss1 + w[j];

		d = c;
		c = ROTATE(b, 9);
		b = a;
		a = tt1;

		h = g;
		g = ROTATE(f, 19);
		f = e;
		e = P0(tt2);
	}

	ctx->h[0]  ^=  a ;
	ctx->h[1]  ^=  b ;
	ctx->h[2]  ^=  c ;
	ctx->h[3]  ^=  d ;
	ctx->h[4]  ^=  e ;
	ctx->h[5]  ^=  f ;
	ctx->h[6]  ^=  g ;
	ctx->h[7]  ^=  h ;

}


void SM3_Init (SM3_CTX *ctx)
{
	ctx->h[0] = 0x7380166fUL;
	ctx->h[1] = 0x4914b2b9UL;
	ctx->h[2] = 0x172442d7UL;
	ctx->h[3] = 0xda8a0600UL;
	ctx->h[4] = 0xa96f30bcUL;
	ctx->h[5] = 0x163138aaUL;
	ctx->h[6] = 0xe38dee4dUL;
	ctx->h[7] = 0xb0fb0e4eUL;
	ctx->Nl   = 0;
	ctx->Nh   = 0;
	ctx->num  = 0;
}

void SM3_Update(SM3_CTX *ctx, const void *data, unsigned int len)
{
	unsigned char *d;
	unsigned long l;
	int i, sw, sc;


	if (len == 0)
		return;

	l = (ctx->Nl + (len << 3)) & 0xffffffffL;
	if (l < ctx->Nl) /* overflow */
		ctx->Nh++;
	ctx->Nh += (len >> 29);
	ctx->Nl = l;


	d = (unsigned char *)data;

	while (len >= SM3_CBLOCK)
	{
		ctx->data[0] = c_2_nl(d);
		d += 4;
		ctx->data[1] = c_2_nl(d);
		d += 4;
		ctx->data[2] = c_2_nl(d);
		d += 4;
		ctx->data[3] = c_2_nl(d);
		d += 4;
		ctx->data[4] = c_2_nl(d);
		d += 4;
		ctx->data[5] = c_2_nl(d);
		d += 4;
		ctx->data[6] = c_2_nl(d);
		d += 4;
		ctx->data[7] = c_2_nl(d);
		d += 4;
		ctx->data[8] = c_2_nl(d);
		d += 4;
		ctx->data[9] = c_2_nl(d);
		d += 4;
		ctx->data[10] = c_2_nl(d);
		d += 4;
		ctx->data[11] = c_2_nl(d);
		d += 4;
		ctx->data[12] = c_2_nl(d);
		d += 4;
		ctx->data[13] = c_2_nl(d);
		d += 4;
		ctx->data[14] = c_2_nl(d);
		d += 4;
		ctx->data[15] = c_2_nl(d);
		d += 4;

		sm3_block(ctx);
		len -= SM3_CBLOCK;
	}

	if(len > 0)
	{
		memset(ctx->data, 0, 64);
		ctx->num = len + 1;
		sw = len >> 2;
		sc = len & 0x3;

		for(i = 0; i < sw; i++)
		{
			ctx->data[i] = c_2_nl(d);
			d += 4;
		}

		switch(sc)
		{
			case 0:
				ctx->data[i] = 0x80000000;
				break;
			case 1:
				ctx->data[i] = (d[0] << 24) | 0x800000;
				break;
			case 2:
				ctx->data[i] = (d[0] << 24) | (d[1] << 16) | 0x8000;
				break;
			case 3:
				ctx->data[i] = (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | 0x80;
				break;
		}

	}


}

void SM3_Final(unsigned char *md, SM3_CTX *ctx)
{

	if(ctx->num == 0)
	{
		memset(ctx->data, 0, 64);
		ctx->data[0] = 0x80000000;
		ctx->data[14] = ctx->Nh;
		ctx->data[15] = ctx->Nl;
	}
	else
	{
		if(ctx->num <= SM3_LAST_BLOCK)
		{
			ctx->data[14] = ctx->Nh;
			ctx->data[15] = ctx->Nl;
		}
		else
		{
			sm3_block(ctx);
			memset(ctx->data, 0, 56);
			ctx->data[14] = ctx->Nh;
			ctx->data[15] = ctx->Nl;
		}
	}

	sm3_block(ctx);

	nl2c(ctx->h[0], md);
	nl2c(ctx->h[1], md);
	nl2c(ctx->h[2], md);
	nl2c(ctx->h[3], md);
	nl2c(ctx->h[4], md);
	nl2c(ctx->h[5], md);
	nl2c(ctx->h[6], md);
	nl2c(ctx->h[7], md);
}

unsigned char *sm3(const unsigned char *d, unsigned int n, unsigned char *md)
{
	SM3_CTX ctx;

	SM3_Init(&ctx);
	SM3_Update(&ctx, d, n);
	SM3_Final(md, &ctx);
	memset(&ctx, 0, sizeof(ctx));

	return(md);
}



#if 0

int main()
{
	unsigned char data[] = "abc";
	/*66c7f0f4 62eeedd9 d1f2d46b dc10e4e2 4167c487 5cf2f7a2 297da02b 8f4ba8e0*/
	unsigned char data1[] = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
	/*debe9ff9 2275b8a1 38604889 c18e5a4d 6fdb70e5 387e5765 293dcba3 9c0c5732*/
	unsigned char md[SM3_DIGEST_LENGTH];

	clock_t start,end;
	double tt;
	int j;

	memset(md, 0, sizeof(md));
	sm3(data, 3, md);
#if DEBUG_SM3
	PrintBuf(md, 32);
#endif

	memset(md, 0, sizeof(md));
	sm3(data1, 64, md);
#if DEBUG_SM3
	PrintBuf(md, 32);
#endif

	start = clock();

	for(j=0;j<1000000;j++)
	{
		sm3(data1, 55, md);
	}


	end = clock();

	tt = (double)(end-start)/CLOCKS_PER_SEC;
	printf("speed:%lfMbps\n", (double)512/tt);

	return 0;
}
#endif
