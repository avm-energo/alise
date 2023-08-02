cmake_minimum_required(VERSION 3.18)
project(alise VERSION 1.2.2)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

option(ALISE_STM "Build with STM32 support" OFF)
option(ALISE_LOCALDEBUG "Local debug on PC" OFF)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Xml REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (ALISE_LOCALDEBUG)
    add_compile_definitions(ALISE_LOCALDEBUG)
endif()

if (ALISE_STM)
    set(AVTUK_MODULE_PATH avtuk-14)
    add_compile_definitions(AVTUK_STM)
else()
    set(AVTUK_MODULE_PATH avtuk-12)
    add_compile_definitions(AVTUK_NO_STM)
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
  REMOTE conancenter
  SETTINGS ${settings})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")

find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)
find_package(Protobuf REQUIRED)

set(SOURCES
    main.cpp
    avtukccu.h
    timesyncronizer.cpp
    timesyncronizer.h
    helper.h
    zerorunner.cpp
    zerorunner.h
    controller.cpp
    controller.h
    controllerfabric.cpp
    controllerfabric.h
    zeropublisher.cpp
    zeropublisher.h
    zerosubscriber.cpp
    zerosubscriber.h
    ${AVTUK_MODULE_PATH}/interfaces.qrc
    recovery.cpp
    recovery.h
    aliseconstants.cpp
    aliseconstants.h
    protos/protos.proto
    broker.cpp
    broker.h
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
add_subdirectory(interfaces)
target_include_directories(${PROJECT_NAME} PUBLIC ${CONAN_INCLUDE_DIRS_CPPZMQ})

protobuf_generate(
    LANGUAGE cpp
    TARGET ${PROJECT_NAME}
    PROTOS ${PROJECT_SOURCE_DIR}/protos/protos.proto)

configure_file(${CMAKE_SOURCE_DIR}/cconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

if (ALISE_STM)
    ##WARNING Link interfaces before gen (!)
    target_link_libraries(${PROJECT_NAME} interfaces)
else()
    target_link_libraries(${PROJECT_NAME} ${GPIODCXX_LIBRARY})
endif()
target_link_libraries(${PROJECT_NAME} gen Qt5::Core Qt5::Xml pthread gitversion ZeroMQ::libzmq-static cppzmq::cppzmq protobuf::libprotobuf)

install(TARGETS ${PROJECT_NAME})

###### DEB PACKAGE SECTION ######

if (ALISE_STM)
    set(REQUIRED_SERVICE_PATH)
endif()
# Configure the service file so that it knows where the
# binary was installed.
set(SERVICE_NAME ${PROJECT_NAME}.service)
if (REQUIRED_SERVICE_PATH)
    set(SERVICE_PATH_NAME ${PROJECT_NAME}.path)
endif()
set(EXE_PATH ${CPACK_PACKAGING_INSTALL_PREFIX}/bin/${PROJECT_NAME})
set(SERVICE_IN ${SERVICE_NAME}.in)
set(SERVICE_OUT ${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_NAME})
configure_file(${SERVICE_IN} ${SERVICE_OUT} @ONLY)

# Install the service file in the share directory. The post
# install DEB script will handle placing the file where
# it's supposed to go.
install(FILES ${SERVICE_OUT} DESTINATION share)
if (REQUIRED_SERVICE_PATH)
    install(FILES ${SERVICE_PATH_NAME} DESTINATION share)
endif()

if (NOT EXISTS /root/sonica/alise/settings/settings.ini)
    install(FILES settings.ini DESTINATION /root/sonica/alise/settings)
endif ()

# Configure the scripts that are executed by the DEB.
# This could be done without going through the rigmarole of
# using configures, but I don't want to hard-code the name
# of the service into each file.
#
# Each script has the option of performing verbose logging
# Switch this variable to TRUE to enable those logs.
# I found this helpful while troubeshooting package upgrades.
set(VERBOSE_LOGGING FALSE)
set(POSTINST_IN ${CMAKE_CURRENT_SOURCE_DIR}/deb-scripts/postinst.in)
set(POSTINST_OUT ${CMAKE_CURRENT_BINARY_DIR}/postinst)
configure_file(${POSTINST_IN} ${POSTINST_OUT} @ONLY)
set(POSTRM_IN ${CMAKE_CURRENT_SOURCE_DIR}/deb-scripts/postrm.in)
set(POSTRM_OUT ${CMAKE_CURRENT_BINARY_DIR}/postrm)
configure_file(${POSTRM_IN} ${POSTRM_OUT} @ONLY)
set(PRERM_IN ${CMAKE_CURRENT_SOURCE_DIR}/deb-scripts/prerm.in)
set(PRERM_OUT ${CMAKE_CURRENT_BINARY_DIR}/prerm)
configure_file(${PRERM_IN} ${PRERM_OUT} @ONLY)

set(CONFFILE ${CMAKE_CURRENT_SOURCE_DIR}/deb-scripts/conffiles)

set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "AVM-Energo") #required
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Avtuk LInux SErver")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/package")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
#set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${POSTINST_OUT}" "${POSTRM_OUT}" "${PRERM_OUT}" "${CONFFILE}")
set(CPACK_DEB_POST_INSTALL_SCRIPT_FILE ${POSTINST_OUT})
set(CPACK_DEB_POST_UNINSTALL_SCRIPT_FILE ${POSTUN_OUT})
set(CPACK_DEB_PRE_UNINSTALL_SCRIPT_FILE ${PREUN_OUT})
set(CPACK_DEB_PACKAGE_REQUIRES_POSTUN "systemd")
set(CPACK_DEB_PACKAGE_REQUIRES_PREUN "systemd")
set(CPACK_DEB_PACKAGE_REQUIRES_POST "systemd")
include(CPack)
