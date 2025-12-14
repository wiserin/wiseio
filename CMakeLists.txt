cmake_minimum_required(VERSION 3.12)

include(FetchContent)
FetchContent_Declare(
    WiseLogging
    GIT_REPOSITORY https://github.com/wiserin/CPP-logging.git
    GIT_TAG v1.0.2
)
FetchContent_MakeAvailable(WiseLogging)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

project(
    WiseIO
    VERSION 1.0
    DESCRIPTION "Wise IO controller"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

enable_testing()

add_subdirectory(lib)
# add_subdirectory(bin)

add_subdirectory(tests)
