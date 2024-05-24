#include "App.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
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

struct Begin
{
	bool alpha = true;
	bool ztest = true;
	f32 pointSize = 5.0f;
	f32 lineWidth = 2.0f;
};

struct End
{
	u32 mode = 0;
};

struct Viewport
{
	u32 x = 0, y = 0;
	u32 w = 0, h = 0;
};

struct Clear
{
	f32 r = 0, g = 0, b = 0, a = 0;
	f32 d = 0, s = 0;
	u32 flags = 0;
};

struct View
{
	f32 m[16]{};
};

struct Projection
{
	f32 m[16]{};
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
	u32 frames = 0;
	f64 time = 0.f;
	f64 fps = 0.f;
	f64 spf = 0.f;

	// Window
	GLFWwindow* window = nullptr;

	// Graphics
	GLuint shader = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
	std::vector<Vertex> vertices{};
	std::vector<u8> cmds{};

	View view{};
	Projection projection{};

	// Script
	WrenVM* vm = nullptr;
	WrenHandle* gameClass = nullptr;
	WrenHandle* initMethod = nullptr;
	WrenHandle* updateMethod = nullptr;
	bool error = false;
	bool reload = true;
	bool paused = false;

	std::string filepath{};
	std::string script{};
	std::unordered_map<size_t, ScriptClass> classes{};
	std::unordered_map<size_t, ScriptMethodFn> methods{};

	// Editor
	f32 fontSize = 1.5f;
};
static AppData g_app;

// Util
void App::Log(bool verbose, const char* file, i32 line, const char* func, u32 color, const char* format, ...)
{
	// Initialize the argument list
	va_list args;
	va_start(args, format);

	// Determine the size of the formatted string
	va_list args_copy;
	va_copy(args_copy, args);
	int size = std::vsnprintf(nullptr, 0, format, args_copy);
	va_end(args_copy);

	if (size < 0)
	{
		va_end(args);
		throw std::runtime_error("Error during formatting.");
	}

	// Allocate memory for the formatted string
	std::vector<char> buffer(size + 1);
	std::vsnprintf(buffer.data(), buffer.size(), format, args);

	va_end(args);

	// Output the formatted string to the console
	LogData log{};
	log.color = color;

	if (verbose)
		log.message = "[" + std::string(file) + ":" + std::to_string(line) + " (" + func + ")]\n" + buffer.data();
	else
		log.message = buffer.data();

	g_app.logs.emplace_back(log);

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

// Window
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
static u32 cmd_new_id()
{
	static u32 counter = 0;
	return ++counter;
}

static u32 cmd_id(u8* ptr)
{
	u32 id = *reinterpret_cast<u32*>(ptr);
	return id;
}

template <typename T>
static const T& cmd_data(u8* ptr)
{
	const auto& data = *reinterpret_cast<T*>(ptr + sizeof(u32));
	return data;
}

template <typename T>
static void cmd_push(const T& cmd)
{
	const size_t offset = g_app.cmds.size();
	g_app.cmds.resize(g_app.cmds.size() + sizeof(u32) + sizeof(T));
	u8* ptr = g_app.cmds.data() + offset;

	u32& id = *reinterpret_cast<u32*>(ptr);
	id = CmdId<T>::Id();
	ptr += sizeof(u32);

	std::memcpy(ptr, &cmd, sizeof(T));
}

template <typename T>
struct CmdIdImpl
{
	static u32 Id()
	{
		static const u32 id = cmd_new_id();
		return id;
	}
};

template <typename T>
struct CmdId
{
	static u32 Id()
	{
		return CmdIdImpl<std::decay_t<T>>::Id();
	}
};

template <typename T>
static void cmd_impl(const T& v) { return false; }

template <typename T>
static bool cmd_execute(u8*& ptr)
{
	u32 id = cmd_id(ptr);
	if (id != CmdId<T>::Id())
		return false;

	cmd_impl(cmd_data<T>(ptr));
	ptr += sizeof(u32) + sizeof(T);

	return true;
}

void App::BeginDraw(bool alpha, bool ztest, f32 pointSize, f32 lineWidth)
{
	cmd_push(Begin{ alpha, ztest, pointSize, lineWidth });
}

void App::EndDraw(u32 mode)
{
	cmd_push(End{ mode });
}

void App::SetViewport(u32 x, u32 y, u32 w, u32 h)
{
	cmd_push(Viewport{ x, y, w, h });
}

void App::ClearScreen(f32 r, f32 g, f32 b, f32 a, f32 d, f32 s, u32 flags)
{
	cmd_push(Clear{ r, g, b, a, d, s, flags });
}

void App::SetView(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33)
{
	cmd_push(View{ m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 });
}

void App::SetProjection(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33)
{
	cmd_push(Projection{ m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 });
}

void App::AddVertex(f32 x, f32 y, f32 z, u32 c)
{
	cmd_push(Vertex{ x, y, z, c});
}

template <>
static void cmd_impl(const Begin& v)
{
	if (v.alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	if (v.ztest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glPointSize(v.pointSize);
	glLineWidth(v.lineWidth);
}

template <>
static void cmd_impl(const End& v)
{
	glBindVertexArray(g_app.vao);

	glUseProgram(g_app.shader);
	glUniformMatrix4fv(glGetUniformLocation(g_app.shader, "ViewMtx"), 1, GL_FALSE, (GLfloat*)&g_app.view);
	glUniformMatrix4fv(glGetUniformLocation(g_app.shader, "ProjMtx"), 1, GL_FALSE, (GLfloat*)&g_app.projection);

	glBindBuffer(GL_ARRAY_BUFFER, g_app.vbo);

	GLenum mode = GL_POINTS;
	switch (v.mode)
	{
	case 0: mode = GL_POINTS; break;
	case 1: mode = GL_LINES; break;
	case 2: mode = GL_TRIANGLES; break;
	}

	glBufferData(GL_ARRAY_BUFFER, g_app.vertices.size() * sizeof(Vertex), g_app.vertices.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(mode, 0, (GLsizei)g_app.vertices.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);
	glBindVertexArray(0);

	g_app.vertices.clear();
}

template <>
static void cmd_impl(const Viewport& v)
{
	glViewport(v.x, v.y, v.w, v.h);
}

template <>
static void cmd_impl(const Clear& v)
{
	glClearColor(v.r, v.g, v.b, v.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

template <>
static void cmd_impl(const View& v)
{
	g_app.view = v;
}

template <>
static void cmd_impl(const Projection& v)
{
	g_app.projection = v;
}

template <>
static void cmd_impl(const Vertex& v)
{
	g_app.vertices.emplace_back(v);
}

// Script
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

static std::string file_load(const char* filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filepath << std::endl;
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
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filepath << std::endl;
		return;
	}

	file << src;
	file.close();
}

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

// Setup
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

	if (config.fullscreen)
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

	return true;
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

	static const char* vert_src =
		"#version 330 core\n"
		"precision highp float;\n"
		"layout (location = 0) in vec3 Position;\n"
		"layout (location = 1) in vec4 Color;\n"
		"out vec4 Frag_Color;\n"
		"uniform mat4 ViewMtx;\n"
		"uniform mat4 ProjMtx;\n"
		"void main()\n"
		"{\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * ViewMtx * vec4(Position, 1);\n"
		"}";
	static const char* frag_src =
		"#version 330 core\n"
		"precision mediump float;\n"
		"in vec4 Frag_Color;\n"
		"layout (location = 0) out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"	Out_Color = Frag_Color;\n"
		"}";

	g_app.shader = opengl_load_shader(vert_src, frag_src);

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
	io.Fonts->AddFontFromFileTTF(PATH("/Common/Fonts/UbuntuMono-Regular.ttf"), 20);

	io.IniFilename = PATH("/imgui.ini");
	return true;
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

bool App::Initialize(const AppConfig& config)
{
	if (!glfw_initialize(config))
		return false;

	if (!opengl_initialize(config))
		return false;

	if (!imgui_initialize(config))
		return false;

	g_app.filepath = config.gamefile;
	g_app.script = file_load(g_app.filepath.c_str());

	return true;
}

static void glfw_shutdown()
{
	glfwDestroyWindow(g_app.window);
	glfwTerminate();
}

static void opengl_shutdown()
{
	glDeleteProgram(g_app.shader);
	glDeleteBuffers(1, &g_app.vbo);
	glDeleteVertexArrays(1, &g_app.vao);
}

static void imgui_shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

static void wren_shutdown()
{
	if (g_app.gameClass) wrenReleaseHandle(g_app.vm, g_app.gameClass);
	if (g_app.initMethod) wrenReleaseHandle(g_app.vm, g_app.initMethod);
	if (g_app.updateMethod) wrenReleaseHandle(g_app.vm, g_app.updateMethod);
	if (g_app.vm) wrenFreeVM(g_app.vm);

	g_app.vm = nullptr;
	g_app.gameClass = nullptr;
	g_app.initMethod = nullptr;
	g_app.updateMethod = nullptr;
}

void App::Shutdown()
{
	wren_shutdown();
	imgui_shutdown();
	opengl_shutdown();
	glfw_shutdown();
}

void App::Reload(AppBindApiFn BindApi)
{
	if (!g_app.reload)
		return;
	g_app.reload = false;

	if (g_app.vm != nullptr)
	{
		wren_shutdown();
	}

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

	BindMethod("app", "App", true, "view(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 10);
			SCRIPT_ARGS_ARR(0, 16, f32, Float);
			SetView(
				v[0], v[1], v[2], v[3],
				v[4], v[5], v[6], v[7],
				v[8], v[9], v[10], v[11],
				v[12], v[13], v[14], v[15]);
		});

	BindMethod("app", "App", true, "projection(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 10);
			SCRIPT_ARGS_ARR(0, 16, f32, Float);
			SetProjection(
				v[0], v[1], v[2], v[3],
				v[4], v[5], v[6], v[7],
				v[8], v[9], v[10], v[11],
				v[12], v[13], v[14], v[15]);
		});

	BindMethod("app", "App", true, "vertex(_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 3, f32, Float);
			AddVertex(v[0], v[1], v[2], GetSlotUInt(vm, 4));
		});

	ParseFile("app", PATH("/Common/app.wren"));

	BindApi();

	// Game callbacks
	ParseSource("game", g_app.script.c_str());
	if (!g_app.error)
	{
		wrenEnsureSlots(g_app.vm, 1);
		wrenGetVariable(g_app.vm, "game", "Game", 0);
		g_app.gameClass = wrenGetSlotHandle(g_app.vm, 0);
		wrenSetSlotHandle(g_app.vm, 0, g_app.gameClass);
		g_app.initMethod = wrenMakeCallHandle(g_app.vm, "init()");
		g_app.updateMethod = wrenMakeCallHandle(g_app.vm, "update(_)");

		wrenEnsureSlots(g_app.vm, 1);
		wrenSetSlotHandle(g_app.vm, 0, g_app.gameClass);
		wrenCall(g_app.vm, g_app.initMethod);
	}
	else
	{
		wren_shutdown();
	}
}

static void app_update_gui()
{
	ImGui::Begin("App", 0, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit"))
				App::Close();
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGui::BeginTabBar("TabBar"))
	{
		if (ImGui::BeginTabItem("Code"))
		{
			// Script loader
			ImGui::InputText("Path", &g_app.filepath);

			ImGui::SameLine();

			if (ImGui::Button("Load"))
				g_app.script = file_load(g_app.filepath.c_str());

			ImGui::SameLine();

			if (ImGui::Button("Save"))
				file_save(g_app.filepath.c_str(), g_app.script);

			// Code editor
			if (ImGui::Button("Reload"))
				g_app.reload = true;

			ImGui::SameLine();

			if (ImGui::Button(g_app.paused ? "Play" : "Pause"))
				g_app.paused = !g_app.paused;

			ImGui::SameLine();

			ImGui::InputFloat("Font Size", &g_app.fontSize, 0.1f, 1.0f, "%.3f", 0);
			g_app.fontSize = std::max(0.5f, g_app.fontSize);

			ImGui::SetWindowFontScale(g_app.fontSize);
			ImGui::InputTextMultiline("##Source", &g_app.script, ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_AllowTabInput);
			ImGui::SetWindowFontScale(1.0f);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Console"))
		{
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

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void App::Update(f64 dt)
{
	glfwPollEvents();

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	g_app.frames++;
	g_app.time += dt;
	if (g_app.time >= 1.f)
	{
		g_app.fps = g_app.frames / g_app.time;
		g_app.spf = g_app.time / g_app.frames;
		g_app.frames = 0;
		g_app.time -= 1.f;
	}

	ImGui::Begin("Debug");
	ImGui::Text("FPS: %2.f", g_app.fps);
	ImGui::Text("Avg Frame (t): %2.fms", g_app.spf * 1000);

	if (!g_app.error && !g_app.paused)
	{
		g_app.cmds.clear();

		wrenEnsureSlots(g_app.vm, 2);
		wrenSetSlotHandle(g_app.vm, 0, g_app.gameClass);
		wrenSetSlotDouble(g_app.vm, 1, dt);
		wrenCall(g_app.vm, g_app.updateMethod);
	}

	ImGui::End();

	app_update_gui();
}

static void app_render_graphics()
{
	u8* ptr = g_app.cmds.data();
	while (ptr != g_app.cmds.data() + g_app.cmds.size())
	{
		if (cmd_execute<Begin>(ptr))
			continue;
		if (cmd_execute<End>(ptr))
			continue;
		if (cmd_execute<Viewport>(ptr))
			continue;
		if (cmd_execute<Clear>(ptr))
			continue;
		if (cmd_execute<View>(ptr))
			continue;
		if (cmd_execute<Projection>(ptr))
			continue;
		if (cmd_execute<Vertex>(ptr))
			continue;

		break;
	}
}

void App::Render()
{
	glViewport(0, 0, GetWidth(), GetHeight());
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	app_render_graphics();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(g_app.window);
}

int App::Run(AppConfigureFn Configure, AppBindApiFn BindApi)
{
	if (!Initialize(Configure()))
	{
		return EXIT_FAILURE;
	}

	f64 lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(g_app.window))
	{
		Reload(BindApi);

		f64 currentTime = glfwGetTime();
		f64 deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Update(deltaTime);
		Render();
	}

	Shutdown();
	return EXIT_SUCCESS;
}