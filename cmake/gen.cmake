include(FetchContent)

FetchContent_Declare(
  gen
  GIT_REPOSITORY https://github.com/avm-energo/gen.git
  GIT_TAG        origin/alise-bullseye
)

set(GEN_STATIC ON)

FetchContent_MakeAvailable(gen)
