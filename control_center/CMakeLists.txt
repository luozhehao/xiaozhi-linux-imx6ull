cmake_minimum_required(VERSION 3.5)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# set(tools "/usr/local/arm/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf")
set(tools "/usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf")
# /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabihf-g++)

project(CONTROL VERSION 1.0.1)

add_subdirectory(src)



