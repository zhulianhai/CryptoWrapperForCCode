
#include <string.h>
#include <stdio.h>
#include "CryptoWrapperAPI.h"

#ifdef WIN32
#include <windows.h>
#elif defined LINUX
#include <unistd.h>
#endif

int main(int argc, char* argv[])
{
	printf("hello, world\n");

	unsigned char encrypt_input_data[1024] = "11111111122222222222222222333333333333333333333334444444444444444445555555555555555555666666666666666677777777777777777";
	size_t encrypt_input_length = strlen((char*)encrypt_input_data);
	unsigned char encrypt_output_data[1024 + 32] = { 0 };
	size_t encrypt_output_length;

	unsigned char decrypt_output_data[1024 + 32] = { 0 };
	size_t decrypt_output_length;
	for (int i = 0; i < 10000; i++)
	{
		unsigned char buff_password[20] = { 0 };
		unsigned int nLen = CreateSecretKeyAES(buff_password, sizeof(buff_password));
		printf("%s\n", buff_password);

		
		if (0 != EncryptAES(buff_password, encrypt_input_data, encrypt_input_length, encrypt_output_data, encrypt_output_length))
		{
			printf("EncryptAES error, buff_password:%s\n", buff_password);
			return 0;
		}

		if (0 != CheckEncrypted(encrypt_output_data, encrypt_output_length))
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
		Sleep(100);
#elif defined LINUX
		usleep(100 * 1000);
#endif
	}

	return 0;
}

