cmake_minimum_required(VERSION 3.18)
project(alise VERSION 3.0.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(ALISE_STM "Build with STM32 support" OFF)
option(ALISE_LOCALDEBUG "Local debug on PC" OFF)

find_package(QT NAMES Qt6)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Xml HttpServer REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (ALISE_LOCALDEBUG)
    add_compile_definitions(ALISE_LOCALDEBUG)
endif()

if (ALISE_STM)
    set(AVTUK_MODULE_PATH avtuk-14)
    add_compile_definitions(AVTUK_STM)
else()
    set(AVTUK_MODULE_PATH avtuk-12)
    find_library(GPIODCXX_LIBRARY NAMES libgpiodcxx.so)
    if(NOT GPIODCXX_LIBRARY)
      message(FATAL_ERROR "gpiod library not found. Install apt install libgpiod-dev")
    endif()
endif()

include(gen)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")

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
    engine.cpp
    engine.h
    helper.h
    httpapiserver.h
    httpapiserver.cpp
    httpmiddleware.h
    httpmiddleware.cpp
    logger.cpp
    logger.h
    recovery.cpp
    recovery.h
    timesyncronizer.cpp
    timesyncronizer.h
    ${AVTUK_MODULE_PATH}/interfaces.qrc
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

configure_file(${CMAKE_SOURCE_DIR}/cconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

if (ALISE_STM)
    add_subdirectory(interfaces)
    ##WARNING Link interfaces before gen (!)
    target_link_libraries(${PROJECT_NAME} PUBLIC interfaces)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC gen ${GPIODCXX_LIBRARY})
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::Xml Qt6::HttpServer pthread gitversion)

install(TARGETS ${PROJECT_NAME})

set(VERSION_IN ${CMAKE_CURRENT_SOURCE_DIR}/VERSION.in)
set(VERSION_OUT ${CMAKE_CURRENT_SOURCE_DIR}/package/src/VERSION)
configure_file(${VERSION_IN} ${VERSION_OUT} @ONLY)
