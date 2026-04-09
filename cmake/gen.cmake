include(FetchContent)

FetchContent_Declare(avm-gen
  GIT_REPOSITORY    git@github.com:avm-energo/avm-gen.git
  GIT_TAG           main
)

set(GEN_STATIC ON)

FetchContent_MakeAvailable(avm-gen)
