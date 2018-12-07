cmake_minimum_required(VERSION 2.6)
project(alise)

add_executable(alise main.cpp)

install(TARGETS alise RUNTIME DESTINATION bin)
