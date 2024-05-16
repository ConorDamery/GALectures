cmake_minimum_required (VERSION 3.22)
message("CMake Version: ${CMAKE_VERSION}")

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project (GALectures)

add_subdirectory(ThirdParty)
add_subdirectory(Common)

add_subdirectory(Pt1)
add_subdirectory(Pt2)