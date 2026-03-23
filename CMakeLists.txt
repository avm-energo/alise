cmake_minimum_required(VERSION 3.18)
project(alise VERSION 4.0.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(ALISE_STM "Build with STM32 support" OFF)
option(ALISE_LOCALDEBUG "Local debug on PC" OFF)

find_package(QT NAMES Qt6)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core HttpServer REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (ALISE_LOCALDEBUG)
    add_compile_definitions(ALISE_LOCALDEBUG)
endif()

if (ALISE_STM)
    set(AVTUK_MODULE_PATH avtuk-14)
    add_compile_definitions(AVTUK_STM)
else()
    set(AVTUK_MODULE_PATH avtuk-12)
    find_package(PkgConfig REQUIRED)
    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6 AND ${QT_VERSION_MINOR} GREATER_EQUAL 8)
        pkg_check_modules(GPIOD REQUIRED libgpiodcxx)
        set(GPIOD_VERS 2)
        message(STATUS "Using gpiod library, C++ version")
    else()
        pkg_check_modules(GPIOD REQUIRED libgpiod)
        set(GPIOD_VERS 1)
        message(STATUS "Using gpiod library, C version")
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
    ntpstatus.h
    ntpstatus.cpp
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
    if(${GPIOD_VERS} EQUAL 2)
    list(APPEND SOURCES
         gpiobroker.cpp
         gpiobroker.h)
 else()
     list(APPEND SOURCES
          gpiobroker1.cpp
          gpiobroker1.h)
  endif()
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
add_subdirectory(gitversion)

configure_file(${CMAKE_SOURCE_DIR}/cconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

target_include_directories(${PROJECT_NAME} PRIVATE ${avm-gen_SOURCE_DIR}/include)

if (ALISE_STM)
    add_subdirectory(interfaces)
    target_link_libraries(${PROJECT_NAME} PUBLIC interfaces)
else()
    include_directories(${GPIOD_INCLUDE_DIRS})
    link_directories(${GPIOD_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC avm-gen ${GPIOD_LIBRARIES})
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::HttpServer pthread gitversion)

install(TARGETS ${PROJECT_NAME})

set(VERSION_IN ${CMAKE_CURRENT_SOURCE_DIR}/VERSION.in)
set(VERSION_OUT ${CMAKE_CURRENT_SOURCE_DIR}/package/src/VERSION)
configure_file(${VERSION_IN} ${VERSION_OUT} @ONLY)
