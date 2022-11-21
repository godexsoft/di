include(FetchContent)

FetchContent_Declare(
    GoogleTest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG origin/main
)

FetchContent_MakeAvailable(GoogleTest)
