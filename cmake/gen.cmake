include(FetchContent)

FetchContent_Declare(
  gen
  GIT_REPOSITORY https://github.com/avm-energo/gen.git
  GIT_TAG        ea63d22cc9e8d4a148d408a35fdb034f3b443dc4
)

set(GEN_STATIC ON)

FetchContent_MakeAvailable(gen)
