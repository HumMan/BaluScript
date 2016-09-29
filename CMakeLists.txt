cmake_minimum_required(VERSION 2.8)

project(BaluScript)

file(GLOB_RECURSE BaluScript_SRC
    "Source/*.h"
    "Source/*.cpp"
)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -Wwrite-strings")
endif()

include_directories("Include")

add_library(BaluScript ${BaluScript_SRC})
