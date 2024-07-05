# Install script for directory: /home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/llvm-objdump-16")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-build/libgen.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gen" TYPE FILE FILES
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/binary_file.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/colors.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/comaexception.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/datatypes.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/error.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/errorqueue.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/executecommand.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/files.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/files/lzma_stream.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/files/lzma_util.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/gen_export.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/helper.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/integers.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/logclass.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/logger.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/pch.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/settings.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/singleton.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/std_ext.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/stdfunc.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/timefunc.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/uint24.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/utils/convertable.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/utils/crc16.h"
    "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src/include/gen/utils/crc32.h"
    )
endif()

