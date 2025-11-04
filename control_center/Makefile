######################################

#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
# SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
# OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))


#target you can change test to what you want
#目标文件名，输入任意你想要的执行文件名
# 定义输出目录变量
# 目录设置
# SRC_DIR = src
SRC_DIR = ./
BUILD_DIR = build
TARGET  := $(BUILD_DIR)/control_center
PROJECT_ROOT_DIR := $(shell pwd)
# ###############
# # 自动查找源文件
C_SRCS = $(wildcard $(SRC_DIR)/*.c)
CPP_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(C_SRCS:$(SRC_DIR)/%.c= $(BUILD_DIR)/%.o) \
       $(CPP_SRCS:$(SRC_DIR)/%.cpp= $(BUILD_DIR)/%.o)

SRCS =  C_SRCS
SRCS += CPP_SRCS

###############

#compile and lib parameter
#编译参数 特别
CROSS_COMPILE := /usr/local/arm/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
#   /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
# CROSS_COMPILE := /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# CROSS_COMPILE := arm-poky-linux-gnueabi-
CC      := $(CROSS_COMPILE)gcc
# CC      := "arm-poky-linux-gnueabi-gcc  -march=armv7ve -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7 \
# 	     --sysroot=/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi"
# CXX 	:= g++
CXX 	:= $(CROSS_COMPILE)g++
LIBS    := 
LDFLAGS := -lssl -lcrypto -lpthread -lcurl -g
# LDFLAGS := -m64 -lpostproc -lm -lpthread -ldl -fpermissive  -llzma -lz -lx264 -lva -lva-drm  -lva-x11  -lvdpau -lX11  -lXv -lX11 -lXext
# LDFLAGS += -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lasound -lSDL2 -lsndio
DEFINES :=
# INCLUDE := -I.
CFLAGS  := -g -Wall -O2 -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 $(DEFINES)  -std=c++11 -std=gnu++11 #$(INCLUDE)
CFLAGS  += -static-libstdc++ -static-libgcc   #  添加静态库
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H  #-static

#############################
# 头文件路径  $(PROJECT_ROOT_DIR)/
CPPFLAGS += -I"/home/lozoho/ai_space/atom_sound/alsa-lib-share/include"
CPPFLAGS += -I"/home/lozoho/ai_space/atom_sound/opus_arm_install/include"
CPPFLAGS += -I"/home/lozoho/ai_space/atom_sound/speexdsp_arm_install/include"


CPPFLAGS += -I"/home/lozoho/arm_libs/openssl_install/include"
CPPFLAGS += -I"/home/lozoho/arm_libs/curl_install/include"
CPPFLAGS += -I"/home/lozoho/arm_libs/websocketpp_inc"
CPPFLAGS += -I"/home/lozoho/arm_libs/boost_install/include"
#CPPFLAGS += -I"/home/lozoho/ai_space/atom_sound/websocketpp"

######################
INCLUDES 	 =  -I./
# -I/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include\
#-I/usr/include

CPPFLAGS += $(INCLUDES)

######################
# 静态链接库
LIBS += /home/lozoho/arm_libs/boost_install/lib/libboost_system.a
LIBS += -L"/home/lozoho/arm_libs/openssl_install/lib" -lssl -lcrypto
LIBS += -L"/home/lozoho/arm_libs/curl_install/lib"  -lcurl

##############################
  
#i think you should do anything here
#下面的基本上不需要做任何改动了
.PHONY : everything objs clean veryclean rebuild
  
everything : $(TARGET)
  
all : $(TARGET)

# C 源文件编译规则
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "编译: $< -> $@"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MP -MT $@ -MF $*.d $<

# C++ 源文件编译规则（如果需要）
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "编译: $< -> $@"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM -MP -MT $@ -MF $*.d $<



  
objs : $(OBJS)
  
rebuild: veryclean everything
                
clean :
	rm -fr *.so
	rm -fr $(BUILD_DIR)/*.so
	rm -fr *.o
	rm -fr $(BUILD_DIR)/*.o
	rm -fr *.d
	rm -fr $(BUILD_DIR)/*.d


    
veryclean : clean
	rm -fr $(TARGET)

$(TARGET) : $(OBJS)
	@mkdir -p $(@D)
	$(CXX)  $(CXXFLAGS)  $(CPPFLAGS) -o $@ $(OBJS)  $(LIBS) $(LDFLAGS)
	echo "Done!"
	echo "$(TARGET) has been built!"


