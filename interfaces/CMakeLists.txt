cmake_minimum_required(VERSION 3.14)

project(interfaces LANGUAGES CXX)

set(INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/interfaces")
set(SRC_PATH     "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(files_h
    "${INCLUDE_PATH}/connectioncontext.h"
    "${INCLUDE_PATH}/connectionmanager.h"
    "${INCLUDE_PATH}/conn/async_connection.h"
    "${INCLUDE_PATH}/conn/sync_connection.h"
    "${INCLUDE_PATH}/exec/default_query_executor.h"
    "${INCLUDE_PATH}/exec/query_executor_fabric.h"
    "${INCLUDE_PATH}/ifaces/baseinterface.h"
    "${INCLUDE_PATH}/ifaces/usbhidport.h"
    "${INCLUDE_PATH}/parsers/base_request_parser.h"
    "${INCLUDE_PATH}/parsers/base_response_parser.h"
    "${INCLUDE_PATH}/parsers/protocom_request_parser.h"
    "${INCLUDE_PATH}/parsers/protocom_response_parser.h"
    "${INCLUDE_PATH}/types/base_settings.h"
    "${INCLUDE_PATH}/types/common_types.h"
    "${INCLUDE_PATH}/types/protocol_settings.h"
    "${INCLUDE_PATH}/types/protocom_types.h"
    "${INCLUDE_PATH}/types/usbhidportinfo.h"
    "${INCLUDE_PATH}/types/usbhidsettings.h"
    "${INCLUDE_PATH}/utils/request_queue.h"
    "${INCLUDE_PATH}/utils/slot_trait.h"
    "${INCLUDE_PATH}/utils/unique_queue.h"
)

set(files_cpp
    "${SRC_PATH}/connectioncontext.cpp"
    "${SRC_PATH}/connectionmanager.cpp"
    "${SRC_PATH}/conn/async_connection.cpp"
    "${SRC_PATH}/conn/sync_connection.cpp"
    "${SRC_PATH}/exec/default_query_executor.cpp"
    "${SRC_PATH}/exec/query_executor_fabric.cpp"
    "${SRC_PATH}/ifaces/baseinterface.cpp"
    "${SRC_PATH}/ifaces/usbhidport.cpp"
    "${SRC_PATH}/parsers/base_request_parser.cpp"
    "${SRC_PATH}/parsers/base_response_parser.cpp"
    "${SRC_PATH}/parsers/protocom_request_parser.cpp"
    "${SRC_PATH}/parsers/protocom_response_parser.cpp"
    "${SRC_PATH}/types/protocol_settings.cpp"
    "${SRC_PATH}/types/usbhidportinfo.cpp"
    "${SRC_PATH}/types/usbhidsettings.cpp"
    "${SRC_PATH}/utils/request_queue.cpp"
)

add_library(${PROJECT_NAME} STATIC ${files_cpp} ${files_h})

# Including directories for library
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Linking dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC gen)

if(UNIX)
  target_link_libraries(${PROJECT_NAME} PRIVATE hidapi-hidraw)
endif()
if(WIN32)
  find_package(hidapi REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE hidapi::hidapi)
endif()
