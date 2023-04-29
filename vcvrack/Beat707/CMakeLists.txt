cmake_minimum_required(VERSION 3.8)

project(Beat707)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable pthreads when using the GNU Compiler Collection (GCC) or Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Add include directories
include_directories(src src_logic src_test)

# Add src/hardware.cpp
set(SOURCE_FILES src/hardware.cpp)

# Add all *.cpp files in src_logic
file(GLOB SRC_LOGIC_FILES "src_logic/*.cpp")
list(APPEND SOURCE_FILES ${SRC_LOGIC_FILES})

# Add all *.cpp files in src_test
file(GLOB SRC_TEST_FILES "src_test/*.cpp")
list(APPEND SOURCE_FILES ${SRC_TEST_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Og")

add_executable(MyProject ${SOURCE_FILES})
