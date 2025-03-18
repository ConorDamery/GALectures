#include "App.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <wren.hpp>

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

struct LogData
{
	u32 color = 0xFFFFFFFF;
	std::string message{};
};

struct Vertex
{
	f32 pos[3] = { 0, 0, 0 };
	u32 col = 0;
};

struct AppData
{
	// Util
	std::vector<LogData> logs{};
	u32 frames{ 0 };
	f64 time{ 0 };
	f64 fps{ 0 };
	f64 spf{ 0 };

	// Window
	GLFWwindow* window{ nullptr };
	WindowMode winMode{ WindowMode::WINDOW };
	int winX{ 0 }, winY{ 0 };
	int winWidth{ 0 }, winHeight{ 0 };

	// Graphics
	GLuint shader{ 0 };
	GLuint vao{ 0 };
	GLuint vbo{ 0 };
	std::vector<GLuint> shaders{};
	const char* uniformName{ nullptr };
	std::vector<Vertex> vertices{};

	// Script
	WrenVM* vm{ nullptr };
	WrenHandle* mainClass{ nullptr };
	WrenHandle* initMethod{ nullptr };
	WrenHandle* updateMethod{ nullptr };
	WrenHandle* renderMethod{ nullptr };
	bool error{ false };
	bool paused{ false };

	FrameOp frameOp{ FrameOp::NONE };

	i64 current_path{ 0 };
	std::unordered_map<size_t, ScriptClass> classes{};
	std::unordered_map<size_t, ScriptMethodFn> methods{};

	// Editor
	f32 fontSize{ 1.0f };
	bool showSettings{ false };
	bool showConsole{ false };
};
static AppData g_app;

// Static utils
static std::string file_load(const char* filepath)
{
#if _DEBUG
	std::string pathStr = PROJECT_PATH;
	pathStr += filepath;
	const char* path = pathStr.c_str();
#else
	const char* path = filepath;
#endif

	std::ifstream file(path);
	if (!file.is_open())
	{
		LOGE("Error opening file: %s", filepath);
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return buffer.str();
}

static void file_save(const char* filepath, const std::string& src)
{
#if _DEBUG
	std::string pathStr = PROJECT_PATH;
	pathStr += filepath;
	const char* path = pathStr.c_str();
#else
	const char* path = filepath;
#endif

	std::ofstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Error opening file: " << filepath << std::endl;
		return;
	}

	file << src;
	file.close();
}

static void glfw_error_callback(int i, const char* c)
{
	LOGE("GLFW Error [%s]: ", c);
}

static bool glfw_initialize(const AppConfig& config)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		LOGE("Failed to initialize GLFW!");
		return false;
	}

	GLFWmonitor* pMonitor = nullptr;
	i32 width = config.width;
	i32 height = config.height;

#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#else
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, false);
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, config.msaa);

	if (config.windowMode == WindowMode::FULLSCREEN)
	{
		pMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
		width = pMode->width;
		height = pMode->height;

		glfwWindowHint(GLFW_RED_BITS, pMode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, pMode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, pMode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, pMode->refreshRate);
	}

	g_app.window = glfwCreateWindow(width, height, config.title, pMonitor, NULL);
	if (g_app.window == NULL)
	{
		LOGE("Failed to create GLFW window!");
		return false;
	}

	glfwMakeContextCurrent(g_app.window);
	glfwSwapInterval(1);

	if (config.windowMode == WindowMode::BORDERLESS)
	{
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

		glfwSetWindowAttrib(g_app.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(g_app.window, 0, 0);
		glfwSetWindowSize(g_app.window, mode->width, mode->height);
	}

	return true;
}

static void glfw_shutdown()
{
	glfwDestroyWindow(g_app.window);
	glfwTerminate();
}

#if _DEBUG
static const char* opengl_source(GLenum source)
{
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:               return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     return "Window System";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:   return "Shader Compiler";
	case GL_DEBUG_SOURCE_THIRD_PARTY:       return "Third Party";
	case GL_DEBUG_SOURCE_APPLICATION:       return "Application";
	case GL_DEBUG_SOURCE_OTHER:             return "Other";
	}

	return "Unknown";
}

static const char* opengl_type(GLenum type)
{
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               return "Error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behaviour";
	case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
	case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
	case GL_DEBUG_TYPE_MARKER:              return "Marker";
	case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push Group";
	case GL_DEBUG_TYPE_POP_GROUP:           return "Pop Group";
	case GL_DEBUG_TYPE_OTHER:               return "Other";
	}

	return "Unknown";
}

static const char* opengl_severity(GLenum severity)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:            return "High";
	case GL_DEBUG_SEVERITY_MEDIUM:          return "Medium";
	case GL_DEBUG_SEVERITY_LOW:             return "Low";
	case GL_DEBUG_SEVERITY_NOTIFICATION:    return "Notification";
	}

	return "Unknown";
}

static void APIENTRY opengl_debug_callback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// Ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	if (type == GL_DEBUG_TYPE_ERROR)
		LOGE("GL %s - %s (%s) [%d]: %s", opengl_source(source), opengl_type(type), opengl_severity(severity), id, message);
	else if (type == GL_DEBUG_TYPE_OTHER)
		LOGI("GL %s - %s (%s) [%d]: %s", opengl_source(source), opengl_type(type), opengl_severity(severity), id, message);
	else
		LOGW("GL %s - %s (%s) [%d]: %s", opengl_source(source), opengl_type(type), opengl_severity(severity), id, message);
}
#endif

static GLuint opengl_load_shader(const char* vsrc, const char* fsrc)
{
	// Compile vertex shader
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vsrc, NULL);
	glCompileShader(vshader);

	i32 success = 0;
	char log[1024];
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, 1024, NULL, log);
		LOGE("Failed to compile vertex shader: %s", log);
	}

	// Compile fragment shader
	GLuint fshader(glCreateShader(GL_FRAGMENT_SHADER));
	glShaderSource(fshader, 1, &fsrc, NULL);
	glCompileShader(fshader);

	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, 1024, NULL, log);
		LOGE("Failed to compile fragment shader: %s", log);
	}

	// Link vertex and fragment shader together
	GLuint program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 1024, NULL, log);
		LOGE("Failed to link shader program: %s", log);
	}

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 1024, NULL, log);
		LOGE("Shader validation error: %s", log);
	}

	// Delete shaders objects
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	g_app.shaders.emplace_back(program);
	return program;
}

static bool opengl_initialize(const AppConfig& config)
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOGE("Failed to initialize GLAD!");
		return false;
	}

#if _DEBUG
	i32 flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		// Initialize debug output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(opengl_debug_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else
	{
		LOGW("Failed to initialize OpenGL debug output.");
	}
#endif

	glGenVertexArrays(1, &g_app.vao);
	glGenBuffers(1, &g_app.vbo);

	glBindVertexArray(g_app.vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_app.vbo);

	const GLsizei stride = 3 * sizeof(f32) + sizeof(u32);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (GLvoid*)(3 * sizeof(f32)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

static void opengl_shutdown()
{
	glDeleteProgram(g_app.shader);
	glDeleteBuffers(1, &g_app.vbo);
	glDeleteVertexArrays(1, &g_app.vao);
}

static bool imgui_initialize(const AppConfig& config)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	if (!ImGui_ImplGlfw_InitForOpenGL(g_app.window, true))
	{
		LOGE("Failed to initialize ImGui GLFW!");
		return false;
	}

	if (!ImGui_ImplOpenGL3_Init("#version 330 core\n"))
	{
		LOGE("Failed to initialize ImGui OpenGL!");
		return false;
	}

	ImGui::StyleColorsDark();
#ifdef _DEBUG
	io.Fonts->AddFontFromFileTTF(PROJECT_PATH"Assets/Common/Fonts/UbuntuMono-Regular.ttf", 20);
#else
	io.Fonts->AddFontFromFileTTF("Assets/Common/Fonts/UbuntuMono-Regular.ttf", 20);
#endif

	io.IniFilename = "imgui.ini";
	return true;
}

static void imgui_shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

static size_t wren_class_hash(const char* moduleName, const char* className)
{
	static std::hash<std::string> hash;
	return hash(moduleName) ^ hash(className);
}

static size_t wren_method_hash(const char* moduleName, const char* className, bool isStatic, const char* signature)
{
	static std::hash<std::string> hash;
	return hash(moduleName) ^ hash(className) ^ hash(isStatic ? "s" : "") ^ hash(signature);
}

static WrenForeignMethodFn wren_bind_method(WrenVM* vm, const char* moduleName, const char* className, bool isStatic, const char* signature)
{
	if (std::strcmp(moduleName, "random") == 0)
		return nullptr;
	if (std::strcmp(moduleName, "meta") == 0)
		return nullptr;

	const size_t hash = wren_method_hash(moduleName, className, isStatic, signature);
	const auto it = g_app.methods.find(hash);
	if (it != g_app.methods.end())
		return it->second;
	return nullptr;
}

static WrenForeignMethodFn wren_allocate(const size_t classHash)
{
	const auto it = g_app.classes.find(classHash);
	if (it != g_app.classes.end())
		return it->second.allocate;
	return nullptr;
}

static WrenFinalizerFn wren_finalizer(const size_t classHash)
{
	const auto it = g_app.classes.find(classHash);
	if (it != g_app.classes.end())
		return it->second.finalize;
	return nullptr;
}

static WrenForeignClassMethods wren_bind_class(WrenVM* vm, const char* moduleName, const char* className)
{
	WrenForeignClassMethods methods{};
	methods.allocate = nullptr;
	methods.finalize = nullptr;

	if (std::strcmp(moduleName, "random") == 0) return methods;
	if (std::strcmp(moduleName, "meta") == 0) return methods;

	const size_t hash = wren_class_hash(moduleName, className);
	methods.allocate = wren_allocate(hash);
	methods.finalize = wren_finalizer(hash);

	return methods;
}

static WrenLoadModuleResult wren_load_module(WrenVM* vm, const char* name)
{
	WrenLoadModuleResult res{};
	return res;
}

static void wren_write(WrenVM* vm, const char* text)
{
	if (strcmp(text, "\n") == 0)
		return;

	App::Log(false, "", 0, "", 0xFFFFFFFF, "%s", text);
}

static void wren_error(WrenVM* vm, WrenErrorType errorType, const char* module, const int line, const char* msg)
{
	switch (errorType)
	{
	case WREN_ERROR_COMPILE:
		LOGE("Wren Error Compile [%s line %d] %s", module, line, msg);
		break;
	case WREN_ERROR_STACK_TRACE:
		LOGE("Wren Error Stack Trace [%s line %d] %s", module, line, msg);
		break;
	case WREN_ERROR_RUNTIME:
		LOGE("Wren Error Runtime [%s line %d] %s", module, line, msg);
		break;
	}

	g_app.error = true;
}

static bool wren_initialize()
{
	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = wren_write;
	config.errorFn = wren_error;
	config.bindForeignMethodFn = wren_bind_method;
	config.bindForeignClassFn = wren_bind_class;
	config.loadModuleFn = wren_load_module;
	config.initialHeapSize = 1024LL * 1024 * 32;
	config.minHeapSize = 1024LL * 1024 * 16;
	config.heapGrowthPercent = 80;
	g_app.vm = wrenNewVM(&config);

	return true;
}

static void wren_shutdown()
{
	if (g_app.mainClass) wrenReleaseHandle(g_app.vm, g_app.mainClass);
	if (g_app.initMethod) wrenReleaseHandle(g_app.vm, g_app.initMethod);
	if (g_app.updateMethod) wrenReleaseHandle(g_app.vm, g_app.updateMethod);
	if (g_app.vm) wrenFreeVM(g_app.vm);

	g_app.vm = nullptr;
	g_app.mainClass = nullptr;
	g_app.initMethod = nullptr;
	g_app.updateMethod = nullptr;
}

// App
void App::SetFrameOp(FrameOp op)
{
	g_app.frameOp = op;
}

// Util
void App::Log(bool verbose, const char* file, i32 line, const char* func, u32 color, const char* format, ...)
{
	static std::string buffer; // Persistent buffer to reduce allocations

	// Initialize the argument list
	va_list args;
	va_start(args, format);

	// Determine the size of the formatted string
	int size = std::vsnprintf(nullptr, 0, format, args);
	if (size < 0)
	{
		va_end(args);
		throw std::runtime_error("Error during formatting.");
	}

	buffer.resize(size + 1); // +1 for null terminator

	// Format the message directly into the buffer
	std::vsnprintf(&buffer[0], buffer.size(), format, args);
	va_end(args);

	// Create log entry
	LogData log{};
	log.color = color;

	if (verbose)
	{
		const char* filename = std::max(strrchr(file, '/'), strrchr(file, '\\'));
		filename = filename ? filename + 1 : file; // Move past '/' or '\' if found

		// Format metadata in the same buffer
		size = std::snprintf(nullptr, 0, "[%s:%d (%s)]\n%s", filename, line, func, buffer.c_str());

		std::string temp;
		temp.resize(size + 1);

		std::snprintf(&temp[0], temp.size(), "[%s:%d (%s)]\n%s", filename, line, func, buffer.c_str());

		log.message = std::move(temp);
	}
	else
	{
		log.message = buffer;
	}

#if _DEBUG
	std::cout << log.message << std::endl;
#endif

	// Store log message
	g_app.logs.emplace_back(std::move(log));

	// Limit log size
	if (g_app.logs.size() > 1024)
		g_app.logs.erase(g_app.logs.begin());
}

// Window
void App::WinSetMode(WindowMode windowMode)
{
	if (windowMode == g_app.winMode)
		return;

	GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);

	if (windowMode == WindowMode::FULLSCREEN)
	{
		glfwSetWindowMonitor(g_app.window, pMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else if (windowMode == WindowMode::BORDERLESS)
	{
		glfwSetWindowAttrib(g_app.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(g_app.window, 0, 0);
		glfwSetWindowSize(g_app.window, mode->width, mode->height);
	}
	else
	{
		glfwSetWindowMonitor(g_app.window, nullptr, g_app.winX, g_app.winY, g_app.winWidth, g_app.winHeight, 0);
		glfwSetWindowAttrib(g_app.window, GLFW_DECORATED, GLFW_TRUE);
	}

	g_app.winMode = windowMode;
}

i32 App::WinWidth()
{
	i32 w, h;
	glfwGetWindowSize(g_app.window, &w, &h);
	return w;
}

i32 App::WinHeight()
{
	i32 w, h;
	glfwGetWindowSize(g_app.window, &w, &h);
	return h;
}

f64 App::WinMouseX()
{
	f64 x, y;
	glfwGetCursorPos(g_app.window, &x, &y);
	return x;
}

f64 App::WinMouseY()
{
	f64 x, y;
	glfwGetCursorPos(g_app.window, &x, &y);
	return y;
}

bool App::WinButton(i32 b)
{
	return glfwGetMouseButton(g_app.window, b) == GLFW_PRESS;
}

bool App::WinKey(i32 k)
{
	GLFW_KEY_W;
	return glfwGetKey(g_app.window, k) == GLFW_PRESS;
}

i32 App::WinPadCount()
{
	i32 count = 0;
	for (i32 i = 0; i < 16; ++i)
	{
		if (glfwJoystickPresent(GLFW_JOYSTICK_1 + i))
			++count;
	}
	return count;
}

bool App::WinPadButton(i32 i, i32 b)
{
	GLFWgamepadstate state;
	return glfwJoystickPresent(GLFW_JOYSTICK_1 + i) &&
		glfwGetGamepadState(GLFW_JOYSTICK_1 + i, &state) &&
		state.buttons[b] == GLFW_PRESS;
}

f32 App::WinPadAxis(i32 i, i32 a)
{
	GLFWgamepadstate state;
	return glfwJoystickPresent(GLFW_JOYSTICK_1 + i) &&
		glfwGetGamepadState(GLFW_JOYSTICK_1 + i, &state) ? state.axes[a] : 0.0f;
}

void App::WinClose()
{
	glfwSetWindowShouldClose(g_app.window, GLFW_TRUE);
}

// Graphics
static std::string shader_process_includes(const std::string& source)
{
	std::stringstream processed;
	std::istringstream input(source);
	std::string line;
	static std::string includePath;

	while (std::getline(input, line))
	{
		// Check for #include "filename"
		if (line.rfind("#include", 0) == 0)
		{
			size_t start = line.find('"');
			size_t end = line.rfind('"');
			if (start != std::string::npos && end != std::string::npos && start < end)
			{
				size_t len = end - start - 1;

				// Ensure buffer is large enough
				includePath.resize(len);
				std::memcpy(&includePath[0], line.c_str() + start + 1, len);
				includePath[len] = '\0'; // Null-terminate manually

				std::string includedSource = file_load(includePath.c_str());
				processed << shader_process_includes(includedSource) << "\n";
				continue; // Skip writing the #include line itself
			}
		}
		processed << line << "\n";
	}
	return processed.str();
}

u32 App::GlCreateShader(const char* path)
{
	auto src = shader_process_includes(file_load(path));
	auto vsrc = "#version 330 core\n#define VERT\n" + src;
	auto fsrc = "#version 330 core\n#define FRAG\n" + src;
	return opengl_load_shader(vsrc.c_str(), fsrc.c_str());
}

void App::GlDestroyShader(u32 shader)
{
	glDeleteProgram(shader);
}

void App::GlSetShader(u32 shader)
{
	g_app.shader = shader;
	glUseProgram(g_app.shader);
}

void App::GlBegin(bool alpha, bool ztest, f32 pointSize, f32 lineWidth)
{
	if (alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	if (ztest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glPointSize(pointSize);
	glLineWidth(lineWidth);

	g_app.vertices.clear();
}

void App::GlEnd(u32 mode)
{
	glBindVertexArray(g_app.vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_app.vbo);

	glBufferData(GL_ARRAY_BUFFER, g_app.vertices.size() * sizeof(Vertex), g_app.vertices.data(), GL_DYNAMIC_DRAW);
	
	for (u32 bit = 1; bit <= (u32)GlTopology::TRIANGLE_FAN; bit <<= 1)
	{
		if (mode & bit)
		{
			u32 i = 0, b = bit;
			while (b > 1 && ++i) b >>= 1;
			glDrawArrays(GL_POINTS + i, 0, (GLsizei)g_app.vertices.size());
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void App::GlViewport(u32 x, u32 y, u32 w, u32 h)
{
	glViewport(x, y, w, h);
}

void App::GlClear(f32 r, f32 g, f32 b, f32 a, f32 d, f32 s, u32 flags)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void App::GlUniform(const char* name)
{
	g_app.uniformName = name;
}

void App::GlFloat(f32 x)
{
	glUniform1f(glGetUniformLocation(g_app.shader, g_app.uniformName), x);
}

void App::GlVec2F(f32 x, f32 y)
{
	glUniform2f(glGetUniformLocation(g_app.shader, g_app.uniformName), x, y);
}

void App::GlVec3F(f32 x, f32 y, f32 z)
{
	glUniform3f(glGetUniformLocation(g_app.shader, g_app.uniformName), x, y, z);
}

void App::GlVec4F(f32 x, f32 y, f32 z, f32 w)
{
	glUniform4f(glGetUniformLocation(g_app.shader, g_app.uniformName), x, y, z, w);
}

void App::GlMat2x2F(
	f32 m00, f32 m01,
	f32 m10, f32 m11)
{
	const GLfloat v[4] = { m00, m01, m10, m11 };
	glUniformMatrix2fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlMat2x3F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12)
{
	const GLfloat v[6] = { m00, m01, m02, m10, m11, m12 };
	glUniformMatrix2x3fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlMat2x4F(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13)
{
	const GLfloat v[8] = { m00, m01, m02, m03, m10, m11, m12, m13 };
	glUniformMatrix2x4fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlMat3x2F(
	f32 m00, f32 m01,
	f32 m10, f32 m11,
	f32 m20, f32 m21)
{
	const GLfloat v[6] = { m00, m01, m10, m11, m20, m21 };
	glUniformMatrix3x2fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlMat3x3F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12,
	f32 m20, f32 m21, f32 m22)
{
	const GLfloat v[9] = { m00, m01, m02, m10, m11, m12, m20, m21, m22 };
	glUniformMatrix3fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlMat3x4F(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23)
{
	const GLfloat v[12] = { m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23 };
	glUniformMatrix3x4fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlMat4x2F(
	f32 m00, f32 m01,
	f32 m10, f32 m11,
	f32 m20, f32 m21,
	f32 m30, f32 m31)
{
	const GLfloat v[8] = { m00, m01, m10, m11, m20, m21 };
	glUniformMatrix4x2fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlMat4x3F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12,
	f32 m20, f32 m21, f32 m22,
	f32 m30, f32 m31, f32 m32)
{
	const GLfloat v[12] = { m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32 };
	glUniformMatrix4x3fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlMat4x4F(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33)
{
	const GLfloat v[16] = { m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 };
	glUniformMatrix4fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::GlVertex(f32 x, f32 y, f32 z, u32 c)
{
	g_app.vertices.emplace_back(Vertex{ { x, y, z }, c });
}

// Gui
bool App::GuiBool(const char* label, bool v)
{
	ImGui::Checkbox(label, &v);
	return v;
}

i32 App::GuiInt(const char* label, i32 i)
{
	ImGui::InputInt(label, &i);
	return i;
}

i32 App::GuiInt(const char* label, i32 i, i32 min, i32 max)
{
	ImGui::SliderInt(label, &i, min, max);
	return i;
}

f32 App::GuiFloat(const char* label, f32 v)
{
	ImGui::DragFloat(label, &v, 0.1f);
	return v;
}

void App::GuiSeparator(const char* label)
{
	ImGui::SeparatorText(label);
}

bool App::GuiButton(const char* label)
{
	return ImGui::Button(label);
}

void App::GuiSameLine()
{
	ImGui::SameLine();
}

bool App::GuiBeginChild(const char* label, f32 px, f32 py)
{
	ImVec2 available = ImGui::GetContentRegionAvail();
	ImVec2 size = ImVec2(
		px == -1 ? available.x : px * available.x,
		py == -1 ? available.y : py * available.y
	);

	return ImGui::BeginChild(label, size);
}

void App::GuiEndChild()
{
	ImGui::EndChild();
}

// Script
void App::WrenParseFile(const char* moduleName, const char* filepath)
{
	auto src = file_load(filepath);
	WrenParseSource(moduleName, src.c_str());
}

void App::WrenParseSource(const char* moduleName, const char* source)
{
	switch (wrenInterpret(g_app.vm, moduleName, source))
	{
	case WREN_RESULT_COMPILE_ERROR:
		g_app.error = true;
		LOGE("Wren compilation error on module: %s", moduleName);
		break;

	case WREN_RESULT_RUNTIME_ERROR:
		g_app.error = true;
		LOGE("Wren runtime error on module: %s", moduleName);
		break;

	case WREN_RESULT_SUCCESS:
		g_app.error = false;
		LOGI("Wren successfully compiled module: %s", moduleName);
		break;
	}
}

void App::WrenBindClass(const char* moduleName, const char* className, ScriptClass scriptClass)
{
	const size_t hash = wren_class_hash(moduleName, className);
	g_app.classes.insert(std::make_pair(hash, scriptClass));
}

void App::WrenBindMethod(const char* moduleName, const char* className, bool isStatic, const char* signature, ScriptMethodFn scriptMethod)
{
	const size_t hash = wren_method_hash(moduleName, className, isStatic, signature);
	g_app.methods.insert(std::make_pair(hash, scriptMethod));
}

void App::WrenEnsureSlots(ScriptVM* vm, i32 count)
{
	wrenEnsureSlots(vm, count);
}

void App::WrenGetVariable(ScriptVM* vm, const char* moduleName, const char* className, i32 slot)
{
	wrenGetVariable(vm, moduleName, className, slot);
}

bool App::WrenGetSlotBool(ScriptVM* vm, i32 slot)
{
	return wrenGetSlotBool(vm, slot);
}

u32 App::WrenGetSlotUInt(ScriptVM* vm, i32 slot)
{
	return (u32)wrenGetSlotDouble(vm, slot);
}

i32 App::WrenGetSlotInt(ScriptVM* vm, i32 slot)
{
	return (i32)wrenGetSlotDouble(vm, slot);
}

f32 App::WrenGetSlotFloat(ScriptVM* vm, i32 slot)
{
	return (f32)wrenGetSlotDouble(vm, slot);
}

f64 App::WrenGetSlotDouble(ScriptVM* vm, i32 slot)
{
	return wrenGetSlotDouble(vm, slot);
}

const char* App::WrenGetSlotString(ScriptVM* vm, i32 slot)
{
	return wrenGetSlotString(vm, slot);
}

void* App::WrenGetSlotObject(ScriptVM* vm, i32 slot)
{
	return wrenGetSlotForeign(vm, slot);
}

const char* App::WrenGetSlotBytes(ScriptVM* vm, i32 slot, i32* length)
{
	return wrenGetSlotBytes(vm, slot, length);
}

void App::WrenSetSlotBool(ScriptVM* vm, i32 slot, bool value)
{
	wrenSetSlotBool(vm, slot, value);
}

void App::WrenSetSlotUInt(ScriptVM* vm, i32 slot, u32 value)
{
	wrenSetSlotDouble(vm, slot, (f64)value);
}

void App::WrenSetSlotInt(ScriptVM* vm, i32 slot, i32 value)
{
	wrenSetSlotDouble(vm, slot, (f64)value);
}

void App::WrenSetSlotFloat(ScriptVM* vm, i32 slot, f32 value)
{
	wrenSetSlotDouble(vm, slot, (f64)value);
}

void App::WrenSetSlotDouble(ScriptVM* vm, i32 slot, f64 value)
{
	wrenSetSlotDouble(vm, slot, (f64)value);
}

void* App::WrenSetSlotNewObject(ScriptVM* vm, i32 slot, i32 classSlot, size_t size)
{
	return wrenSetSlotNewForeign(vm, slot, classSlot, size);
}

bool App::Initialize(const AppConfig& config)
{
	if (config.pathCount == 0)
		return false;

	if (!glfw_initialize(config))
		return false;

	if (!opengl_initialize(config))
		return false;

	if (!imgui_initialize(config))
		return false;

	g_app.current_path = 0;
	g_app.frameOp = FrameOp::RELOAD;

	return true;
}

void App::Shutdown()
{
	wren_shutdown();
	imgui_shutdown();
	opengl_shutdown();
	glfw_shutdown();
}

void App::Prev(const AppConfig& config)
{
	g_app.current_path--;
	g_app.current_path = g_app.current_path < 0 ? 0 : g_app.current_path >= config.pathCount ? config.pathCount : g_app.current_path;
	Reload(config);
}

void App::Next(const AppConfig& config)
{
	g_app.current_path++;
	g_app.current_path = g_app.current_path < 0 ? 0 : g_app.current_path >= config.pathCount ? config.pathCount : g_app.current_path;
	Reload(config);
}

void App::Reload(const AppConfig& config)
{
	// Clear resources
	for (const auto shader : g_app.shaders)
		GlDestroyShader(shader);
	g_app.shaders.clear();

	// Reload wren vm
	if (g_app.vm != nullptr)
		wren_shutdown();
	wren_initialize();

	// Window
	WrenBindMethod("app", "App", true, "winWidth",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 0);
			WrenSetSlotInt(vm, 0, WinWidth());
		});

	WrenBindMethod("app", "App", true, "winHeight",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 0);
			WrenSetSlotInt(vm, 0, WinHeight());
		});

	WrenBindMethod("app", "App", true, "winMouseX",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 0);
			WrenSetSlotDouble(vm, 0, WinMouseX());
		});

	WrenBindMethod("app", "App", true, "winMouseY",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 0);
			WrenSetSlotDouble(vm, 0, WinMouseY());
		});

	WrenBindMethod("app", "App", true, "winButton(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotBool(vm, 0, WinButton(WrenGetSlotInt(vm, 1)));
		});

	WrenBindMethod("app", "App", true, "winKey(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotBool(vm, 0, WinKey(WrenGetSlotInt(vm, 1)));
		});

	WrenBindMethod("app", "App", true, "winPadCount()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotInt(vm, 0, WinPadCount());
		});

	WrenBindMethod("app", "App", true, "winPadButton(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotBool(vm, 0, WinPadButton(WrenGetSlotInt(vm, 1), WrenGetSlotInt(vm, 2)));
		});

	WrenBindMethod("app", "App", true, "winPadAxis(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotFloat(vm, 0, WinPadAxis(WrenGetSlotInt(vm, 1), WrenGetSlotInt(vm, 2)));
		});

	WrenBindMethod("app", "App", true, "winClose()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 0);
			WinClose();
		});

	// Graphics
#define SCRIPT_ARGS_ARR(s, n, p, t) p v[n]; for (u8 i = s; i < n; ++i) v[i] = WrenGetSlot##t##(vm, i + 1);

	WrenBindMethod("app", "App", true, "glCreateShader(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto shader = GlCreateShader(WrenGetSlotString(vm, 1));
			WrenSetSlotUInt(vm, 0, shader);
		});

	WrenBindMethod("app", "App", true, "glDestroyShader(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto shader = WrenGetSlotUInt(vm, 1);
			GlDestroyShader(shader);
		});

	WrenBindMethod("app", "App", true, "glSetShader(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto shader = WrenGetSlotUInt(vm, 1);
			GlSetShader(shader);
		});

	WrenBindMethod("app", "App", true, "glBegin(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 3);
			GlBegin(WrenGetSlotBool(vm, 1), WrenGetSlotBool(vm, 2), WrenGetSlotFloat(vm, 3), WrenGetSlotFloat(vm, 4));
		});

	WrenBindMethod("app", "App", true, "glEnd(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GlEnd(WrenGetSlotUInt(vm, 1));
		});

	WrenBindMethod("app", "App", true, "glViewport(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 4, u32, UInt);
			GlViewport(v[0], v[1], v[2], v[3]);
		});

	WrenBindMethod("app", "App", true, "glClear(_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 7);
			SCRIPT_ARGS_ARR(0, 6, f32, Float);
			GlClear(v[0], v[1], v[2], v[3], v[4], v[5], WrenGetSlotUInt(vm, 7));
		});

	WrenBindMethod("app", "App", true, "glUniform(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			const char* value = WrenGetSlotString(vm, 1);
			GlUniform(value);
		});

	WrenBindMethod("app", "App", true, "glFloat(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			f32 value = WrenGetSlotFloat(vm, 1);
			GlFloat(value);
		});

	WrenBindMethod("app", "App", true, "glVec2f(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			SCRIPT_ARGS_ARR(0, 2, f32, Float);
			GlVec2F(v[0], v[1]);
		});

	WrenBindMethod("app", "App", true, "glVec3f(_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 3);
			SCRIPT_ARGS_ARR(0, 3, f32, Float);
			GlVec3F(v[0], v[1], v[2]);
		});

	WrenBindMethod("app", "App", true, "glVec4f(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			SCRIPT_ARGS_ARR(0, 4, f32, Float);
			GlVec4F(v[0], v[1], v[2], v[3]);
		});

	WrenBindMethod("app", "App", true, "glMat2x2f(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			SCRIPT_ARGS_ARR(0, 4, f32, Float);
			GlMat2x2F(v[0], v[1], v[2], v[3]);
		});

	WrenBindMethod("app", "App", true, "glMat2x3f(_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 6);
			SCRIPT_ARGS_ARR(0, 6, f32, Float);
			GlMat2x3F(v[0], v[1], v[2], v[3], v[4], v[5]);
		});

	WrenBindMethod("app", "App", true, "glMat2x4f(_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 8);
			SCRIPT_ARGS_ARR(0, 8, f32, Float);
			GlMat2x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
		});

	WrenBindMethod("app", "App", true, "glMat3x2f(_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 6);
			SCRIPT_ARGS_ARR(0, 6, f32, Float);
			GlMat3x2F(v[0], v[1], v[2], v[3], v[4], v[5]);
		});

	WrenBindMethod("app", "App", true, "glMat3x3f(_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 9);
			SCRIPT_ARGS_ARR(0, 9, f32, Float);
			GlMat3x3F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
		});

	WrenBindMethod("app", "App", true, "glMat3x4f(_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 12);
			SCRIPT_ARGS_ARR(0, 12, f32, Float);
			GlMat3x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11]);
		});

	WrenBindMethod("app", "App", true, "glMat4x2f(_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 8);
			SCRIPT_ARGS_ARR(0, 8, f32, Float);
			GlMat4x2F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
		});

	WrenBindMethod("app", "App", true, "glMat4x3f(_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 12);
			SCRIPT_ARGS_ARR(0, 12, f32, Float);
			GlMat4x3F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11]);
		});

	WrenBindMethod("app", "App", true, "glMat4x4f(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 16);
			SCRIPT_ARGS_ARR(0, 16, f32, Float);
			GlMat4x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
		});

	WrenBindMethod("app", "App", true, "glVertex(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 3, f32, Float);
			GlVertex(v[0], v[1], v[2], WrenGetSlotUInt(vm, 4));
		});

	// Gui
	WrenBindMethod("app", "App", true, "guiBool(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotBool(vm, 0, GuiBool(WrenGetSlotString(vm, 1), WrenGetSlotBool(vm, 2)));
		});

	WrenBindMethod("app", "App", true, "guiInt(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotInt(vm, 0, GuiInt(WrenGetSlotString(vm, 1), WrenGetSlotInt(vm, 2)));
		});

	WrenBindMethod("app", "App", true, "guiInt(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			WrenSetSlotInt(vm, 0, GuiInt(WrenGetSlotString(vm, 1), WrenGetSlotInt(vm, 2), WrenGetSlotInt(vm, 3), WrenGetSlotInt(vm, 4)));
		});

	WrenBindMethod("app", "App", true, "guiFloat(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotFloat(vm, 0, GuiFloat(WrenGetSlotString(vm, 1), WrenGetSlotFloat(vm, 2)));
		});

	WrenBindMethod("app", "App", true, "guiSeparator(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiSeparator(WrenGetSlotString(vm, 1));
		});

	WrenBindMethod("app", "App", true, "guiButton(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotBool(vm, 0, GuiButton(WrenGetSlotString(vm, 1)));
		});

	WrenBindMethod("app", "App", true, "guiSameLine()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiSameLine();
		});

	WrenBindMethod("app", "App", true, "guiBeginChild(_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 3);
			WrenSetSlotBool(vm, 0, GuiBeginChild(WrenGetSlotString(vm, 1), WrenGetSlotFloat(vm, 2), WrenGetSlotFloat(vm, 3)));
		});

	WrenBindMethod("app", "App", true, "guiEndChild()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiEndChild();
		});

	WrenParseFile("app", "Assets/Common/Scripts/app.wren");

	config.bindApiFn();

	// Main callbacks
	WrenParseFile("main", config.paths[g_app.current_path]);
	if (!g_app.error)
	{
		wrenEnsureSlots(g_app.vm, 1);
		wrenGetVariable(g_app.vm, "main", "Main", 0);
		g_app.mainClass = wrenGetSlotHandle(g_app.vm, 0);
		wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
		g_app.initMethod = wrenMakeCallHandle(g_app.vm, "init()");
		g_app.updateMethod = wrenMakeCallHandle(g_app.vm, "update(_)");
		g_app.renderMethod = wrenMakeCallHandle(g_app.vm, "render()");

		wrenEnsureSlots(g_app.vm, 1);
		wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
		wrenCall(g_app.vm, g_app.initMethod);
	}
	else
	{
		wren_shutdown();
	}
}

void App::Update(f64 dt)
{
	glfwPollEvents();

	g_app.frames++;
	g_app.time += dt;
	if (g_app.time >= 1.f)
	{
		g_app.fps = g_app.frames / g_app.time;
		g_app.spf = g_app.time / g_app.frames;
		g_app.frames = 1;
		g_app.time = std::fmodf(g_app.time, 1.f);
	}

	if (!g_app.error && !g_app.paused)
	{
		wrenEnsureSlots(g_app.vm, 2);
		wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
		wrenSetSlotDouble(g_app.vm, 1, dt);
		wrenCall(g_app.vm, g_app.updateMethod);
	}
}

void App::Render()
{
	glViewport(0, 0, WinWidth(), WinHeight());
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("App"))
		{
			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Console"))
					g_app.showConsole = !g_app.showConsole;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::BeginMenu("Window"))
				{
					if (ImGui::BeginMenu("Mode"))
					{
						if (ImGui::MenuItem("Windowed"))
							WinSetMode(WindowMode::WINDOW);
						if (ImGui::MenuItem("Borderless"))
							WinSetMode(WindowMode::BORDERLESS);
						if (ImGui::MenuItem("Fullscreen"))
							WinSetMode(WindowMode::FULLSCREEN);

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}
				
				if (ImGui::BeginMenu("Font"))
				{
					ImGui::SliderFloat("Size", &g_app.fontSize, 0.5f, 2.0f);

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				WinClose();

			ImGui::EndMenu();
		}

		if (ImGui::Button("Reload"))
			g_app.frameOp = FrameOp::RELOAD;

		if (ImGui::Button(g_app.paused ? "Play" : "Pause"))
			g_app.paused = !g_app.paused;

		ImGui::Text("%.0f fps | %.2f ms", g_app.fps, g_app.spf * 1000);

		ImGui::EndMainMenuBar();
	}

	if (g_app.showSettings)
	{
		ImGui::Begin("Settings", &g_app.showSettings);

		// Script loader
		//ImGui::InputText("Path", &g_app.filepath);
		//
		//ImGui::SameLine();
		//
		//if (ImGui::Button("Load"))
		//	g_app.script = file_load(g_app.filepath.c_str());
		//
		//ImGui::SameLine();
		//
		//if (ImGui::Button("Save"))
		//	file_save(g_app.filepath.c_str(), g_app.script);
	}

	if (g_app.showConsole)
	{
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.05f, ImGui::GetFrameHeight() + (ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()) * 0.25f), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.9f, (ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()) * 0.5f), ImGuiCond_Appearing);

		ImGui::Begin("Console", &g_app.showConsole, ImGuiWindowFlags_NoSavedSettings);

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
		{
			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::Selectable("Clear"))
					g_app.logs.clear();
				
				if (ImGui::Selectable("Copy"))
				{
					static std::string clipboard;
					clipboard.clear();
					clipboard.reserve(4096);

					for (const auto& log : g_app.logs)
					{
						std::size_t length = log.message.size();
						if (length > 0 && log.message[length - 1] == '\0') --length;
						clipboard.append(log.message, 0, length).append("\n");
					}

					ImGui::SetClipboardText(clipboard.c_str());
				}

				ImGui::EndPopup();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
			for (const auto& log : g_app.logs)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, log.color);
				ImGui::TextUnformatted(log.message.c_str());
				ImGui::PopStyleColor();
			}

			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::PopStyleVar();
		}
		ImGui::EndChild();

		ImGui::End();
	}

	ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

	ImGui::Begin("##Overlay", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

	ImGui::SetWindowFontScale(g_app.fontSize);
	if (!g_app.error && !g_app.paused)
	{
		wrenEnsureSlots(g_app.vm, 1);
		wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
		wrenCall(g_app.vm, g_app.renderMethod);
	}
	ImGui::SetWindowFontScale(1.0f);

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(g_app.window);
}

int App::Run(AppConfigureFn configFn)
{
	auto config = configFn();
	if (!Initialize(config))
	{
		return EXIT_FAILURE;
	}

	f64 lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(g_app.window))
	{
		switch (g_app.frameOp)
		{
		case FrameOp::RELOAD: Reload(config); break;
		case FrameOp::NEXT: Next(config); break;
		case FrameOp::PREV: Prev(config); break;
		}
		g_app.frameOp = FrameOp::NONE;

		f64 currentTime = glfwGetTime();
		f64 deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Update(deltaTime);
		Render();
	}

	Shutdown();
	return EXIT_SUCCESS;
}