project(alise VERSION 1.1.429)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5Core)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Xml REQUIRED)

if (AVTUK-14)
    set(AVTUK_MODULE_PATH avtuk-14)
    add_compile_definitions(AVTUK_STM)
elseif  (AVTUK-12)
    set(AVTUK_MODULE_PATH avtuk-12)
    add_compile_definitions(AVTUK_NO_STM)
    find_library(GPIODCXX_LIBRARY NAMES libgpiodcxx.so)
    if(NOT GPIODCXX_LIBRARY)
      message(FATAL_ERROR "gpiod library not found. Install apt install libgpiod-dev")
    endif()
endif()

if (NOT ( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.txt" AND ${CACHED_PROJECT_TARGET_NAME} STREQUAL ${PROJECT_TARGET_NAME}))

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env
            CONAN_USER_HOME=${USER_DIRECTORY} ${CONAN_EXEC} install . --profile ${CONAN_PROFILE} -g cmake -g cmake_find_package -s arch=${CONAN_TARGET_NAME} -s arch_build=${CONAN_TARGET_NAME} -if ${CMAKE_CURRENT_BINARY_DIR} --build=missing
        RESULT_VARIABLE CMD_ERROR_CODE
        OUTPUT_VARIABLE CMD_OUTPUT
        ERROR_VARIABLE CMD_ERROR
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    message(STATUS "Init conan for " ${PROJECT_TARGET_NAME})
    message(STATUS "CMD_ERROR_CODE: ${CMD_ERROR} with  ${CMD_OUTPUT} and ${CMD_ERROR}")

endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")

find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)

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
    zeropublisher.cpp
    zeropublisher.h
    zerosubscriber.cpp
    zerosubscriber.h
    ${AVTUK_MODULE_PATH}/interfaces.qrc
    recovery.cpp
    recovery.h)

if (AVTUK-14)
    list(APPEND SOURCES
        stmbroker.cpp
        stmbroker.h)
elseif  (AVTUK-12)
    list(APPEND SOURCES
        gpiobroker.cpp
        gpiobroker.h)
endif()
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CONAN_INCLUDE_DIRS_CPPZMQ})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROTOS_GENERATED_DIR})

configure_file(${CMAKE_SOURCE_DIR}/cconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

message(STATUS ALISE ${protos_DIR})
if (AVTUK-14)
    ##WARNING Link interfaces before gen (!)
    target_link_libraries(${PROJECT_NAME} interfaces)
endif()
if (AVTUK-12)
    target_link_libraries(${PROJECT_NAME} ${GPIODCXX_LIBRARY})
endif()
target_link_libraries(${PROJECT_NAME}  gen comaversion protos Qt5::Core Qt5::Xml pthread ZeroMQ::libzmq-static cppzmq::cppzmq)


install(TARGETS ${PROJECT_NAME}
)

###### DEB PACKAGE SECTION ######

if (AVTUK-14)
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
# Configure the scripts that are executed by the DEB.
# This could be done without going through the rigmarole of
# using configures, but I don't want to hard-code the name
# of the service into each file.
#
# Each script has the option of performing verbose logging
# Switch this variable to TRUE to enable those logs.
# I found this helpful while troubeshooting package upgrades.
set(VERBOSE_LOGGING FALSE)
set(POSTINST_IN ${CMAKE_CURRENT_LIST_DIR}/${AVTUK_MODULE_PATH}/postinst.in)
set(POSTINST_OUT ${CMAKE_CURRENT_BINARY_DIR}/postinst)
configure_file(${POSTINST_IN} ${POSTINST_OUT} @ONLY)
set(POSTRM_IN ${CMAKE_CURRENT_LIST_DIR}/${AVTUK_MODULE_PATH}/postrm.in)
set(POSTRM_OUT ${CMAKE_CURRENT_BINARY_DIR}/postrm)
configure_file(${POSTRM_IN} ${POSTRM_OUT} @ONLY)
set(PRERM_IN ${CMAKE_CURRENT_LIST_DIR}/${AVTUK_MODULE_PATH}/prerm.in)
set(PRERM_OUT ${CMAKE_CURRENT_BINARY_DIR}/prerm)
configure_file(${PRERM_IN} ${PRERM_OUT} @ONLY)

set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
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
    "${POSTINST_OUT}" "${POSTRM_OUT}" "${PRERM_OUT}")
set(CPACK_DEB_POST_INSTALL_SCRIPT_FILE ${POSTINST_OUT})
set(CPACK_DEB_POST_UNINSTALL_SCRIPT_FILE ${POSTUN_OUT})
set(CPACK_DEB_PRE_UNINSTALL_SCRIPT_FILE ${PREUN_OUT})
set(CPACK_DEB_PACKAGE_REQUIRES_POSTUN "systemd")
set(CPACK_DEB_PACKAGE_REQUIRES_PREUN "systemd")
set(CPACK_DEB_PACKAGE_REQUIRES_POST "systemd")
include(CPack)
