CUR_DIR := $(shell pwd)
TOP_PATH:= $(CUR_DIR)/../../
SRC_PATH:= $(TOP_PATH)/src
COMM_PATH:= $(TOP_PATH)/common/
OBJ_DIR := $(CUR_DIR)/obj/CryptoWrapper/CryptoWrapper/build
OBJ_DIR1 := $(CUR_DIR)/obj/CryptoWrapper/src/aes
OBJ_DIR2 := $(CUR_DIR)/obj/CryptoWrapper/src/sm4
OBJ_DIR3 := $(CUR_DIR)/obj/CryptoWrapper/src/sm2

$(shell [ -d ${OBJ_DIR} ] || mkdir -p ${OBJ_DIR})
$(shell [ -d ${OBJ_DIR1} ] || mkdir -p ${OBJ_DIR1})
$(shell [ -d ${OBJ_DIR3} ] || mkdir -p ${OBJ_DIR3})



ver = debug
ifeq ($(ver), debug)
CFLAGS := -g -fPIC -Wall -DLINUX -DPOSIX -Wunused-variable -fpermissive -Wsign-compare  -D_REENTRANT
else
CFLAGS := -O2 -fPIC -Wall -DLINUX -DPOSIX -Wunused-variable -fpermissive -Wsign-compare -D_REENTRANT
endif


CC := g++

LOCAL_SRC_AES_FILES :=  \
		../../src/aes/aes_cbc.c \
 		../../src/aes/aes_cfb.c \
 		../../src/aes/aes_core.c \
 		../../src/aes/aes_ctr.c \
  	    ../../src/aes/aes_ecb.c \
  		../../src/aes/aes_ige.c \
  		../../src/aes/aes_misc.c \
  		../../src/aes/aes_ofb.c \
        ../../src/aes/aes_wrap.c \
  		../../src/aes/a_object.c \
        ../../src/aes/asn1_lib.c \
        ../../src/aes/bio_lib.c \
        ../../src/aes/bn_asm.c \
  		../../src/aes/bn_lib.c \
		../../src/aes/bn_print.c \
  		../../src/aes/bn_shift.c \
  		../../src/aes/bn_word.c \
  		../../src/aes/b_print.c \
  		../../src/aes/bss_file.c \
  		../../src/aes/buf_str.c \
  		../../src/aes/cbc128.c \
  		../../src/aes/ccm128.c \
  		../../src/aes/cfb128.c \
  		../../src/aes/cryptlib.c \
  		../../src/aes/ctr128.c  \
  		../../src/aes/cts128.c \
  		../../src/aes/err.c \
  		../../src/aes/ex_data.c \
  		../../src/aes/gcm128.c \
  		../../src/aes/lhash.c \
  		../../src/aes/mem.c \
  		../../src/aes/mem_clr.c \
  		../../src/aes/mem_dbg.c \
  		../../src/aes/obj_dat.c \
	  	../../src/aes/obj_lib.c \
	    ../../src/aes/ofb128.c \
  		../../src/aes/o_init.c \
  		../../src/aes/stack.c \
  		../../src/aes/xts128.c 


 LOCAL_SRC_SM4_FILES :=  \
		../../src/sm4/sm4.c  



LOCAL_SRC_SM2_FILES := \
		    ../../src/sm2/mraes.c \
 			../../src/sm2/mralloc.c \
 			../../src/sm2/mrarth0.c\
 			../../src/sm2/mrarth1.c\
 			../../src/sm2/mrarth2.c\
 			../../src/sm2/mrarth3.c\
 			../../src/sm2/mrbits.c\
 			../../src/sm2/mrbrick.c\
 			../../src/sm2/mrcore.c\
 			../../src/sm2/mrcrt.c\
 			../../src/sm2/mrcurve.c\
			../../src/sm2/mrebrick.c\
 			../../src/sm2/mrec2m.c\
 			../../src/sm2/mrecn2.c\
 			../../src/sm2/mrfast.c\
 			../../src/sm2/mrgcd.c\
			../../src/sm2/mrgcm.c\
			../../src/sm2/mrgf2m.c\
 			../../src/sm2/mrio1.c\
 			../../src/sm2/mrio2.c\
 			../../src/sm2/mrjack.c\
 			../../src/sm2/mrlucas.c\
 		    ../../src/sm2/mrmonty.c\
 			../../src/sm2/mrpower.c\
 			../../src/sm2/mrprime.c\
 			../../src/sm2/mrrand.c\
 			../../src/sm2/mrscrt.c\
 			../../src/sm2/mrshs256.c\
 			../../src/sm2/mrshs512.c\
 			../../src/sm2/mrshs.c\
 			../../src/sm2/mrsmall.c\
 			../../src/sm2/mrsroot.c\
 			../../src/sm2/mrstrong.c\
 			../../src/sm2/mrxgcd.c\
 			../../src/sm2/mrzzn2b.c\
 			../../src/sm2/mrzzn2.c\
 			../../src/sm2/mrzzn3.c\
 			../../src/sm2/sm2.c\
 			../../src/sm2/sm3.c

 LOCAL_SRC_FILES :=  \
		../../src/CryptoWrapper.cpp
  		

CSRCS := \
	$(LOCAL_SRC_FILES) \
	$(LOCAL_SRC_AES_FILES) \
	$(LOCAL_SRC_SM4_FILES) \
	$(LOCAL_SRC_SM2_FILES)


COBJS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(filter %.c,$(CSRCS)))
CCOBJS := $(patsubst %.cc, $(OBJ_DIR)/%.o, $(filter %.cc,$(CSRCS)))
CPPOBJS := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(filter %.cpp,$(CSRCS)))
	
	
	

INCDIR := \
		  -I./../../src \
		  -I./../../include/sm4/ \
		  -I./../../include/sm2/ \
		  -I./../../include/aes/ \
		  -I./../../include/AES/ \
		  -I./../../interface/ \


TARGET := libCryptoWrapper.so

$(TARGET): $(COBJS) $(CCOBJS) $(CPPOBJS)
	$(CC) -shared -fPIC -Wl,--whole-archive  $^ -Wl,--no-whole-archive -Wl,-soname -Wl,$@ -o $@
	mv $(TARGET)  ../../lib/

$(COBJS):$(OBJ_DIR)/%.o:%.c
	$(CC) -fPIC -c  $< -o $@ $(INCDIR) $(CFLAGS) 
$(CCOBJS):$(OBJ_DIR)/%.o:%.cc
	$(CC) -fPIC -c  $< -o $@ $(INCDIR) $(CFLAGS) 
$(CPPOBJS):$(OBJ_DIR)/%.o:%.cpp
	$(CC) -fPIC -c  $< -o $@ $(INCDIR) $(CFLAGS) 
clean:  
	rm -rf $(COBJS) $(CCOBJS) $(CPPOBJS) $(TARGET)  
