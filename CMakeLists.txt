cmake_minimum_required(VERSION 2.6)
project(alise)

add_definitions(-std=c++11)

find_path(LibArchive_INCLUDE_DIR
    NAMES archive.h
    PATHS
    "/usr/include"
    )
    
find_library(LibArchive_LIBRARY
    NAMES archive libarchive
    PATHS
    "/usr/lib"
    )
    
    find_package(Threads)
    
add_executable(alise
    main.cpp
    aliseserver.cpp
    blinker.cpp
    connectionthread.cpp
    global.cpp
    log.cpp
    )

target_link_libraries(alise ${LibArchive_LIBRARY})

target_link_libraries(alise ${CMAKE_THREAD_LIBS_INIT})
    
install(TARGETS alise RUNTIME DESTINATION bin)
