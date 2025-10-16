######################################

#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
  
#target you can change test to what you want
#目标文件名，输入任意你想要的执行文件名
TARGET  := sound_app

PROJECT_ROOT_DIR := $(shell pwd)

###############

###############

#compile and lib parameter
#编译参数 特别
CROSS_COMPILE := /usr/local/arm/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# CROSS_COMPILE := arm-poky-linux-gnueabi-
CC      := $(CROSS_COMPILE)gcc
# CC      := "arm-poky-linux-gnueabi-gcc  -march=armv7ve -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7 \
# 	     --sysroot=/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi"
# CXX 	:= g++
CXX 	:= $(CROSS_COMPILE)g++
LIBS    := 
LDFLAGS := -lasound -pthread -lopus -lspeexdsp -g
# LDFLAGS := -m64 -lpostproc -lm -lpthread -ldl -fpermissive  -llzma -lz -lx264 -lva -lva-drm  -lva-x11  -lvdpau -lX11  -lXv -lX11 -lXext
# LDFLAGS += -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lasound -lSDL2 -lsndio
DEFINES :=
# INCLUDE := -I.
CFLAGS  := -g -Wall -O2 -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 $(DEFINES) -std=c++11 -std=gnu++11 #$(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H  #-static

#############################
# 头文件路径  $(PROJECT_ROOT_DIR)/
CPPFLAGS += -I"/home/lozoho/ai_space/atom_sound/alsa-lib-share/include"
CPPFLAGS += -I"/home/lozoho/ai_space/atom_sound/opus_arm_install/include"
CPPFLAGS += -I"/home/lozoho/ai_space/atom_sound/speexdsp_arm_install/include"
# CPPFLAGS += -I"./include/live555/usageEnvironment"
# CPPFLAGS += -I"./include/live555/groupsock"
# CPPFLAGS += -I"./include/live555/liveMedia"
# CPPFLAGS += -I"./include/live555/basicUsageEnvironment"
# CPPFLAGS += -I"./include/x264"
# CPPFLAGS += -I"./include/encoder"


######################
INCLUDES 	 =  -I./
# -I/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include\
#-I/usr/include

CPPFLAGS += $(INCLUDES)

# LIVE555_LIBS =  ./lib/livelib/libliveMedia.a ./lib/livelib/libgroupsock.a \
# 				./lib/livelib/libBasicUsageEnvironment.a ./lib/livelib/libUsageEnvironment.a
# X264_LIBS 	 =  ./lib/x264lib/libx264.a ./lib/x264lib/libx264.so.148
# LIBS         +=  $(LIVE555_LIBS) $(X264_LIBS)

# LIBS += -L"$(PROJECT_ROOT_DIR)/lib/libffmpeg/lib" -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil 

######################
# 静态链接库
# LIBS += -L"../liblive555/lib" -lliveMedia -lBasicUsageEnvironment -lUsageEnvironment -lgroupsock 
# LIBS += -L"$(PROJECT_ROOT_DIR)/libffmpeg/lib" -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil 
# LIBS += ../liblive555/lib/libliveMedia.a
# LIBS += ../liblive555/lib/libBasicUsageEnvironment.a
# LIBS += ../liblive555/lib/libUsageEnvironment.a
# LIBS += ../liblive555/lib/libgroupsock.a
# LIBS += ../liblive555/lib/libmylive.a
##############################

LIBS += -L"/home/lozoho/ai_space/atom_sound/alsa-lib-share/lib" -lasound
LIBS += -L"/home/lozoho/ai_space/atom_sound/opus_arm_install/lib" -lopus
LIBS += -L"/home/lozoho/ai_space/atom_sound/speexdsp_arm_install/lib" -lspeexdsp
  
#i think you should do anything here
#下面的基本上不需要做任何改动了
.PHONY : everything objs clean veryclean rebuild
  
everything : $(TARGET)
  
all : $(TARGET)
  
objs : $(OBJS)
  
rebuild: veryclean everything
                
clean :
	rm -fr *.so
	rm -fr *.o
    
veryclean : clean
	rm -fr $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX)  $(CXXFLAGS)  $(CPPFLAGS) -o $@ $(OBJS)  $(LIBS) $(LDFLAGS)
	echo "Done!"
	echo "$(TARGET) has been built!"