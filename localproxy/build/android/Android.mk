
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../../
SRC_PATH:= $(TOP_PATH)/src/
COMM_PATH:= $(TOP_PATH)/../common/
AES_PATH:= $(TOP_PATH)/../BaseLib/AES/

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
		  $(SRC_PATH)/LocalProxyImpl.cpp\
		  $(SRC_PATH)/NetTestManager.cpp\
		  $(SRC_PATH)/RemoteTransport.cpp\
		  $(SRC_PATH)/RtpChannel.cpp \
		  $(SRC_PATH)/RtpChannelManager.cpp \
		  $(SRC_PATH)/SipRelay.cpp \
		  $(COMM_PATH)/BaseThread.cpp \
		  $(COMM_PATH)/NetClient.cpp \
		  $(COMM_PATH)/TcpNetClient.cpp \
		  $(COMM_PATH)/TraceLog.cpp \
		  $(COMM_PATH)/UdpNetClient.cpp \
		  $(COMM_PATH)/PackDef.cc \
		  $(COMM_PATH)/ParsePdu.cc \
		  $(COMM_PATH)/Utils.cpp \
		  $(COMM_PATH)/Common.c \

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/ \
	$(TOP_PATH)/include/ \
	$(COMM_PATH)/ \
	$(AES_PATH)/include \
	$(AES_PATH)/interface \

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog  -L$(AES_PATH)/build/android/libs/armeabi-v7a/ -lCryptoWrapper

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

LOCAL_CFLAGS := -fexpensive-optimizations -fpermissive -pthread -DHAVE_NEON=1 \
		-mfpu=neon -mfloat-abi=softfp -flax-vector-conversions -fPIC -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch 

LOCAL_MODULE:= liblocalproxy

#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)

