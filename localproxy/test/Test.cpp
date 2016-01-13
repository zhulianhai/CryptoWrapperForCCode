#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "Utils.h"
#include "AESWrapper.h"
#include "ParsePdu.h"
#include "PackDef.h"
using namespace std;

int main(int argc, char* argv[])
{
	do
	{
		//char* data, uint16_t data_len, uint16_t cmd_type, char* out_buf
		char nOriginalData[1024] = "11111113333333333333444444abc";
		char nTargetData[1024] = { 0 };
		uint16_t nTargetLen = 0;
		char *pParsedData = new char[1024];
		uint16_t nParseLength = 0;
		ParsePdu pdu;
		pdu.packPduBuf(nOriginalData, strlen(nOriginalData), PACK_CMD_ID_RELAYSVR_AUTH_REQ, nTargetData, nTargetLen);

		ParsePdu pdu1;
		uint16_t nCmdType;
		pdu1.parsePduHead(nTargetData, nTargetLen, nCmdType, pParsedData, nParseLength);
	} while (false);


	do
	{
		ParsePdu pdu1;
		PACK_RELAYSVR_HEARTBEAT_PACKET nHeartReq;
		nHeartReq.heartbeat_tag_ = RESPONSE::HEARTBEAT_REQ;
		nHeartReq.reserved_ = Utils::Time();
		PDUSerial<PACK_RELAYSVR_HEARTBEAT_PACKET> pdu_serial(nHeartReq);
		char nBuffer[PACK_MAX_LEN] = { 0 };
		uint16_t nLength = PACK_MAX_LEN;
		pdu1.packPduBuf(pdu_serial, nBuffer, nLength);

		ParsePdu pdu2;
		uint16_t nCmdType;
		char *pParsedData = new char[1024];
		uint16_t nParseLength = 0;
		pdu2.parsePduHead(nBuffer, nLength, nCmdType, pParsedData, nParseLength);
		char *p = pParsedData;
	} while (false);

    return 0;
}

int TestAesApi()
{
	do
	{
		std::string strPassword;
		cout << "please input password:" << endl;
		while (true)
		{
			strPassword.clear();
			getline(cin, strPassword);
			if (strPassword.size() != 16)
			{
				printf("length is 16. please reset password!\r\n");
				cout << "please input password:" << endl;
				continue;
			}
			else
				break;
		}
		AESWrapper::PASSWORD_LENGTH nPasswordLength;
		if (strPassword.size() == 16)
			nPasswordLength = AESWrapper::PASSWORD_LENGTH_128;

		char userkey[1024] = { 0 };
		memcpy(userkey, strPassword.data(), strPassword.size());


		FILE *pReadFP = NULL;
		FILE *pWriteEncryptedFP = NULL;
		FILE *pWriteDecryptedFP = NULL;
		pReadFP = fopen("original.txt", "r");
		if (NULL == pReadFP)
		{
			printf("can not find original.txt\r\n");
			return -1;
		}

		pWriteEncryptedFP = fopen("encrypted.txt", "wb");
		if (pWriteEncryptedFP == NULL)
		{
			printf("can not find encrypted.txt\r\n");
			return -1;
		}

		pWriteDecryptedFP = fopen("decrypted.txt", "wb");
		if (pWriteDecryptedFP == NULL)
		{
			printf("can not find decrypted.txt\r\n");
			return -1;
		}
		unsigned char nOriginalData[10240] = { 0 };
		int nReadLength = fread(nOriginalData, 1, 1024, pReadFP);

		AESWrapper* pAESWrapper = AESWrapper::GetInstance();
		if (!pAESWrapper->SetEncryptKey((const unsigned char*)userkey, nPasswordLength))
			return -1;

		unsigned char nEncryptedData[10240] = { 0 };
		size_t nEncryptedLength = 0;
		if (!pAESWrapper->Encrypt((const unsigned char*)nOriginalData, nReadLength, nEncryptedData, nEncryptedLength))
			return -1;

		fwrite(nEncryptedData, nEncryptedLength, 1, pWriteEncryptedFP);

		unsigned char nDecryptData[10240] = { 0 };
		size_t nDecryptLength = 0;
		if (!pAESWrapper->Decrypt(nEncryptedData, nEncryptedLength, nDecryptData, nDecryptLength))
			return -1;

		fwrite(nDecryptData, nDecryptLength, 1, pWriteDecryptedFP);

		fclose(pWriteDecryptedFP);
		fclose(pWriteEncryptedFP);
		fclose(pReadFP);
	} while (false);


	do
	{
		std::string strPassword;
		cout << "please input password:" << endl;
		while (true)
		{
			strPassword.clear();
			getline(cin, strPassword);
			if (strPassword.size() != 16)
			{
				printf("length is 16. please reset password!\r\n");
				cout << "please input password:" << endl;
				continue;
			}
			else
				break;
		}
		AESWrapper::PASSWORD_LENGTH nPasswordLength;
		if (strPassword.size() == 16)
			nPasswordLength = AESWrapper::PASSWORD_LENGTH_128;
		char userkey[1024] = { 0 };
		memcpy(userkey, strPassword.data(), strPassword.size());

		FILE *pReadEncryptedFP = NULL;
		FILE *pWriteDecryptedFP = NULL;

		pReadEncryptedFP = fopen("encrypted.txt", "r");
		if (pReadEncryptedFP == NULL)
		{
			printf("can not find encrypted.txt\r\n");
			return -1;
		}

		pWriteDecryptedFP = fopen("decrypted-1.txt", "wb");
		if (pWriteDecryptedFP == NULL)
		{
			printf("can not find decrypted.txt\r\n");
			return -1;
		}
		unsigned char nOriginalData[10240] = { 0 };
		int nReadLength = fread(nOriginalData, 1, 1024, pReadEncryptedFP);

		AESWrapper* pAESWrapper = AESWrapper::GetInstance();
		pAESWrapper->SetEncryptKey((const unsigned char*)userkey, nPasswordLength);
		unsigned char nDecryptData[10240] = { 0 };
		size_t nDecryptLength = 0;
		if (!pAESWrapper->Decrypt(nOriginalData, nReadLength, nDecryptData, nDecryptLength))
			return -1;

		fwrite(nDecryptData, nDecryptLength, 1, pWriteDecryptedFP);

		fclose(pWriteDecryptedFP);
		fclose(pReadEncryptedFP);
	} while (false);
	return 0;
}
