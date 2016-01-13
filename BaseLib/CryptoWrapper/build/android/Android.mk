
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../../
SRC_PATH:= $(TOP_PATH)/

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
		$(SRC_PATH)/src/aes/aes_cbc.c \
 		$(SRC_PATH)/src/aes/aes_cfb.c \
 		$(SRC_PATH)/src/aes/aes_core.c \
 		$(SRC_PATH)/src/aes/aes_ctr.c \
  	        $(SRC_PATH)/src/aes/aes_ecb.c \
  		$(SRC_PATH)/src/aes/aes_ige.c \
  		$(SRC_PATH)/src/aes/aes_misc.c \
  		$(SRC_PATH)/src/aes/aes_ofb.c \
                $(SRC_PATH)/src/aes/aes_wrap.c \
  		$(SRC_PATH)/src/aes/a_object.c \
                $(SRC_PATH)/src/aes/asn1_lib.c \
                $(SRC_PATH)/src/aes/bio_lib.c \
                $(SRC_PATH)/src/aes/bn_asm.c \
  		$(SRC_PATH)/src/aes/bn_lib.c \
		$(SRC_PATH)/src/aes/bn_print.c \
  		$(SRC_PATH)/src/aes/bn_shift.c \
  		$(SRC_PATH)/src/aes/bn_word.c \
  		$(SRC_PATH)/src/aes/b_print.c \
  		$(SRC_PATH)/src/aes/bss_file.c \
  		$(SRC_PATH)/src/aes/buf_str.c \
  		$(SRC_PATH)/src/aes/cbc128.c \
  		$(SRC_PATH)/src/aes/ccm128.c \
  		$(SRC_PATH)/src/aes/cfb128.c \
  		$(SRC_PATH)/src/aes/cryptlib.c \
  		$(SRC_PATH)/src/aes/ctr128.c  \
  		$(SRC_PATH)/src/aes/cts128.c \
  		$(SRC_PATH)/src/aes/err.c \
  		$(SRC_PATH)/src/aes/ex_data.c \
  		$(SRC_PATH)/src/aes/gcm128.c \
  		$(SRC_PATH)/src/aes/lhash.c \
  		$(SRC_PATH)/src/aes/mem.c \
  		$(SRC_PATH)/src/aes/mem_clr.c \
  		$(SRC_PATH)/src/aes/mem_dbg.c \
  		$(SRC_PATH)/src/aes/obj_dat.c \
	  	$(SRC_PATH)/src/aes/obj_lib.c \
	        $(SRC_PATH)/src/aes/ofb128.c \
  		$(SRC_PATH)/src/aes/o_init.c \
  		$(SRC_PATH)/src/aes/stack.c \
  		$(SRC_PATH)/src/aes/xts128.c \
		$(SRC_PATH)/src/sm4/sm4.c   \
		$(SRC_PATH)/src/CryptoWrapper.cpp

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/ \
	$(TOP_PATH)/include/ \
	$(TOP_PATH)/include/AES \
	$(TOP_PATH)/include/sm4 \
	$(TOP_PATH)/interface/ \

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog 

LOCAL_CPPFLAGS := -DHAMMER_TIME=1 \
		  -DHASHNAMESPACE=__gnu_cxx \
		  -DHASH_NAMESPACE=__gnu_cxx \
		  -DDISABLE_DYNAMIC_CAST \
		  -D_REENTRANT \
		  -DANDROID \
		  -DEXPAT_RELATIVE_PATH \
		  -DFEATURE_ENABLE_SSL \
		  -DHAVE_OPENSSL_SSL_H=1 \
		  -DFEATURE_ENABLE_PSTN \
		  -DHAVE_WEBRTC_VOICE=1 \
		  -DLOGGING=1 \
		  -DRAYCOM_ANDROID \

LOCAL_CFLAGS := -fexpensive-optimizations -pthread -DHAVE_NEON=1 \
		-mfpu=neon -mfloat-abi=softfp -flax-vector-conversions -fPIC -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch 

LOCAL_MODULE:= libCryptoWrapper

#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)

