include(FetchContent)

FetchContent_Declare(
  gen
  GIT_REPOSITORY https://github.com/avm-energo/gen.git
#  if(ALISE_BUILDFORDEB STREQUAL "buster")
#    GIT_TAG        origin/alice-buster
#elseif(ALISE_BUILDFORDEB STREQUAL "bullseye")
    GIT_TAG        353366cbfa7688fa3714db38f7c9f58cd5d87204
)

#if(BUILD_WITH_ALISE)
  set(GEN_STATIC ON)
#else()
#  set(GEN_STATIC OFF)
#endif()

FetchContent_MakeAvailable(gen)
