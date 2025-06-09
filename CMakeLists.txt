cmake_minimum_required (VERSION 3.22)
message("CMake Version: ${CMAKE_VERSION}")

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project (GASandbox)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")
endif()

set(ASSETS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/Assets" CACHE STRING "Path to the Assets folder")

add_subdirectory(ThirdParty)

add_library (GASandboxLib STATIC
	"Source/App.cpp"
    "Source/Backend/File.cpp"
    "Source/Backend/Win.cpp"
    "Source/Backend/Gl.cpp"
    "Source/Backend/Gui.cpp"
    "Source/Backend/Sfx.cpp"
    "Source/Backend/Net.cpp"
    "Source/Backend/Wren.cpp"
)
target_link_libraries (GASandboxLib ghc_filesystem stb cereal cgltf miniaudio glfw enet_static glad imgui wren)
target_include_directories (GASandboxLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")

if(WIN32)
    set(ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/GASandbox.rc")
    add_executable(GASandbox WIN32 "Source/Main.cpp" ${ICON_RESOURCE})
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(GASandbox PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    else()
        set_target_properties(GASandbox PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif()
else()
    add_executable(GASandbox "Source/Main.cpp")
endif()

target_link_libraries(GASandbox GASandboxLib)