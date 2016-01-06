CUR_DIR := $(shell pwd)
OBJ_DIR := $(CUR_DIR)/obj/
OBJ_SRC := $(CUR_DIR)/obj/src

STATIC_DIR := $(CUR_DIR)/../linux/obj/
$(shell [ -d ${OBJ_SRC} ] || mkdir -p ${OBJ_SRC})



# 编译选项
ver = debug
ifeq ($(ver), debug)
CFLAGS := -g -fPIC -Wall -DLINUX   -DPOSIX  -Wunused-variable  
	
else
CFLAGS := -O2 -fPIC -Wall -DLINUX   -DPOSIX \
	-Wunused-variable  
endif


#编译方式 gcc or g++ 改动
CC := g++ 



#编译要依赖的动态库静态库
SHARED_LIBRARIES := \
		-L$(CUR_DIR)/lib \
		-Wl,-rpath=./lib \
		-lCryptoWrapper 

#STATIC_LIBRARIES: = 



#源代码文件
LOCAL_CPP_SRC_FILES :=  \
        src/Test.cpp

#LOCAL_C_SRC_FILES :=  \



#链接选项	 
#LDFLAGS:= -shared -fPIC  
LDFLAGS:= -lrt


#源代码文件
CSRCS :=  \
	   $(LOCAL_CPP_SRC_FILES) 
	   
#$(LOCAL_C_SRC_FILES) 


#源代码文件对应的.o文件
#COBJS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(CSRCS)) 
CPPOBJS := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(CSRCS)) 
#CXXOBJS := $(patsubst %.cc, $(OBJ_DIR)/%.o, $(CSRCS)) 

	
	

#将动态库和静态库结合起来
LDLIB = \
	$(SHARED_LIBRARIES) 

#	$(STATIC_LIBRARIES) \


#头文件位置
INCDIR := \
	  -I./src \
	  -I./include 


#动态库静态库的名字 libxxx.so or libxxx.a
#TARGET := libxxx.so 

#可执行文件
EXEC := test

#生成可执行程序，依赖于OBJS

#可执行文件
$(EXEC):$(CPPOBJS)                     
	$(CC) $(CFLAGS) $(INCDIR) $< -o  $(EXEC)  $(LDLIB) $(LDFLAGS)
	@echo "Create Application ($(EXEC)) successful..."

#动态库
$(TARGET): $(COBJS) $(CPPOBJS) $(CXXOBJS)
	$(CC) -shared -fPIC -Wl,--whole-archive  $(LDLIB) -Wl,--no-whole-archive  -Wl,-soname -Wl,$@ -o $@

#静态库
$(TARGET): $(COBJS) $(CPPOBJS) $(CXXOBJS)
	ar r $(TARGET) $^


$(COBJS):$(OBJ_DIR)/%.o:%.c
	$(CC) -c  $< -o $@ $(INCDIR) $(CFLAGS) 
$(CPPOBJS):$(OBJ_DIR)/%.o:%.cpp
	$(CC) -c  $< -o $@ $(INCDIR) $(CFLAGS) 
$(CXXOBJS):$(OBJ_DIR)/%.o:%.cc
	$(CC) -c  $< -o $@ $(INCDIR) $(CFLAGS) 

clean:  
	rm -rf *.o  *.a  $(CPPOBJS) $(TARGET) $(EXEC) 
