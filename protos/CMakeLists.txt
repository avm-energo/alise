cmake_minimum_required(VERSION 3.14)

project(protos CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

conan_cmake_configure(
    REQUIRES protobuf/3.17.1
    GENERATORS cmake_find_package)

conan_cmake_install(
  PATH_OR_REFERENCE .
  BUILD missing
  REMOTE conancenter
  SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "Protobuf cmake search path: ${CMAKE_CURRENT_BINARY_DIR}")
find_package(Protobuf REQUIRED)

include(ProtobufGenerateWrapper)
set(PROTO_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "PROTO_FOLDER: ${PROTO_FOLDER}")
set(PROTO_SRCS)
set(PROTO_HDRS)
generate_protos(${PROTO_FOLDER})

set(${PROJECT_NAME}_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Protobuf generated files" FORCE)
message(STATUS "PROTOBUF_GENERATED ${PROTOS_GENERATED_DIR}")
message(STATUS "PROTOBUF_INCLUDE_DIR ${PROTOBUF_INCLUDE_DIR}")
message(STATUS "PROTOBUF_EXECUTABLE ${PROTOBUF_PROTOC_EXECUTABLE}")

message(STATUS "Generated srcs: " ${PROTO_SRCS})
message(STATUS "Generated hdrs: " ${PROTO_HDRS})

add_library(${PROJECT_NAME} STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${protobuf_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::protobuf)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_LIBRARY)
