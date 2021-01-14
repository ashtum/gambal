cmake_minimum_required(VERSION 3.2)

project(
  ashmon
  VERSION 2.0.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wfatal-errors -Wextra -Wnon-virtual-dtor -pedantic)

if(NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "" FORCE)
  endif()
endif()

find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})
link_directories(${X11_LIBRARIES})

add_executable(ashmon src/ashmon.cpp)
target_link_libraries(ashmon ${X11_LIBRARIES})

install(TARGETS ashmon DESTINATION bin)
