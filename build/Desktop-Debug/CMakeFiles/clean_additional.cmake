# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/alise_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/alise_autogen.dir/ParseCache.txt"
  "_deps/gen-build/CMakeFiles/gen_autogen.dir/AutogenUsed.txt"
  "_deps/gen-build/CMakeFiles/gen_autogen.dir/ParseCache.txt"
  "_deps/gen-build/gen_autogen"
  "alise_autogen"
  "gitversion/CMakeFiles/gitversion_autogen.dir/AutogenUsed.txt"
  "gitversion/CMakeFiles/gitversion_autogen.dir/ParseCache.txt"
  "gitversion/gitversion_autogen"
  "interfaces/CMakeFiles/interfaces_autogen.dir/AutogenUsed.txt"
  "interfaces/CMakeFiles/interfaces_autogen.dir/ParseCache.txt"
  "interfaces/interfaces_autogen"
  )
endif()
