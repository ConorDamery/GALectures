add_library (Common STATIC
	"App.cpp"
)
target_link_libraries (Common glfw glad imgui wren)
target_include_directories (Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})