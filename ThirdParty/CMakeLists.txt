# GLFW lib
add_subdirectory(glfw)

# GLAD lib
add_library (glad STATIC "glad/src/glad.c")
if (UNIX)
	target_link_libraries (glad dl)
endif()
target_include_directories (glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

# ImGui lib
add_library (imgui STATIC
	"imgui/imgui.cpp"
	"imgui/imgui_demo.cpp"
	"imgui/imgui_draw.cpp"
	"imgui/imgui_tables.cpp"
	"imgui/imgui_widgets.cpp"
	"imgui/misc/cpp/imgui_stdlib.cpp"
	#"imgui/misc/freetype/imgui_freetype.cpp"
	"imgui/backends/imgui_impl_glfw.cpp"
	"imgui/backends/imgui_impl_opengl3.cpp"
)
target_include_directories (imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
target_link_libraries (imgui glfw glad)

# Wren lib
add_library (wren STATIC
	"wren/src/optional/wren_opt_meta.c"
	"wren/src/optional/wren_opt_random.c"
	"wren/src/vm/wren_compiler.c"
	"wren/src/vm/wren_core.c"
	"wren/src/vm/wren_debug.c"
	"wren/src/vm/wren_primitive.c"
	"wren/src/vm/wren_utils.c"
	"wren/src/vm/wren_value.c"
	"wren/src/vm/wren_vm.c"
)
target_include_directories (wren PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/wren/src/include ${CMAKE_CURRENT_SOURCE_DIR}/wren/src/optional ${CMAKE_CURRENT_SOURCE_DIR}/wren/src/vm)