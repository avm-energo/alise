include(FetchContent)

FetchContent_Declare(
  gen
  GIT_REPOSITORY https://github.com/avm-energo/gen.git
#  if(ALISE_BUILDFORDEB STREQUAL "buster")
#    GIT_TAG        origin/alice-buster
#elseif(ALISE_BUILDFORDEB STREQUAL "bullseye")
    GIT_TAG        4ad32f8ab2e9d368884bed21aeaae1a4f02dfbac
)

set(GEN_STATIC ON)

FetchContent_MakeAvailable(gen)
