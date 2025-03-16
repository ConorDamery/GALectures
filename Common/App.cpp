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
	f32 fontSize{ 1.5f };
	bool showSettings{ false };
	bool showConsole{ false };
};
static AppData g_app;

// Static utils
static std::string file_load(const char* filepath)
{
	std::string path = PROJECT_PATH;
	path += filepath;

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
	std::ofstream file(filepath);
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
	io.Fonts->AddFontFromFileTTF(PROJECT_PATH"/Common/Fonts/UbuntuMono-Regular.ttf", 20);

	io.IniFilename = PROJECT_PATH"/imgui.ini";
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

	// Store log message
	g_app.logs.emplace_back(std::move(log));

	// Limit log size
	if (g_app.logs.size() > 1024)
		g_app.logs.erase(g_app.logs.begin());
}

bool App::DebugBool(const char* label, bool v)
{
	ImGui::Checkbox(label, &v);
	return v;
}

i32 App::DebugInt(const char* label, i32 i)
{
	ImGui::InputInt(label, &i);
	return i;
}

i32 App::DebugInt(const char* label, i32 i, i32 min, i32 max)
{
	ImGui::SliderInt(label, &i, min, max);
	return i;
}

f32 App::DebugFloat(const char* label, f32 v)
{
	ImGui::DragFloat(label, &v, 0.1f);
	return v;
}

void App::DebugSeparator(const char* label)
{
	ImGui::SeparatorText(label);
}

bool App::DebugButton(const char* label)
{
	return ImGui::Button(label);
}

f32 App::GetFps()
{
	return g_app.fps;
}

f32 App::GetAvgFrameTime()
{
	return g_app.spf;
}

// Window
void App::SetWindowMode(WindowMode windowMode)
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

i32 App::GetWidth()
{
	i32 w, h;
	glfwGetWindowSize(g_app.window, &w, &h);
	return w;
}

i32 App::GetHeight()
{
	i32 w, h;
	glfwGetWindowSize(g_app.window, &w, &h);
	return h;
}

f64 App::GetMouseX()
{
	f64 x, y;
	glfwGetCursorPos(g_app.window, &x, &y);
	return x;
}

f64 App::GetMouseY()
{
	f64 x, y;
	glfwGetCursorPos(g_app.window, &x, &y);
	return y;
}

bool App::GetButton(i32 b)
{
	return glfwGetMouseButton(g_app.window, b) == GLFW_PRESS;
}

bool App::GetKey(i32 k)
{
	return glfwGetKey(g_app.window, k) == GLFW_PRESS;
}

void App::Close()
{
	glfwSetWindowShouldClose(g_app.window, GLFW_TRUE);
}

// Graphics
u32 App::CreateShader(const char* path)
{
	auto src = file_load(path);
	auto vsrc = "#version 330 core\n#define VERT\n" + src;
	auto fsrc = "#version 330 core\n#define FRAG\n" + src;
	return opengl_load_shader(vsrc.c_str(), fsrc.c_str());
}

void App::DestroyShader(u32 shader)
{
	glDeleteProgram(shader);
}

void App::SetShader(u32 shader)
{
	g_app.shader = shader;
	glUseProgram(g_app.shader);
}

void App::BeginDraw(bool alpha, bool ztest, f32 pointSize, f32 lineWidth)
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
}

void App::EndDraw(u32 mode)
{
	glBindVertexArray(g_app.vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_app.vbo);

	GLenum glMode = GL_POINTS;
	switch (mode)
	{
	case 0: glMode = GL_POINTS; break;
	case 1: glMode = GL_LINES; break;
	case 2: glMode = GL_TRIANGLES; break;
	}

	glBufferData(GL_ARRAY_BUFFER, g_app.vertices.size() * sizeof(Vertex), g_app.vertices.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(glMode, 0, (GLsizei)g_app.vertices.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	g_app.vertices.clear();
}

void App::SetViewport(u32 x, u32 y, u32 w, u32 h)
{
	glViewport(x, y, w, h);
}

void App::ClearScreen(f32 r, f32 g, f32 b, f32 a, f32 d, f32 s, u32 flags)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void App::SetUniform(const char* name)
{
	g_app.uniformName = name;
}

void App::SetFloat(f32 x)
{
	glUniform1f(glGetUniformLocation(g_app.shader, g_app.uniformName), x);
}

void App::SetVec2F(f32 x, f32 y)
{
	glUniform2f(glGetUniformLocation(g_app.shader, g_app.uniformName), x, y);
}

void App::SetVec3F(f32 x, f32 y, f32 z)
{
	glUniform3f(glGetUniformLocation(g_app.shader, g_app.uniformName), x, y, z);
}

void App::SetVec4F(f32 x, f32 y, f32 z, f32 w)
{
	glUniform4f(glGetUniformLocation(g_app.shader, g_app.uniformName), x, y, z, w);
}

void App::SetMat2x2F(
	f32 m00, f32 m01,
	f32 m10, f32 m11)
{
	const GLfloat v[4] = { m00, m01, m10, m11 };
	glUniformMatrix2fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::SetMat3x2F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12)
{
	const GLfloat v[6] = { m00, m01, m02, m10, m11, m12 };
	glUniformMatrix3x2fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::SetMat2x3F(
	f32 m00, f32 m01,
	f32 m10, f32 m11,
	f32 m20, f32 m21)
{
	const GLfloat v[6] = { m00, m01, m10, m11, m20, m21 };
	glUniformMatrix2x3fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::SetMat3x3F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12,
	f32 m20, f32 m21, f32 m22)
{
	const GLfloat v[9] = { m00, m01, m02, m10, m11, m12, m20, m21, m22 };
	glUniformMatrix3fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::SetMat4x3F(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23)
{
	const GLfloat v[12] = { m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23 };
	glUniformMatrix4x3fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::SetMat3x4F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12,
	f32 m20, f32 m21, f32 m22,
	f32 m30, f32 m31, f32 m32)
{
	const GLfloat v[12] = { m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32 };
	glUniformMatrix3x4fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::SetMat4x4F(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33)
{
	const GLfloat v[16] = { m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 };
	glUniformMatrix4fv(glGetUniformLocation(g_app.shader, g_app.uniformName), 1, GL_FALSE, v);
}

void App::AddVertex(f32 x, f32 y, f32 z, u32 c)
{
	g_app.vertices.emplace_back(Vertex{ { x, y, z }, c });
}

// Script
void App::ParseFile(const char* moduleName, const char* filepath)
{
	auto src = file_load(filepath);
	ParseSource(moduleName, src.c_str());
}

void App::ParseSource(const char* moduleName, const char* source)
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

void App::BindClass(const char* moduleName, const char* className, ScriptClass scriptClass)
{
	const size_t hash = wren_class_hash(moduleName, className);
	g_app.classes.insert(std::make_pair(hash, scriptClass));
}

void App::BindMethod(const char* moduleName, const char* className, bool isStatic, const char* signature, ScriptMethodFn scriptMethod)
{
	const size_t hash = wren_method_hash(moduleName, className, isStatic, signature);
	g_app.methods.insert(std::make_pair(hash, scriptMethod));
}

void App::EnsureSlots(ScriptVM* vm, i32 count)
{
	wrenEnsureSlots(vm, count);
}

void App::GetVariable(ScriptVM* vm, const char* moduleName, const char* className, i32 slot)
{
	wrenGetVariable(vm, moduleName, className, slot);
}

bool App::GetSlotBool(ScriptVM* vm, i32 slot)
{
	return wrenGetSlotBool(vm, slot);
}

u32 App::GetSlotUInt(ScriptVM* vm, i32 slot)
{
	return (u32)wrenGetSlotDouble(vm, slot);
}

i32 App::GetSlotInt(ScriptVM* vm, i32 slot)
{
	return (i32)wrenGetSlotDouble(vm, slot);
}

f32 App::GetSlotFloat(ScriptVM* vm, i32 slot)
{
	return (f32)wrenGetSlotDouble(vm, slot);
}

f64 App::GetSlotDouble(ScriptVM* vm, i32 slot)
{
	return wrenGetSlotDouble(vm, slot);
}

const char* App::GetSlotString(ScriptVM* vm, i32 slot)
{
	return wrenGetSlotString(vm, slot);
}

void* App::GetSlotObject(ScriptVM* vm, i32 slot)
{
	return wrenGetSlotForeign(vm, slot);
}

const char* App::GetSlotBytes(ScriptVM* vm, i32 slot, i32* length)
{
	return wrenGetSlotBytes(vm, slot, length);
}

void App::SetSlotBool(ScriptVM* vm, i32 slot, bool value)
{
	wrenSetSlotBool(vm, slot, value);
}

void App::SetSlotUInt(ScriptVM* vm, i32 slot, u32 value)
{
	wrenSetSlotDouble(vm, slot, (f64)value);
}

void App::SetSlotInt(ScriptVM* vm, i32 slot, i32 value)
{
	wrenSetSlotDouble(vm, slot, (f64)value);
}

void App::SetSlotFloat(ScriptVM* vm, i32 slot, f32 value)
{
	wrenSetSlotDouble(vm, slot, (f64)value);
}

void App::SetSlotDouble(ScriptVM* vm, i32 slot, f64 value)
{
	wrenSetSlotDouble(vm, slot, (f64)value);
}

void* App::SetSlotNewObject(ScriptVM* vm, i32 slot, i32 classSlot, size_t size)
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
	// Reload wren vm
	if (g_app.vm != nullptr)
		wren_shutdown();
	wren_initialize();

	// Utils
	BindMethod("app", "App", true, "debugBool(_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 2);
			SetSlotBool(vm, 0, DebugBool(GetSlotString(vm, 1), GetSlotBool(vm, 2)));
		});

	BindMethod("app", "App", true, "debugInt(_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 2);
			SetSlotInt(vm, 0, DebugInt(GetSlotString(vm, 1), GetSlotInt(vm, 2)));
		});

	BindMethod("app", "App", true, "debugInt(_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 4);
			SetSlotInt(vm, 0, DebugInt(GetSlotString(vm, 1), GetSlotInt(vm, 2), GetSlotInt(vm, 3), GetSlotInt(vm, 4)));
		});

	BindMethod("app", "App", true, "debugFloat(_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 2);
			SetSlotFloat(vm, 0, DebugFloat(GetSlotString(vm, 1), GetSlotFloat(vm, 2)));
		});

	BindMethod("app", "App", true, "debugSeparator(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			DebugSeparator(GetSlotString(vm, 1));
		});

	BindMethod("app", "App", true, "debugButton(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			SetSlotBool(vm, 0, DebugButton(GetSlotString(vm, 1)));
		});

	// Window
	BindMethod("app", "App", true, "width",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 0);
			SetSlotInt(vm, 0, GetWidth());
		});

	BindMethod("app", "App", true, "height",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 0);
			SetSlotInt(vm, 0, GetHeight());
		});

	BindMethod("app", "App", true, "mouseX",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 0);
			SetSlotDouble(vm, 0, GetMouseX());
		});

	BindMethod("app", "App", true, "mouseY",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 0);
			SetSlotDouble(vm, 0, GetMouseY());
		});

	BindMethod("app", "App", true, "getButton(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			SetSlotBool(vm, 0, GetButton(GetSlotInt(vm, 1)));
		});

	BindMethod("app", "App", true, "getKey(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			SetSlotBool(vm, 0, GetKey(GetSlotInt(vm, 1)));
		});

	BindMethod("app", "App", true, "close()",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 0);
			Close();
		});

	// Graphics
#define SCRIPT_ARGS_ARR(s, n, p, t) p v[n]; for (u8 i = s; i < n; ++i) v[i] = GetSlot##t##(vm, i + 1);

	BindMethod("app", "App", true, "createShader(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			auto shader = CreateShader(GetSlotString(vm, 1));
			SetSlotUInt(vm, 0, shader);
		});

	BindMethod("app", "App", true, "destroyShader(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			auto shader = GetSlotUInt(vm, 1);
			DestroyShader(shader);
		});

	BindMethod("app", "App", true, "setShader(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			auto shader = GetSlotUInt(vm, 1);
			SetShader(shader);
		});

	BindMethod("app", "App", true, "begin(_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 3);
			BeginDraw(GetSlotBool(vm, 1), GetSlotBool(vm, 2), GetSlotFloat(vm, 3), GetSlotFloat(vm, 4));
		});

	BindMethod("app", "App", true, "end(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			EndDraw(GetSlotUInt(vm, 1));
		});

	BindMethod("app", "App", true, "viewport(_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 4, u32, UInt);
			SetViewport(v[0], v[1], v[2], v[3]);
		});

	BindMethod("app", "App", true, "clear(_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 7);
			SCRIPT_ARGS_ARR(0, 6, f32, Float);
			ClearScreen(v[0], v[1], v[2], v[3], v[4], v[5], GetSlotUInt(vm, 7));
		});

	BindMethod("app", "App", true, "uniform(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			const char* value = GetSlotString(vm, 1);
			SetUniform(value);
		});

	BindMethod("app", "App", true, "float(_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 1);
			f32 value = GetSlotFloat(vm, 1);
			SetFloat(value);
		});

	BindMethod("app", "App", true, "vec2f(_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 2);
			SCRIPT_ARGS_ARR(0, 2, f32, Float);
			SetVec2F(v[0], v[1]);
		});

	BindMethod("app", "App", true, "vec3f(_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 3);
			SCRIPT_ARGS_ARR(0, 3, f32, Float);
			SetVec3F(v[0], v[1], v[2]);
		});

	BindMethod("app", "App", true, "vec4f(_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 4);
			SCRIPT_ARGS_ARR(0, 4, f32, Float);
			SetVec4F(v[0], v[1], v[2], v[3]);
		});

	BindMethod("app", "App", true, "mat2x2f(_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 4);
			SCRIPT_ARGS_ARR(0, 4, f32, Float);
			SetMat2x2F(v[0], v[1], v[2], v[3]);
		});

	BindMethod("app", "App", true, "mat3x2f(_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 6);
			SCRIPT_ARGS_ARR(0, 6, f32, Float);
			SetMat3x2F(v[0], v[1], v[2], v[3], v[4], v[5]);
		});

	BindMethod("app", "App", true, "mat2x3f(_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 6);
			SCRIPT_ARGS_ARR(0, 6, f32, Float);
			SetMat2x3F(v[0], v[1], v[2], v[3], v[4], v[5]);
		});

	BindMethod("app", "App", true, "mat3x3f(_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 9);
			SCRIPT_ARGS_ARR(0, 9, f32, Float);
			SetMat3x3F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
		});

	BindMethod("app", "App", true, "mat4x3f(_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 12);
			SCRIPT_ARGS_ARR(0, 12, f32, Float);
			SetMat4x3F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11]);
		});

	BindMethod("app", "App", true, "mat3x4f(_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 12);
			SCRIPT_ARGS_ARR(0, 12, f32, Float);
			SetMat3x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11]);
		});

	BindMethod("app", "App", true, "mat4x4f(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 16);
			SCRIPT_ARGS_ARR(0, 16, f32, Float);
			SetMat4x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
		});

	BindMethod("app", "App", true, "vertex(_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 3, f32, Float);
			AddVertex(v[0], v[1], v[2], GetSlotUInt(vm, 4));
		});

	ParseFile("app", "/Common/app.wren");

	config.bindApiFn();

	// Main callbacks
	ParseFile("main", config.paths[g_app.current_path]);
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
	glViewport(0, 0, GetWidth(), GetHeight());
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("App"))
		{
			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::BeginMenu("Window Mode"))
				{
					if (ImGui::MenuItem("Windowed"))
						SetWindowMode(WindowMode::WINDOW);
					if (ImGui::MenuItem("Borderless"))
						SetWindowMode(WindowMode::BORDERLESS);
					if (ImGui::MenuItem("Fullscreen"))
						SetWindowMode(WindowMode::FULLSCREEN);

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Console"))
					g_app.showConsole = !g_app.showConsole;

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				App::Close();

			ImGui::EndMenu();
		}

		if (ImGui::Button("Reload"))
			g_app.frameOp = FrameOp::RELOAD;

		if (ImGui::Button(g_app.paused ? "Play" : "Pause"))
			g_app.paused = !g_app.paused;

		ImGui::Text("FPS: %.1f", g_app.fps);
		ImGui::Text("Avg Frame (t): %.4f ms", g_app.spf * 1000);

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
		ImGui::Begin("Console", &g_app.showConsole);

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
		{
			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::Selectable("Clear"))
					g_app.logs.clear();
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

	ImGui::Begin("##Overlay", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

	if (!g_app.error && !g_app.paused)
	{
		wrenEnsureSlots(g_app.vm, 1);
		wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
		wrenCall(g_app.vm, g_app.renderMethod);
	}

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