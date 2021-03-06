
#include <string.h>
#include <stdio.h>
#include "CryptoWrapperAPI.h"

#ifdef WIN32
#include <windows.h>
#elif defined LINUX
#include <unistd.h>
#endif

int TestAes();
int TestSm4();
int TestSm4ForStand();

int main(int argc, char* argv[])
{
	printf("hello, world\n");

	TestSm4ForStand();

	{
		long nStartTime = GetTickCount();
		//TestAes();
		long nEndTime = GetTickCount();
		printf("TestAes time:%ld\n", nEndTime - nStartTime);
	}
	{
		long nStartTime = GetTickCount();
		TestSm4();
		long nEndTime = GetTickCount();
		printf("TestSm4 time:%ld\n", nEndTime - nStartTime);
	}

	return 0;
}

int TestSm4ForStand()
{
	unsigned char key[16] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
	unsigned char input[16 + 32] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };
	unsigned char output[16 + 32];
	unsigned char output_dec[16 + 32];
	unsigned long i;

	//encrypt standard testing vector
	size_t encrypt_output_length = 0;
	for (i = 0; i < 16; i++)
	{
		printf("%02x ", input[i]);
	}
	printf("\n");
	memset(output, 0, 48);
	if (0 != EncryptSM4(key, input, 16, output, encrypt_output_length))
	{
		printf("EncryptSM4 error, buff_password:%s\n", key);
		return 0;
	}
	for (i = 0; i<encrypt_output_length; i++)
		printf("%02x ", output[i]);
	printf("\n");

	//decrypt testing
	size_t decrypt_output_length = 0;
	if (0 != DecryptSM4(key, output, encrypt_output_length, output_dec, decrypt_output_length))
	{
		printf("DecryptSM4 error, buff_password:%s\n", key);
		return 0;
	}
	for (i = 0; i<16; i++)
		printf("%02x ", output_dec[i]);
	printf("\n");

	//decrypt 1M times testing vector based on standards.
	i = 0;
#if 0
	while (i<1000000)
	{
		EncryptSM4(key, input, 16, input, encrypt_output_length);
		//sm4_crypt_ecb(&ctx, 1, 16, input, input);
		i++;
	}
	for (i = 0; i<16; i++)
		printf("%02x ", input[i]);
	printf("\n");
#endif
	return 0;
}

int TestSm4()
{
	unsigned char encrypt_input_data[1024] = "1111111112222222222222222233333333333333333333333444444444444444444555555555555555555566666666666666667777777777777777711111111122222222222222222333333333333333333333334444444444444444445555555555555555555666666666666666677777777777777777111111111222222222222222223333333333333333333333344444444444444444455555555555555555556666666666666666777777777777777771111111112222222222222222233333333333333333333333444444444444444444555555555555555555566666666666666667777777777777777711111111122222222222222222333333333333333333333334444444444444444445555555555555555555666666666666666677777777777777777111111111222222222222222223333333333333333333333344444444444444444455555555555555555556666666666666666777777777777777771111111112222222222222222233333333333333333333333444444444444444444555555555555555555566666666666666667777777777777777711111111122222222222222222333333333333333333333334444444444444444445555555555555555555666666666666666677777777777777777111111111222222222222222223333333333333333333333";
	size_t encrypt_input_length = strlen((char*)encrypt_input_data);
	unsigned char encrypt_output_data[1024 + 32] = { 0 };
	size_t encrypt_output_length = 0;

	unsigned char decrypt_output_data[1024 + 32] = { 0 };
	size_t decrypt_output_length;
	for (int i = 0; i < 1000; i++)
	{
		/*unsigned char buff_password[20] = { 0 };
		unsigned int nLen = CreateSecretKey(buff_password, sizeof(buff_password));*/
		//printf("%s\n", buff_password);
		unsigned char buff_password[20] = "01234567890abcef";


		if (0 != EncryptSM4(buff_password, encrypt_input_data, encrypt_input_length, encrypt_output_data, encrypt_output_length))
		{
			printf("EncryptSM4 error, buff_password:%s\n", buff_password);
			return 0;
		}

		if (0 != DecryptSM4(buff_password, encrypt_output_data, encrypt_output_length, decrypt_output_data, decrypt_output_length))
		{
			printf("DecryptSM4 error, buff_password:%s\n", buff_password);
			return 0;
		}

		char* pChar = strstr((char*)decrypt_output_data, (char*)encrypt_input_data);

		if (pChar == NULL || decrypt_output_length != encrypt_input_length)
		{
			printf("EncryptSM4 or DecryptSM4 error, buff_password:%s, encrypt_input_data:%s, decrypt_output_data:%s\n", buff_password, encrypt_input_data, decrypt_output_data);
			return 0;
		}
#ifdef WIN32
		//Sleep(100);
#elif defined LINUX
		usleep(100 * 1000);
#endif
	}
	return 0;
}

int TestAes()
{
	unsigned char encrypt_input_data[1024] = "1111111112222222222222222233333333333333333333333444444444444444444555555555555555555566666666666666667777777777777777711111111122222222222222222333333333333333333333334444444444444444445555555555555555555666666666666666677777777777777777111111111222222222222222223333333333333333333333344444444444444444455555555555555555556666666666666666777777777777777771111111112222222222222222233333333333333333333333444444444444444444555555555555555555566666666666666667777777777777777711111111122222222222222222333333333333333333333334444444444444444445555555555555555555666666666666666677777777777777777111111111222222222222222223333333333333333333333344444444444444444455555555555555555556666666666666666777777777777777771111111112222222222222222233333333333333333333333444444444444444444555555555555555555566666666666666667777777777777777711111111122222222222222222333333333333333333333334444444444444444445555555555555555555666666666666666677777777777777777111111111222222222222222223333333333333333333333";
	size_t encrypt_input_length = strlen((char*)encrypt_input_data);
	unsigned char encrypt_output_data[1024 + 32] = { 0 };
	size_t encrypt_output_length;

	unsigned char decrypt_output_data[1024 + 32] = { 0 };
	size_t decrypt_output_length;
	for (int i = 0; i < 1000; i++)
	{
		unsigned char buff_password[20] = "01234567890abcef";
		unsigned int nLen = 16;
		//printf("%s\n", buff_password);


		if (0 != EncryptAES(buff_password, encrypt_input_data, encrypt_input_length, encrypt_output_data, encrypt_output_length))
		{
			printf("EncryptAES error, buff_password:%s\n", buff_password);
			return 0;
		}

		if (0 != CheckEncryptedAES(encrypt_output_data, encrypt_output_length))
		{
			printf("CheckEncrypted error\n");
			return 0;
		}

		if (0 != DecryptAES(buff_password, encrypt_output_data, encrypt_output_length, decrypt_output_data, decrypt_output_length))
		{
			printf("DecryptAES error, buff_password:%s\n", buff_password);
			return 0;
		}

		char* pChar = strstr((char*)decrypt_output_data, (char*)encrypt_input_data);

		if (pChar == NULL || decrypt_output_length != encrypt_input_length)
		{
			printf("EncryptAES or DecryptAES error, buff_password:%s, encrypt_input_data:%s, decrypt_output_data:%s\n", buff_password, encrypt_input_data, decrypt_output_data);
			return 0;
		}
#ifdef WIN32
		//Sleep(1);
#elif defined LINUX
		usleep(100 * 1000);
#endif
	}

	return 0;
}
