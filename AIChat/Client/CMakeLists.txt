# Minimum CMake version requirement
cmake_minimum_required(VERSION 3.10)

# Option to choose target architecture
option(TARGET_ARM "Build for ARM architecture" OFF)

# If building for ARM, set the toolchain file before anything else
if(TARGET_ARM)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake" CACHE FILEPATH "Default toolchain file for ARM builds")
        message(STATUS "No toolchain file specified; using default: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
    message(STATUS "Building for ARM using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
else()
    message(STATUS "Building for x86 native architecture.")
endif()

# Project name and language
project(AIChatClient)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure pkg-config is available
find_package(PkgConfig REQUIRED)

if(TARGET_ARM)
    # 设置交叉编译环境下 pkg-config 使用的路径
    set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/lib/pkgconfig")
endif()

# Find required packages
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
pkg_check_modules(OPUS REQUIRED opus)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
#find_package(websocketpp REQUIRED)
add_subdirectory(websocketpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/snowboy)

# Include directories
include_directories(
    ${OPUS_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    #${WEBSOCKETPP_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
	include_directories(websocketpp)
)

# Add the source files
add_library(AIChatCore STATIC
    src/Audio.cc
    src/WebsocketHandler.cc
    src/Achieve.cc
    src/StateMachine.cc
)

# Link libraries for opus and websocketpp
target_link_libraries(AIChatCore
    ${OPUS_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${WEBSOCKETPP_LIBRARIES}
    ${Boost_LIBRARIES}
    boost_system  # 添加 boost_system
    snowboy-detect-c-wrapper
)

# Add the executable and link it to the core library
add_executable(AIChatClient 
    src/main.cc
)
target_link_libraries(AIChatClient AIChatCore)

# Add the Interface (cmake details in Interface/CMakeLists.txt)
add_subdirectory(Interface)

# Custom command to clean up generated files
if(NOT TARGET clean-all)
    add_custom_target(clean-all
        COMMAND find "${CMAKE_BINARY_DIR}" -mindepth 1 -maxdepth 1 -exec rm -rf {} +
        COMMENT "Cleaning all generated files."
    )
endif()

#执行make install，生成可执行文件夹，复制到开发板上运行
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/AIChatClient_demo") 
# 安装可执行文件到/bin子目录
install(TARGETS AIChatClient DESTINATION bin)

# 安装资源文件到demo/resources子目录
install(FILES ${CMAKE_SOURCE_DIR}/third_party/snowboy/resources/common.res
        DESTINATION third_party/snowboy/resources)
install(FILES ${CMAKE_SOURCE_DIR}/third_party/snowboy/resources/models/echo.pmdl
        DESTINATION third_party/snowboy/resources/models)
install(FILES ${CMAKE_SOURCE_DIR}/third_party/audio/waked.pcm
        DESTINATION third_party/audio)



