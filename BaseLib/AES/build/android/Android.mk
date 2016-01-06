
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../../
SRC_PATH:= $(TOP_PATH)/src/
COMM_PATH:= $(TOP_PATH)/../../common/

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
		  $(SRC_PATH)/a_object.c \
		  $(SRC_PATH)/aes_cbc.c \
		  $(SRC_PATH)/aes_cfb.c \
		  $(SRC_PATH)/aes_core.c \
		  $(SRC_PATH)/aes_ctr.c \
		  $(SRC_PATH)/aes_ecb.c \
		  $(SRC_PATH)/aes_ige.c \
		  $(SRC_PATH)/aes_misc.c \
		  $(SRC_PATH)/aes_ofb.c\
		  $(SRC_PATH)/aes_wrap.c \
		  $(SRC_PATH)/asn1_lib.c \
		  $(SRC_PATH)/b_print.c \
		  $(SRC_PATH)/bio_lib.c \
		  $(SRC_PATH)/bn_asm.c \
		  $(SRC_PATH)/bn_lib.c \
		  $(SRC_PATH)/bn_print.c \
		  $(SRC_PATH)/bn_shift.c \
		  $(SRC_PATH)/bn_word.c \
		  $(SRC_PATH)/bss_file.c \
		  $(SRC_PATH)/buf_str.c \
		  $(SRC_PATH)/cbc128.c \
		  $(SRC_PATH)/ccm128.c \
		  $(SRC_PATH)/cfb128.c \
		  $(SRC_PATH)/cryptlib.c \
		  $(SRC_PATH)/ctr128.c \
		  $(SRC_PATH)/cts128.c \
		  $(SRC_PATH)/err.c \
		  $(SRC_PATH)/ex_data.c \
		  $(SRC_PATH)/gcm128.c \
		  $(SRC_PATH)/lhash.c \
		  $(SRC_PATH)/mem.c \
		  $(SRC_PATH)/mem_clr.c \
		  $(SRC_PATH)/mem_dbg.c \
		  $(SRC_PATH)/o_init.c \
		  $(SRC_PATH)/obj_dat.c \
		  $(SRC_PATH)/obj_lib.c \
		  $(SRC_PATH)/ofb128.c \
		  $(SRC_PATH)/stack.c \
		  $(SRC_PATH)/xts128.c \
		  $(SRC_PATH)/CryptoWrapper.cpp \

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/ \
	$(TOP_PATH)/include/ \
	$(TOP_PATH)/interface/ \
	$(COMM_PATH)/ \

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

