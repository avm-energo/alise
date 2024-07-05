# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-src"
  "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-build"
  "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-subbuild/gen-populate-prefix"
  "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-subbuild/gen-populate-prefix/tmp"
  "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-subbuild/gen-populate-prefix/src/gen-populate-stamp"
  "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-subbuild/gen-populate-prefix/src"
  "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-subbuild/gen-populate-prefix/src/gen-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-subbuild/gen-populate-prefix/src/gen-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/anton/repos/alise/build/Desktop-Debug/_deps/gen-subbuild/gen-populate-prefix/src/gen-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
