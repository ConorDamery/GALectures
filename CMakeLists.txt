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

add_library (Common STATIC
	"Source/App.cpp"
    "Source/Net.cpp"
)
target_link_libraries (Common stb glfw enet_static glad imgui wren)
target_include_directories (Common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Source")

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

target_link_libraries(GASandbox Common)

# Generate manifest file for assets (always needed)
set(MANIFEST_FILE "${ASSETS_FOLDER}/manifest.txt")
file(GLOB_RECURSE ASSET_FILES "${ASSETS_FOLDER}/*")

# Clear the manifest file before writing
file(WRITE "${MANIFEST_FILE}" "")

# Write the relative paths of asset files into manifest.txt
foreach(FILE ${ASSET_FILES})
    file(RELATIVE_PATH REL_PATH "${ASSETS_FOLDER}" "${FILE}")
    file(APPEND "${MANIFEST_FILE}" "Assets/${REL_PATH}\n")
endforeach()

# Ensure manifest.txt is copied for release mode
if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    # Custom target to sync files
    add_custom_target(sync_assets ALL)

    if(EXISTS ${ASSETS_FOLDER})
        set(DEST_FOLDER "$<TARGET_FILE_DIR:GASandbox>/Assets")

        message(STATUS "Merging folder: ${ASSETS_FOLDER} into ${DEST_FOLDER}")

        # Ensure the destination folder exists
        add_custom_command(TARGET sync_assets PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_FOLDER}"
        )

        # Copy files while keeping existing ones
        add_custom_command(TARGET sync_assets POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_FOLDER}/" "${DEST_FOLDER}/"
            COMMENT "Merging ${ASSETS_FOLDER} into ${DEST_FOLDER}"
        )

        # Ensure manifest.txt is copied as well
        install(FILES "${MANIFEST_FILE}" DESTINATION "bin/Assets")
    else()
        message(WARNING "Assets folder not found: ${ASSETS_FOLDER}, skipping copy.")
    endif()

    # Ensure `sync_assets` runs before `GASandbox`
    add_dependencies(GASandbox sync_assets)

    # Install the executable
    install(TARGETS GASandbox DESTINATION bin)

    # Install the entire `Assets/` folder
    install(DIRECTORY "${ASSETS_FOLDER}/" DESTINATION "bin/Assets"
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
    )
endif()