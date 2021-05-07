cmake_minimum_required(VERSION 3.20)
project(shared_flag LANGUAGES CXX VERSION 0.1.0)

# Require C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable all compiler warnings and treat warnings as errors for all targets.
# TODO: Possibly leave this out of the CMake script? It's not technically a build requirement.
if(MSVC)
    add_compile_options(-W4 -WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# TODO: Add clang tidy, cpp lint, include-what-you-use, and link-what-you-use.
#       Maybe better in a separate CI script?

# Define the library target.
add_library(shared_flag STATIC "")
# target_compile_features(shared_flag PUBLIC cxx_std_17) # <-- not needed?
target_include_directories(shared_flag PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_sources(shared_flag PRIVATE
    ${CMAKE_SOURCE_DIR}/include/shared_flag/shared_flag_reader.hpp
    ${CMAKE_SOURCE_DIR}/include/shared_flag/shared_flag.hpp
    ${CMAKE_SOURCE_DIR}/src/shared_flag_reader.cpp
    ${CMAKE_SOURCE_DIR}/src/shared_flag.cpp
)

# Download the unit test framework.
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/703bd9caab50b139428cea1aaff9974ebee5742e.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Define the unit test target.
add_executable(shared_flag.test "")
target_link_libraries(shared_flag.test shared_flag gtest_main)
target_include_directories(shared_flag.test PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_sources(shared_flag.test PRIVATE
    ${CMAKE_SOURCE_DIR}/include/shared_flag/shared_flag_reader.hpp
    ${CMAKE_SOURCE_DIR}/include/shared_flag/shared_flag.hpp    
    ${CMAKE_SOURCE_DIR}/src/shared_flag_reader.cpp
    ${CMAKE_SOURCE_DIR}/src/shared_flag.cpp
    ${CMAKE_SOURCE_DIR}/test/shared_flag_reader.test.cpp
    ${CMAKE_SOURCE_DIR}/test/shared_flag.test.cpp
)

# Tell CMake how to run our unit tests.
include(GoogleTest)
enable_testing()
gtest_discover_tests(shared_flag.test)
