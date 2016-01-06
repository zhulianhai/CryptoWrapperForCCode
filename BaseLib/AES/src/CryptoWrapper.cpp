#include <iostream>
#include <string.h>
#include "aes.h"
#include "evp.h"
#include "CryptoWrapperAPI.h"

typedef enum tagPASSWORD_LENGTH
{
	PASSWORD_LENGTH_128 = 128
}PASSWORD_LENGTH;

#define CHECK_ENCRYPTED_SIZE 4
const int CHECK_ENCRYPTE_KEY_VAULE = 0xF0;

int CheckEncrypted(const unsigned char *input_data, const size_t &input_length)
{
	if (input_length <= CHECK_ENCRYPTED_SIZE)
	{
		//printf("CheckEncrypted error! ,input_length is less than 16\n");
		return -1;
	}

	int nRemainNum = (input_length - CHECK_ENCRYPTED_SIZE) % AES_BLOCK_SIZE;
	if (nRemainNum != 0)
	{
		//printf("CheckEncrypted error! ,RemainNum is not 0\n");
		return -1;
	}

	unsigned char nLastBuffer[CHECK_ENCRYPTED_SIZE] = { 0 };
	memcpy(nLastBuffer, (void*)(input_data + (input_length - CHECK_ENCRYPTED_SIZE)), CHECK_ENCRYPTED_SIZE);
	for (size_t i = 0; i < CHECK_ENCRYPTED_SIZE; i++)
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

	if (0 != CheckEncrypted(input_data, input_length))
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

unsigned int CreateSecretKeyAES(unsigned char* buff_password, const unsigned int buffer_len)
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

