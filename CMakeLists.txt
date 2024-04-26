cmake_minimum_required(VERSION 3.18)
project(alise VERSION 2.1.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(ALISE_STM "Build with STM32 support" OFF)
option(ALISE_LOCALDEBUG "Local debug on PC" OFF)
#set(ALISE_BUILDFORDEB "bullseye" CACHE STRING "debian version to build for")
set(ALISE_POSTFIX)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Xml REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (ALISE_LOCALDEBUG)
    add_compile_definitions(ALISE_LOCALDEBUG)
endif()

if (ALISE_STM)
    set(AVTUK_MODULE_PATH avtuk-14)
    set(ALISE_POSTFIX stm)
    add_compile_definitions(AVTUK_STM)
else()
    set(AVTUK_MODULE_PATH avtuk-12)
    set(ALISE_POSTFIX nostm)
    find_library(GPIODCXX_LIBRARY NAMES libgpiodcxx.so)
    if(NOT GPIODCXX_LIBRARY)
      message(FATAL_ERROR "gpiod library not found. Install apt install libgpiod-dev")
    endif()
endif()

include(gen)

# Downloading conan CMake script and using for autodetecting
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                EXPECTED_HASH SHA256=5cdb3042632da3efff558924eecefd580a0e786863a857ca097c3d1d43df5dcd
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

message(STATUS "Conan settings: ${settings}")

conan_cmake_configure(
    REQUIRES cppzmq/4.7.1
    REQUIRES protobuf/3.16.0
    GENERATORS cmake_find_package)

conan_cmake_install(
  PATH_OR_REFERENCE .
  BUILD missing
  SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")

find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)
find_package(Protobuf REQUIRED)

set(SOURCES
    main.cpp
    aliseconstants.cpp
    aliseconstants.h
    alisesettings.cpp
    alisesettings.h
    avtukccu.h
    broker.cpp
    broker.h
    commandlineparser.cpp
    commandlineparser.h
    controller.cpp
    controller.h
    controllerfabric.cpp
    controllerfabric.h
    helper.h
    logger.cpp
    logger.h
    maincreator.cpp
    maincreator.h
    recovery.cpp
    recovery.h
    timesyncronizer.cpp
    timesyncronizer.h
    zeropublisher.cpp
    zeropublisher.h
    zerorunner.cpp
    zerorunner.h
    zerosubscriber.cpp
    zerosubscriber.h
    ${AVTUK_MODULE_PATH}/interfaces.qrc
    protos/protos.proto
    )

if (ALISE_STM)
    list(APPEND SOURCES
         stmbroker.cpp
         stmbroker.h)
else()
    list(APPEND SOURCES
         gpiobroker.cpp
         gpiobroker.h)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
add_subdirectory(gitversion)
target_include_directories(${PROJECT_NAME} PUBLIC ${CONAN_INCLUDE_DIRS_CPPZMQ})

protobuf_generate(
    LANGUAGE cpp
    TARGET ${PROJECT_NAME}
    PROTOS ${PROJECT_SOURCE_DIR}/protos/protos.proto)

configure_file(${CMAKE_SOURCE_DIR}/cconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

if (ALISE_STM)
    add_subdirectory(interfaces)
    ##WARNING Link interfaces before gen (!)
    target_link_libraries(${PROJECT_NAME} PUBLIC interfaces)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC gen ${GPIODCXX_LIBRARY})
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Xml pthread gitversion ZeroMQ::libzmq-static cppzmq::cppzmq protobuf::libprotobuf)

install(TARGETS ${PROJECT_NAME})

set(VERSION_IN ${CMAKE_CURRENT_SOURCE_DIR}/VERSION.in)
set(VERSION_OUT ${CMAKE_CURRENT_SOURCE_DIR}/package/src/VERSION)
configure_file(${VERSION_IN} ${VERSION_OUT} @ONLY)
