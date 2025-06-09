# Filesystem
add_subdirectory (filesystem)

# STB lib
add_library(stb STATIC "stb/stb_vorbis.c")
target_include_directories (stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)

# Cereal lib
set (BUILD_DOC OFF CACHE BOOL "")
set (BUILD_SANDBOX OFF CACHE BOOL "")
set (SKIP_PERFORMANCE_COMPARISON ON CACHE BOOL "")

add_subdirectory (cereal)

# CGLTF lib
add_library(cgltf INTERFACE)
target_include_directories (cgltf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cgltf)

# Miniaudio lib
add_subdirectory(miniaudio)

# Klein lib
add_subdirectory(klein)

# GLFW lib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")

add_subdirectory(glfw)

# ENet lib
#set(ENET_TEST OFF CACHE BOOL "Build enet tests")

add_subdirectory(enet)

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