#include <iostream>
#include <string.h>
#include "aes.h"
#include "evp.h"
#include "sm4.h"
#include "sm2.h"
#include "CryptoWrapperAPI.h"

#define SM4_BLOCK_SIZE 16

typedef enum tagPASSWORD_LENGTH
{
	PASSWORD_LENGTH_128 = 128
}PASSWORD_LENGTH;

#define CHECK_ENCRYPTED_SIZE 4

const int ENCRYPTE_FLAG_AES = 0x01;
const int ENCRYPTE_FLAG_SM4 = 0x02;
const int CHECK_ENCRYPTE_KEY_VAULE = 0xF0;



int CheckEncryptedSM4(const unsigned char *input_data, const size_t &input_length)
{
	if (input_length <= CHECK_ENCRYPTED_SIZE)
	{
		//printf("CheckEncrypted SM4 error! ,input_length is less than 16\n");
		return -1;
	}

	int nRemainNum = (input_length - CHECK_ENCRYPTED_SIZE) % SM4_BLOCK_SIZE;
	if (nRemainNum != 0)
	{
		//printf("CheckEncrypted SM4 error! ,RemainNum is not 0\n");
		return -1;
	}

	unsigned char nLastBuffer[CHECK_ENCRYPTED_SIZE] = { 0 };
	memcpy(nLastBuffer, (void*)(input_data + (input_length - CHECK_ENCRYPTED_SIZE)), CHECK_ENCRYPTED_SIZE);

	if ((size_t)nLastBuffer[0] != ENCRYPTE_FLAG_SM4)
	{
		//printf("CheckEncrypted error! ,LastBuffer is not %u\n", input_length);
		return -1;
	}

	for (size_t i = 1; i < CHECK_ENCRYPTED_SIZE; i++)
	{
		if ((size_t)nLastBuffer[i] != CHECK_ENCRYPTE_KEY_VAULE)
		{
			//printf("CheckEncrypted error! ,LastBuffer is not %u\n", input_length);
			return -1;
		}
	}

	return 0;
}

int CheckEncryptedAES(const unsigned char *input_data, const size_t &input_length)
{
	if (input_length <= CHECK_ENCRYPTED_SIZE)
	{
		//printf("CheckEncrypted AES error! ,input_length is less than 16\n");
		return -1;
	}

	int nRemainNum = (input_length - CHECK_ENCRYPTED_SIZE) % AES_BLOCK_SIZE;
	if (nRemainNum != 0)
	{
		//printf("CheckEncrypted AES error! ,RemainNum is not 0\n");
		return -1;
	}

	unsigned char nLastBuffer[CHECK_ENCRYPTED_SIZE] = { 0 };
	memcpy(nLastBuffer, (void*)(input_data + (input_length - CHECK_ENCRYPTED_SIZE)), CHECK_ENCRYPTED_SIZE);
	if ((size_t)nLastBuffer[0] != ENCRYPTE_FLAG_AES)
	{
		//printf("CheckEncrypted error! ,LastBuffer is not %u\n", input_length);
		return -1;
	}
	for (size_t i = 1; i < CHECK_ENCRYPTED_SIZE; i++)
	{
		if ((size_t)nLastBuffer[i] != CHECK_ENCRYPTE_KEY_VAULE)
		{
			//printf("CheckEncrypted error! ,LastBuffer is not %u\n", input_length);
			return -1;
		}
	}

	return 0;
}

int EncryptAES(const unsigned char *password, const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length)
{
	if (password == NULL || input_data == NULL || out_data == NULL)
	{
		//printf("Encrypt error! ,param is null\n");
		return -1;
	}
	AES_KEY nAesKkey;

	if (AES_set_encrypt_key((const unsigned char *)password, PASSWORD_LENGTH_128, &nAesKkey) < 0)
	{
		//printf("AES_set_encrypt_key error!\n");
		return -1;
	}

	output_length = 0;
	int nBlockSize = input_length / AES_BLOCK_SIZE;
	int nRemainNum = input_length % AES_BLOCK_SIZE;

	size_t nCurPosition = 0;

	for (int i = 0; i < nBlockSize; i++)
	{
		AES_encrypt((input_data + nCurPosition), (out_data + nCurPosition), &nAesKkey);
		output_length += AES_BLOCK_SIZE;
		nCurPosition += AES_BLOCK_SIZE;
	}

	unsigned char nTemp[AES_BLOCK_SIZE + 1] = { 0 };
	memset(nTemp, 16, sizeof(nTemp));
	if (nRemainNum > 0)
	{
		memset(nTemp, (AES_BLOCK_SIZE - nRemainNum), AES_BLOCK_SIZE);
		memcpy(nTemp, (input_data + nCurPosition), nRemainNum);
	}

	AES_encrypt(nTemp, (out_data + nCurPosition), &nAesKkey);
	output_length += AES_BLOCK_SIZE;
	nCurPosition += AES_BLOCK_SIZE;

	//Push the check data
	memset(out_data + nCurPosition, CHECK_ENCRYPTE_KEY_VAULE, CHECK_ENCRYPTED_SIZE);
	memset(out_data + nCurPosition, ENCRYPTE_FLAG_AES, 1); //set AES flag
	output_length += CHECK_ENCRYPTED_SIZE;
	nCurPosition += CHECK_ENCRYPTED_SIZE;

	return 0;
}

int DecryptAES(const unsigned char *password, const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length)
{
	if (password == NULL || input_data == NULL || out_data == NULL)
	{
		//printf("Encrypt error! ,param is null\n");
		return -1;
	}

	if (0 != CheckEncryptedAES(input_data, input_length))
	{
		//printf("Encrypt error! ,it' not encryped data\n");
		return -1;
	}

	size_t true_data_len = input_length - CHECK_ENCRYPTED_SIZE;


	AES_KEY nAesKkey;

	if (AES_set_decrypt_key((const unsigned char *)password, 128, &nAesKkey) < 0)
	{
		//printf("Decrypt AES_set_decrypt_key error!\n");
		return -1;
	}

	output_length = 0;
	int nBlockSize = true_data_len / AES_BLOCK_SIZE;
	int nRemainNum = true_data_len % AES_BLOCK_SIZE;
	if (nRemainNum > 0)
	{
		//printf("Decrypt error! nRemainNum is %d\n", nRemainNum);
		return -1;
	}
	size_t nCurPosition = 0;

	for (int i = 0; i < nBlockSize; i++)
	{
		AES_decrypt((input_data + nCurPosition), (out_data + nCurPosition), &nAesKkey);
		output_length += AES_BLOCK_SIZE;
		nCurPosition += AES_BLOCK_SIZE;
	}

	unsigned char nDeleteNum = out_data[nCurPosition - 1];
	if ((int)nDeleteNum > 16)
	{
		//printf("Decrypt error! last 16byte > 16\n");
		return -1;
	}

	//LogINFO("Decrypt ok!, output_length is %u", output_length);
	output_length = output_length - nDeleteNum;
	return 0;
}

int EncryptSM4(const unsigned char *password, const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length)
{
	if (password == NULL || input_data == NULL || out_data == NULL)
	{
		//printf("Encrypt error! ,param is null\n");
		return -1;
	}


	int nBlockSize = input_length / SM4_BLOCK_SIZE;
	int nRemainNum = input_length % SM4_BLOCK_SIZE;
	size_t nCurPosition = 0;

	sm4_context ctx;
	sm4_setkey_enc(&ctx, password);
	output_length = 0;

	if (nBlockSize > 0)
	{
		sm4_crypt_ecb(&ctx, 1, (nBlockSize * SM4_BLOCK_SIZE), input_data, out_data);
		output_length += nBlockSize * SM4_BLOCK_SIZE;
		nCurPosition += nBlockSize * SM4_BLOCK_SIZE;
	}

	
	unsigned char nTemp[SM4_BLOCK_SIZE + 1] = { 0 };
	memset(nTemp, 16, sizeof(nTemp));
	if (nRemainNum > 0)
	{
		memset(nTemp, (SM4_BLOCK_SIZE - nRemainNum), SM4_BLOCK_SIZE);
		memcpy(nTemp, (input_data + nCurPosition), nRemainNum);
	}
	sm4_crypt_ecb(&ctx, 1, SM4_BLOCK_SIZE, nTemp, out_data + output_length);
	output_length += SM4_BLOCK_SIZE;
	nCurPosition += SM4_BLOCK_SIZE;


	//Push the check data
	memset(out_data + nCurPosition, CHECK_ENCRYPTE_KEY_VAULE, CHECK_ENCRYPTED_SIZE);
	memset(out_data + nCurPosition, ENCRYPTE_FLAG_SM4, 1); //set sm4 flag
	output_length += CHECK_ENCRYPTED_SIZE;
	nCurPosition += CHECK_ENCRYPTED_SIZE;

	return 0;
}



int DecryptSM4(const unsigned char *password, const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length)
{
	if (password == NULL || input_data == NULL || out_data == NULL)
	{
		//printf("Encrypt error! ,param is null\n");
		return -1;
	}

	if (0 != CheckEncryptedSM4(input_data, input_length))
	{
		//printf("Encrypt error! ,it' not encryped data\n");
		return -1;
	}

	size_t true_data_len = input_length - CHECK_ENCRYPTED_SIZE;
	
	int nBlockSize = true_data_len / SM4_BLOCK_SIZE;
	int nRemainNum = true_data_len % SM4_BLOCK_SIZE;
	if (nRemainNum > 0)
	{
		//printf("Decrypt error! nRemainNum is %d\n", nRemainNum);
		return -1;
	}

	size_t nCurPosition = 0;
	output_length = 0;

	sm4_context ctx;
	sm4_setkey_dec(&ctx, password);
	sm4_crypt_ecb(&ctx, 0, true_data_len, input_data, out_data);
	output_length = true_data_len;
	nCurPosition = true_data_len;

	unsigned char nDeleteNum = out_data[nCurPosition - 1];
	if ((int)nDeleteNum > 16)
	{
		//printf("Decrypt error! last 16byte > 16\n");
		return -1;
	}

	//LogINFO("Decrypt ok!, output_length is %u", output_length);
	output_length = output_length - nDeleteNum;
	return 0;
}

unsigned int CreateSecretKeySM4(unsigned char* buff_password, const unsigned int buffer_len)
{
	const unsigned int password_len = 16;
	const char nKey[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~!@#$^*()_+|,.-=";

	if ((buff_password == NULL) || (buffer_len < password_len))
	{
		//printf("CreateSecretKey error. buffer is null or len is less than 16\n");
		return 0;
	}
	memset(buff_password, 0, 16);

	srand((int)time(0));
	for (int i = 0; i < password_len; i++)
	{
		buff_password[i] = nKey[rand() % (strlen(nKey))];
	}

	return password_len;
}
unsigned int CreateSecretKey(unsigned char* buff_password, const unsigned int buffer_len)
{
	const unsigned int password_len = 16;
	const char nKey[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~!@#$^*()_+|,.-=";

	if ((buff_password == NULL) || (buffer_len < password_len))
	{
		//printf("CreateSecretKey error. buffer is null or len is less than 16\n");
		return 0;
	}
	memset(buff_password, 0, 16);

	srand((int)time(0));
	for (int i = 0; i < password_len; i++)
	{
		buff_password[i] = nKey[rand() % (strlen(nKey))];
	}

	return password_len;
}
int CreateSecretKeySM2(unsigned char* public_key, const unsigned int public_buffer_len,  unsigned int &pubKey_len, 
                                           unsigned char* prive_key, const unsigned int prive_buffer_len, unsigned int &privekey_len)
{
	if ((public_key == NULL) || (public_buffer_len < 64))
	{
		//printf("CreateSecretKey error. buffer is null or len is less than 16\n");
		return 0;
	}

	if ((prive_key == NULL) || (prive_buffer_len < 32))
	{
		//printf("CreateSecretKey error. buffer is null or len is less than 16\n");
		return 0;
	}

	unsigned char  pubKey_x[32],pubKey_y[32];
	unsigned char privkey[32];

	memset(pubKey_x, 0, 32);
	memset(pubKey_y, 0, 32);
	memset(privkey, 0, 32);

	int pubKey_x_len,pubKey_y_len,privkeylen;
	

	sm2_keygen(pubKey_x,&pubKey_x_len,pubKey_y,&pubKey_y_len,privkey,&privkeylen);

	for(int i=0;i < 32;i++)
	{
		public_key[i] = pubKey_x[i];
		//printf("%02x",pubKey_x[i]);
	}
	printf("\npubKey_x_len = %d \n",pubKey_x_len);
    printf("***************************************************\n");

	printf("pubKey_y = ");
	for(int i=0;i < 32;i++)
	{	
		public_key[i + 32] = pubKey_y[i];
		//printf("%02x",pubKey_y[i]);
	}
	printf("\npubKey_y_len = %d \n",pubKey_y_len);
	printf("***************************************************\n");

	pubKey_len = pubKey_x_len + pubKey_y_len;

	printf("privkey = ");
	for(int i=0;i<32;i++)
	{

		prive_key[i] = privkey[i];
		printf("%02x",privkey[i]);
	}
	privekey_len = privkeylen;
	printf("\nprivkeylen = %d \n",privkeylen);

	return 1;
}

int EncryptSM2(const unsigned char *public_key, const unsigned int pubKey_len,const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length )
{
	if (public_key == NULL || input_data == NULL || out_data == NULL)
	{
		//printf("Encrypt error! ,param is null\n");
		return -1;
	}
	if(pubKey_len < 64)
	{
		return -1;
	} 
	unsigned char pubKey_x[32], pubKey_y[32];

	memset(pubKey_x, 0, 32);
	memset(pubKey_y, 0, 32);


	for(int i=0;i < 32;i++)
	{
		pubKey_x[i] = public_key[i];
		//printf("%02x",pubKey_x[i]);
	}
	for(int i=0;i < 32;i++)
	{	
		pubKey_y[i] = public_key[i + 32];
		//printf("%02x",pubKey_y[i]);
	}

	sm2_encrypt((unsigned char *)input_data, input_length, pubKey_x, 32, pubKey_y, 32, out_data);

	output_length = input_length + 64 + 32;

	return 0;

}

int DecryptSM2(const unsigned char *prive_key, const unsigned int privekey_len,const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length)
{

	if (prive_key == NULL || input_data == NULL || out_data == NULL)
	{
		//printf("Encrypt error! ,param is null\n");
		return -1;
	}
	if(privekey_len < 32)
	{
		return -1;
	} 

	unsigned char privkey[32];
	memset(privkey, 0, 32);

	printf("privkey = ");
	for(int i=0;i<32;i++)
	{

		privkey[i] = prive_key[i];
		//printf("%02x",privkey[i]);
	}

	if (sm2_decrypt((unsigned char *)input_data, input_length, privkey, 32, out_data) < 0)
	{
		printf("sm2_decrypt error!\n");
	}
	else
	{
		printf("sm2_decrypt OK!\n");
	}

	output_length = input_length - 96;
	return 0;
}




