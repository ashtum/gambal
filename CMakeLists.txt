cmake_minimum_required(VERSION 3.2)

project(
  gambal
  VERSION 1.4.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
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

add_executable(gambal src/gambal.cpp)
target_link_libraries(gambal ${X11_LIBRARIES})

install(TARGETS gambal DESTINATION bin)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/gambal.desktop
        DESTINATION /usr/share/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/gambal.desktop
        DESTINATION /etc/xdg/autostart)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/gambal.png
        DESTINATION /usr/share/icons/hicolor/512x512/apps/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/gambal.svg
        DESTINATION /usr/share/icons/hicolor/scalable/apps/)
