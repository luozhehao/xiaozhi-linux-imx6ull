set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools "/usr/local/arm/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf")
set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-gnueabihf-g++)

# /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi NG 提示 eabi错误
# set(tools "/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi")
# set(CMAKE_C_COMPILER ${tools}/arm-poky-linux-gnueabi-gcc)
# set(CMAKE_CXX_COMPILER ${tools}/arm-poky-linux-gnueabi-g++)