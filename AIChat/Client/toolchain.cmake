# Specify the cross-compilation toolchain
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

#set(SDK_PATH "/home/book/100ask_imx6ull-sdk")

# Specify the compiler paths
#set(CMAKE_C_COMPILER ${SDK_PATH}/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc)
#set(CMAKE_CXX_COMPILER ${SDK_PATH}/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-g++)

set(CMAKE_C_COMPILER /home/book/100ask_imx6ull-sdk/Buildroot_2020.02.x/output/host/bin/arm-buildroot-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER /home/book/100ask_imx6ull-sdk/Buildroot_2020.02.x/output/host/bin/arm-buildroot-linux-gnueabihf-g++)

# Specify the sysroot (if available)
#set(CMAKE_SYSROOT /home/book/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/arm-buildroot-linux-gnueabihf/sysroot)
set(CMAKE_SYSROOT "/home/book/100ask_imx6ull-sdk/Buildroot_2020.02.x/output/host/arm-buildroot-linux-gnueabihf/sysroot")

# Add paths to find libraries and includes
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
