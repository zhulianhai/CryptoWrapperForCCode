#ifndef AES_WRAPPER_H
#define AES_WRAPPER_H

///************* AES 加密、解密、获取秘钥**************//
//默认属性：AES/ECB/PKCS5Padding mode
//秘钥长度：password must 16 bytes, 128 bits
//****************************************************//


#ifdef CRYPTO_EXPORTS
#define CRYPTO_API __declspec(dllexport)
#else
#ifdef CRYPTO_IMPORTS
#define CRYPTO_API __declspec(dllimport)
#else 
#define CRYPTO_API extern
#endif
#endif

/********************************************************************************
description:
	获取 AES 秘钥
input: 
	buffer_len：密钥缓存的长度，即 buff_password 长度
output:
	buff_password：密钥缓存，至少要分配16个字节
return:
	0：失败，非0:最终得到的密码长度
********************************************************************************/
CRYPTO_API unsigned int CreateSecretKeyAES(unsigned char* buff_password, const unsigned int buffer_len);

/********************************************************************************
description:
检查是否为加密数据
input:
input_data：需要被检查的数据
input_length：需要被检查数据的长度
output:
return:
0：加密数据，非0:非加密数据
********************************************************************************/
CRYPTO_API int CheckEncrypted(const unsigned char *input_data, const size_t &input_length);

/********************************************************************************
description:
	使用 AES 加密数据
input:
	password：密钥
	input_data：需要被加密的数据
	input_length：被加密数据的长度
output:
	out_data：输出的加密数据，需要在外部实现分配好，且长度至少为 input_length + 32
	output_length：输出的加密数据长度
return:
	0：成功，非0:失败
********************************************************************************/
CRYPTO_API int EncryptAES(const unsigned char *password, const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length);

/********************************************************************************
description:
	使用 AES 解密数据
input:
	password：密钥
	input_data：需要被解密的数据
	input_length：被解密数据的长度
output:
	out_data：输出的解密后的原始数据，需要在外部实现分配好，且长度至少为 input_length + 32
	output_length：输出的解密数据长度
return:
	0：成功，非0:失败
********************************************************************************/
CRYPTO_API int DecryptAES(const unsigned char *password, const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length);

#endif								