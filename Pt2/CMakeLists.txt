add_executable (Pt2 "Main.cpp")
target_link_libraries (Pt2 Common)

# Define the list of folders to copy
set(PROJECT_DIRS 
    "${PROJECT_PATH}/Common/Fonts" 
    "${PROJECT_PATH}/Common/Scripts"
    "${PROJECT_PATH}/Pt2/Scripts"
    "${PROJECT_PATH}/Pt2/Shaders"
)

foreach(FOLDER ${PROJECT_DIRS})
    if(EXISTS ${FOLDER})
        # Extract only the folder name (e.g., "Scripts", "Shaders", "Fonts")
        get_filename_component(FOLDER_NAME ${FOLDER} NAME)
        
        # Ensure the target folder exists
        add_custom_command(TARGET Pt2 PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Pt2>/${FOLDER_NAME}"
        )

        message(STATUS "Merging folder: ${FOLDER} into $<TARGET_FILE_DIR:Pt2>/${FOLDER_NAME}")

        # Copy files instead of replacing the whole folder
        add_custom_command(TARGET Pt2 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory 
                "${FOLDER}/"  # Copy contents, not the folder itself
                "$<TARGET_FILE_DIR:Pt2>/${FOLDER_NAME}/"
            COMMENT "Merging ${FOLDER} into $<TARGET_FILE_DIR:Pt2>/${FOLDER_NAME}"
        )
    else()
        message(WARNING "Folder not found: ${FOLDER}, skipping copy.")
    endif()
endforeach()