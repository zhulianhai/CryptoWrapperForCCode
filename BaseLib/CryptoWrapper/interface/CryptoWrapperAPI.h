#ifndef AES_SM4_WRAPPER_H
#define AES_SM4_WRAPPER_H

///************* AES 加密、解密、获取秘钥**************//
//默认属性：AES/ECB/PKCS5Padding mode
//秘钥长度：password must 16 bytes, 128 bits
//****************************************************//

///************* SM4 加密、解密、获取秘钥**************//
//默认属性：SM4 
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
	获取 SM2 公钥和秘钥
input: 
	public_key : 公钥的buffer， 要求至少64个字节
	public_buffer_len ： 公钥的buffer的长度
	prive_key ： 私钥的buffer， 要求至少32个字节
	prive_buffer_len : 私钥的buffer的长度
output:
	public_key ： 公钥
	prive_key ： 私钥
	pubKey_len ： 公钥的缓存的长度
	privekey_len ： 私钥的缓存长度
return:
	0：失败，非0:成功
********************************************************************************/
CRYPTO_API int CreateSecretKeySM2(unsigned char* public_key, const unsigned int public_buffer_len,  unsigned int &pubKey_len, 
                                           unsigned char* prive_key, const unsigned int prive_buffer_len, unsigned int &privekey_len);

/********************************************************************************
description:
使用国密 SM2 加密数据
input:
password：密钥
public_key: 公钥
pubKey_len： 公钥长度
input_data：需要被加密的数据
input_length：被加密数据的长度
output:
out_data：输出的加密数据，需要在外部实现分配好，且长度至少为 input_length + 96
output_length：输出的加密数据长度
return:
0：成功，非0:失败
********************************************************************************/

CRYPTO_API int EncryptSM2(const unsigned char *public_key,const unsigned int pubKey_len,
				              const unsigned char *input_data, const size_t &input_length, 
							  unsigned char *out_data, size_t &output_length);

/********************************************************************************
description:
	使用 SM2 解密数据
input:
	prive_key : 私钥
	privekey_len : 私钥长度

	input_data：需要被解密的数据
	input_length：被解密数据的长度
output:
	out_data：输出的解密后的原始数据，需要在外部实现分配好，且长度至少为 input_length + 96
	output_length：输出的解密数据长度
return:
	0：成功，非0:失败
********************************************************************************/
CRYPTO_API int DecryptSM2(const unsigned char *prive_key, const unsigned int privekey_len,
							const unsigned char *input_data, const size_t &input_length, 
							unsigned char *out_data, size_t &output_length);


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
CRYPTO_API unsigned int CreateSecretKey(unsigned char* buff_password, const unsigned int buffer_len);

/********************************************************************************
description:
	创建 SM4 秘钥 128位
input: 
	buffer_len：密钥缓存的长度，即 buff_password 长度
output:
	buff_password：密钥缓存，至少要分配16个字节
return:
	0：失败，非0:最终得到的密码长度
********************************************************************************/
CRYPTO_API unsigned int CreateSecretKeySM4(unsigned char* buff_password, const unsigned int buffer_len);

/********************************************************************************
description:
检查是否为AES加密数据
input:
input_data：需要被检查的数据
input_length：需要被检查数据的长度
output:
return:
0：加密数据，非0:非加密数据
********************************************************************************/
CRYPTO_API int CheckEncryptedAES(const unsigned char *input_data, const size_t &input_length);
/********************************************************************************
description:
检查是否为SM4加密数据
input:
input_data：需要被检查的数据
input_length：需要被检查数据的长度
output:
return:
0：加密数据，非0:非加密数据
********************************************************************************/
CRYPTO_API int CheckEncryptedSM4(const unsigned char *input_data, const size_t &input_length);

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

/********************************************************************************
description:
使用国密 SM4 加密数据
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
CRYPTO_API int EncryptSM4(const unsigned char *password, const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length);

/********************************************************************************
description:
使用国密 SM4 解密数据
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
CRYPTO_API int DecryptSM4(const unsigned char *password, const unsigned char *input_data, const size_t &input_length, unsigned char *out_data, size_t &output_length);

/********************************************************************************
description:
使用国密 SM4 加密文件
input:
password：密钥
input_file：需要被加密的文件
output_file：输出被加密的文件

return:
0：成功，非0:失败
********************************************************************************/
CRYPTO_API int EncryptFileSM4(const unsigned char *password, FILE * input_file, FILE * output_file);
/********************************************************************************
description:
使用国密 SM4 解密文件
input:
password：密钥
input_file：需要被解密的文件
output_file：解密输出的文件

return:
0：成功，非0:失败
********************************************************************************/
CRYPTO_API int DecryptFileSM4(const unsigned char *password, FILE * input_file, FILE * output_file);


#endif		 //AES_SM4_WRAPPER_H						
