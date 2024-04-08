include(FetchContent)

FetchContent_Declare(
  gen
  GIT_REPOSITORY https://github.com/avm-energo/gen.git
  GIT_TAG        origin/main
)

set(GEN_STATIC ON)
set(GEN_USING_SANITIZERS OFF)
set(GEN_BUILD_TESTS OFF)

FetchContent_MakeAvailable(gen)
