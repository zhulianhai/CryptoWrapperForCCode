CUR_DIR := $(shell pwd)
TOP_PATH:= $(CUR_DIR)/../../../
SRC_PATH:= $(TOP_PATH)/localproxy/src
COMM_PATH:= $(TOP_PATH)/common/
OBJ_DIR := $(CUR_DIR)/obj/localproxy/localproxy/build/linux
OBJ_DIR1 := $(CUR_DIR)/obj/localproxy/localproxy/src
OBJ_DIR2 := $(CUR_DIR)/obj/localproxy/common

SHARE_DIR := $(CUR_DIR)/libs
$(shell [ -d ${SHARE_DIR} ] || mkdir -p ${SHARE_DIR})
$(shell [ -d ${OBJ_DIR} ] || mkdir -p ${OBJ_DIR})
$(shell [ -d ${OBJ_DIR1} ] || mkdir -p ${OBJ_DIR1})
$(shell [ -d ${OBJ_DIR2} ] || mkdir -p ${OBJ_DIR2})



ver = debug
ifeq ($(ver), debug)
CFLAGS := -g -fPIC -Wall -DLINUX -DPOSIX -Wunused-variable -fpermissive -Wsign-compare
else
CFLAGS := -O2 -fPIC -Wall -DLINUX -DPOSIX -Wunused-variable -fpermissive -Wsign-compare
endif


CC := g++  


LOCAL_SRC_FILES :=  \
		  ./../../src/LocalProxyImpl.cpp\
		  ./../../src/NetTestManager.cpp\
		  ./../../src/RemoteTransport.cpp\
		  ./../../src/RtpChannel.cpp \
		  ./../../src/RtpChannelManager.cpp \
		  ./../../src/SipRelay.cpp \
		  ./../../../common/BaseThread.cpp \
		  ./../../../common/NetClient.cpp \
		  ./../../../common/TcpNetClient.cpp \
		  ./../../../common/TraceLog.cpp \
		  ./../../../common/UdpNetClient.cpp \
		  ./../../../common/PackDef.cc \
		  ./../../../common/ParsePdu.cc \
		  ./../../../common/Utils.cpp \
		  ./../../../common/Common.c \

CSRCS := \
	$(LOCAL_SRC_FILES)


COBJS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(filter %.c,$(CSRCS)))
CCOBJS := $(patsubst %.cc, $(OBJ_DIR)/%.o, $(filter %.cc,$(CSRCS)))
CPPOBJS := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(filter %.cpp,$(CSRCS)))
	
	
	

INCDIR := \
		  -I./../../src \
		  -I./../../include\
		  -I./../../../common \
		  -I./../../../BaseLib/CryptoWrapper/interface/ \

TARGET := liblocalproxy.so  
$(TARGET): $(COBJS) $(CCOBJS) $(CPPOBJS)
	$(CC) -shared -L../../../BaseLib/CryptoWrapper/lib/ -lCryptoWrapper -fPIC -Wl,--whole-archive  $^ -Wl,--no-whole-archive -Wl,-soname -Wl,$@ -o $@

$(COBJS):$(OBJ_DIR)/%.o:%.c
	$(CC) -fPIC -c  $< -o $@ $(INCDIR) $(CFLAGS) 
$(CCOBJS):$(OBJ_DIR)/%.o:%.cc
	$(CC) -fPIC -c  $< -o $@ $(INCDIR) $(CFLAGS) 
$(CPPOBJS):$(OBJ_DIR)/%.o:%.cpp
	$(CC) -fPIC -c  $< -o $@ $(INCDIR) $(CFLAGS) 
clean:  
	rm -rf $(COBJS) $(CCOBJS) $(CPPOBJS) $(TARGET)  
