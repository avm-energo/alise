include(FetchContent)

FetchContent_Declare(
  gen
  GIT_REPOSITORY https://git.avmenergo.ru/avm-energo/gen.git
  GIT_TAG        v1.6.0
)

FetchContent_MakeAvailable(gen)
