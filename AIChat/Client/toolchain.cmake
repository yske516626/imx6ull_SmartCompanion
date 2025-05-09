# Specify the cross-compilation toolchain
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# set(SDK_PATH "/home/kingham/Projects/git_projects/Echo-Mate/SDK/rv1106-sdk")
set(SDK_PATH "/home/book/100ask_imx6ull-sdk")

# Specify the compiler paths
#set(CMAKE_C_COMPILER ${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc)
#set(CMAKE_CXX_COMPILER ${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-g++)
set(CMAKE_C_COMPILER ${SDK_PATH}/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${SDK_PATH}/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin/arm-buildroot-linux-gnueabihf-g++)

# Specify the sysroot (if available)
#set(CMAKE_SYSROOT ${SDK_PATH}/sysdrv/source/buildroot/buildroot-2023.02.6/output/host/arm-buildroot-linux-uclibcgnueabihf/sysroot)
set(CMAKE_SYSROOT ${SDK_PATH}/Buildroot_2020.02.x/output/host/arm-buildroot-linux-gnueabihf/sysroot)


# Add paths to find libraries and includes
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)