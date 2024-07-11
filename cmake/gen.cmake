include(FetchContent)

FetchContent_Declare(
  gen
  GIT_REPOSITORY https://git.avmenergo.ru/avm-energo/gen.git
  GIT_TAG        v1.4.0
)

set(GEN_STATIC ON)
set(GEN_USING_SANITIZERS OFF)
set(GEN_BUILD_TESTS OFF)

FetchContent_MakeAvailable(gen)
