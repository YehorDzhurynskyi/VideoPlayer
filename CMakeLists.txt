cmake_minimum_required(VERSION 3.1.0)

if (NOT EPI_BUILD_DIR)
    message(FATAL_ERROR "EPI_BUILD_DIR is undefined")
endif()

include("${EPI_BUILD_DIR}/Scripts/CMake/EpiHelpers.cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(VideoPlayer)

add_subdirectory(Extern/EpiLib)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/Extern/SDL2")
set(SDL2_ROOT_DIR "Extern/SDL2")
find_package(SDL2 MODULE REQUIRED)

add_subdirectory(VideoPlayer)
