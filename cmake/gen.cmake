include(FetchContent)

FetchContent_Declare(
  gen
  GIT_REPOSITORY https://github.com/avm-energo/gen.git
  GIT_TAG        baea5a763413493c523682efd7c1f8b393f4adfc
)

set(GEN_STATIC ON)
set(GEN_USING_SANITIZERS OFF)
set(GEN_BUILD_TESTS OFF)

FetchContent_MakeAvailable(gen)
