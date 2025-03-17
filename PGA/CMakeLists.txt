add_executable(PGA "Main.cpp")
target_link_libraries(PGA Common)

if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    # Custom target to sync files
    add_custom_target(sync_assets ALL)

    if(EXISTS ${ASSETS_FOLDER})
        set(DEST_FOLDER "$<TARGET_FILE_DIR:PGA>/Assets")

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
    else()
        message(WARNING "Assets folder not found: ${ASSETS_FOLDER}, skipping copy.")
    endif()

    # Ensure `sync_assets` runs before `PGA`
    add_dependencies(PGA sync_assets)

    # Install the executable
    install(TARGETS PGA DESTINATION bin)

    # Install the entire `Assets/` folder
    install(DIRECTORY "${ASSETS_FOLDER}/" DESTINATION "bin/Assets"
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*"
    )
endif()