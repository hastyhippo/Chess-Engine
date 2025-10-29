cmake_minimum_required(VERSION 3.15)
project(ChessEngine CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Important: Specify this is a console application
set(CMAKE_RC_FLAGS "-DUNICODE -D_UNICODE")
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
else()
    # For MinGW
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,console")
endif()

# Compiler warnings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Include directories
include_directories(src)

# Find source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(chess_engine ${SOURCES})