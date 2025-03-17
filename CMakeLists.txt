cmake_minimum_required (VERSION 3.22)
message("CMake Version: ${CMAKE_VERSION}")

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project (GALectures)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")
endif()

set(ASSETS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/Assets" CACHE STRING "Path to the Assets folder")

add_subdirectory(ThirdParty)
add_subdirectory(Common)
add_subdirectory(PGA)