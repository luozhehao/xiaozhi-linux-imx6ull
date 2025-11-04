
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# 全局编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# 调试/发布配置
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# 预处理器定义
add_definitions(
    -DSOCKLEN_T=socklen_t
    -D_LARGEFILE_SOURCE=1
    -D_FILE_OFFSET_BITS=64
    -DHAVE_CONFIG_H
)

# 从环境变量或CMake变量获取DEFINES
if(DEFINED ENV{DEFINES})
    add_definitions($ENV{DEFINES})
endif()

##########################################
# 方法 1：使用 GLOB_RECURSE（简单直接）
# file(GLOB_RECURSE SOURCES 
#     "*.cc" 
#     "*.cpp" 
#     "*.cxx"
# )


# 方法 2：更精确的控制（推荐）
set(SOURCE_DIRS
    # audio
    # control
    # display
    # lib
    media
    ./
    # 添加更多目录...
)
# 收集所有源文件
set(SOURCES)
foreach(DIR ${SOURCE_DIRS})
    # file(GLOB_RECURSE DIR_SOURCES      ###递归搜索子目录
    file(GLOB DIR_SOURCES 
        "${DIR}/*.cc"
        "${DIR}/*.cpp"
        "${DIR}/*.cxx"
    )
    list(APPEND SOURCES ${DIR_SOURCES})
endforeach()
##########################################

include_directories(${PROJECT_SOURCE_DIR}/src)

# CPPFLAGS += -I"/home/lozoho/arm_libs/openssl_install/include"
# CPPFLAGS += -I"/home/lozoho/arm_libs/curl_install/include"
# CPPFLAGS += -I"/home/lozoho/arm_libs/websocketpp_inc"
# CPPFLAGS += -I"/home/lozoho/arm_libs/boost_install/include"

include_directories(/home/lozoho/arm_libs/openssl_install/include)
include_directories(/home/lozoho/arm_libs/curl_install/include)
include_directories(/home/lozoho/arm_libs/websocketpp_inc)
include_directories(/home/lozoho/arm_libs/boost_install/include)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/media)
include_directories(${PROJECT_SOURCE_DIR}/lib/cjson_install/include/cjson)
include_directories(${PROJECT_SOURCE_DIR}/lib/alsa-lib-share/include)

# include_directories(${PROJECT_SOURCE_DIR}/src/test_capture_queue)
# include_directories(${PROJECT_SOURCE_DIR}/src/test_play_queue)
# include_directories(${PROJECT_SOURCE_DIR}/src/test_play_queue_mono)
# include_directories(${PROJECT_SOURCE_DIR}/src/test_play_queue_mono_ask)    #OK
# include_directories(${PROJECT_SOURCE_DIR}/src/test_play_monothread)

# 静态链接库
# LIBS += /home/lozoho/arm_libs/boost_install/lib/libboost_system.a
# LIBS += -L"/home/lozoho/arm_libs/openssl_install/lib" -lssl -lcrypto
# LIBS += -L"/home/lozoho/arm_libs/curl_install/lib"  -lcurl

link_directories(${PROJECT_SOURCE_DIR}/lib/alsa-lib-share/lib)
# link_directories(/home/lozoho/arm_libs/boost_install/lib/libboost_system.a)
link_directories(/home/lozoho/arm_libs/openssl_install/lib)
link_directories(/home/lozoho/arm_libs/curl_install/lib)
# link_directories(/home/lozoho/arm_libs/boost_install/lib)
# link_directories(${PROJECT_SOURCE_DIR}/src/lib/cjson_install/lib)
set(BOOST_STATIC_LIB "/home/lozoho/arm_libs/boost_install/lib/libboost_system.a")
set(CJSON_STATIC_LIB "${PROJECT_SOURCE_DIR}/lib/cjson_install/lib/libcjson.a")
# target_link_libraries(your_target PRIVATE ${CJSON_STATIC_LIB})


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# include_directories(${PROJECT_SOURCE_DIR}/libhello)
add_executable(control_process ${SOURCES} )
target_link_libraries(control_process ssl crypto pthread asound curl ${CJSON_STATIC_LIB} ${BOOST_STATIC_LIB} -static-libstdc++ -static-libgcc)
# target_link_libraries(hello libhello)