cmake_minimum_required(VERSION 3.21 FATAL_ERROR) # not really checked
project(java2c VERSION 0.1.0)

enable_language(C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Werror -Wpedantic ${CMAKE_C_FLAGS}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -march=native -mtune=native")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fsanitize=leak,address,undefined,float-divide-by-zero")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG -march=native -mtune=native -s -static")
endif()

add_executable(main src/main.c src/utils.c src/input.c)
target_include_directories(main PRIVATE "include")

message(STATUS "")
message(STATUS "Successfully configured ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER : ${CMAKE_C_COMPILER_ID} v${CMAKE_C_COMPILER_VERSION}")
message(STATUS "")
