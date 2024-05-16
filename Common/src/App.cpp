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
	std::string message;
};

struct Vertex
{
	f32 pos[3] = { 0, 0, 0 };
	u32 col = 0;
};

struct AppData
{
	// Util
	std::vector<LogData> logs;

	// Window
	GLFWwindow* window = nullptr;

	// Graphics
	GLuint shader = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
	std::vector<Vertex> points;
	std::vector<Vertex> lines;
	std::vector<Vertex> quads;
	std::vector<u8> cmds;

	// Script
	WrenVM* vm = nullptr;
	WrenHandle* gameClass = nullptr;
	WrenHandle* initMethod = nullptr;
	WrenHandle* updateMethod = nullptr;
	bool error = false;
	bool reload = true;
	bool paused = false;

	std::string filepath;
	std::string script;
	std::unordered_map<size_t, ScriptClass> classes;
	std::unordered_map<size_t, ScriptMethodFn> methods;

	// Editor
	f32 fontSize = 1.0f;
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
struct Camera
{
	f32 m00 = 0, m01 = 0, m02 = 0, m03 = 0;
	f32 m10 = 0, m11 = 0, m12 = 0, m13 = 0;
	f32 m20 = 0, m21 = 0, m22 = 0, m23 = 0;
	f32 m30 = 0, m31 = 0, m32 = 0, m33 = 0;
};

struct Point2
{
	f32 x1 = 0, y1 = 0;
	u32 c = 0;
};

struct Line2
{
	f32 x1 = 0, y1 = 0;
	f32 x2 = 0, y2 = 0;
	u32 c = 0;
};

struct Point3
{
	f32 x1 = 0, y1 = 0, z1 = 0;
	u32 c = 0;
};

struct Line3
{
	f32 x1 = 0, y1 = 0, z1 = 0;
	f32 x2 = 0, y2 = 0, z2 = 0;
	u32 c = 0;
};

struct Quad2
{
	f32 x1 = 0, y1 = 0;
	f32 x2 = 0, y2 = 0;
	u32 c = 0;
};

struct Quad3
{
	f32 x1 = 0, y1 = 0, z1 = 0;
	f32 x2 = 0, y2 = 0, z2 = 0;
	u32 c = 0;
};

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

void App::SetCamera(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33)
{
	cmd_push(Camera{ m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 });
}

void App::DrawPoint2(f32 x1, f32 y1, u32 c)
{
	cmd_push(Point2{ x1, y1, c });
}

void App::DrawLine2(f32 x1, f32 y1, f32 x2, f32 y2, u32 c)
{
	cmd_push(Line2{ x1, y1, x2, y2, c});
}

void App::DrawQuad2(f32 x1, f32 y1, f32 x2, f32 y2, u32 c)
{
	cmd_push(Quad2{ x1, y1, x2, y2, c });
}

void App::DrawPoint3(f32 x1, f32 y1, f32 z1, u32 c)
{
	cmd_push(Point3{ x1, y1, z1, c });
}

void App::DrawLine3(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, u32 c)
{
	cmd_push(Line3{ x1, y1, z1, x2, y2, z2, c });
}

void App::DrawQuad3(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, u32 c)
{
	cmd_push(Quad3{ x1, y1, z1, x2, y2, z2, c });
}

template <>
static void cmd_impl(const Camera& v)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glEnable(GL_DEPTH_TEST);

	glPointSize(5.0f);
	glLineWidth(2.0f);

	glBindVertexArray(g_app.vao);

	glUseProgram(g_app.shader);
	glUniformMatrix4fv(glGetUniformLocation(g_app.shader, "ProjMtx"), 1, GL_FALSE, (GLfloat*)&v);

	glBindBuffer(GL_ARRAY_BUFFER, g_app.vbo);

	glBufferData(GL_ARRAY_BUFFER, g_app.points.size() * sizeof(Vertex), g_app.points.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_POINTS, 0, (GLsizei)g_app.points.size());

	glBufferData(GL_ARRAY_BUFFER, g_app.lines.size() * sizeof(Vertex), g_app.lines.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_LINES, 0, (GLsizei)g_app.lines.size());

	glBufferData(GL_ARRAY_BUFFER, g_app.quads.size() * sizeof(Vertex), g_app.quads.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)g_app.quads.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);
	glBindVertexArray(0);

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_BLEND);

	g_app.points.clear();
	g_app.lines.clear();
	g_app.quads.clear();
}

template <>
static void cmd_impl(const Point2& v)
{
	g_app.points.emplace_back(Vertex{ v.x1, v.y1, 0, v.c });
}

template <>
static void cmd_impl(const Line2& v)
{
	g_app.lines.emplace_back(Vertex{ v.x1, v.y1, 0, v.c });
	g_app.lines.emplace_back(Vertex{ v.x2, v.y2, 0, v.c });
}

template <>
static void cmd_impl(const Quad2& v)
{
	g_app.quads.emplace_back(Vertex{ v.x1, v.y1, 0, v.c });
	g_app.quads.emplace_back(Vertex{ v.x2, v.y1, 0, v.c });
	g_app.quads.emplace_back(Vertex{ v.x1, v.y2, 0, v.c });

	g_app.quads.emplace_back(Vertex{ v.x2, v.y2, 0, v.c });
	g_app.quads.emplace_back(Vertex{ v.x1, v.y2, 0, v.c });
	g_app.quads.emplace_back(Vertex{ v.x2, v.y1, 0, v.c });
}

template <>
static void cmd_impl(const Point3& v)
{
	g_app.points.emplace_back(Vertex{ v.x1, v.y1, v.z1, v.c });
}

template <>
static void cmd_impl(const Line3& v)
{
	g_app.lines.emplace_back(Vertex{ v.x1, v.y1, v.z1, v.c });
	g_app.lines.emplace_back(Vertex{ v.x2, v.y2, v.z2, v.c });
}

template <>
static void cmd_impl(const Quad3& v)
{
	g_app.quads.emplace_back(Vertex{ v.x1, v.y1, v.z1, v.c });
	g_app.quads.emplace_back(Vertex{ v.x2, v.y1, v.z1, v.c });
	g_app.quads.emplace_back(Vertex{ v.x1, v.y2, v.z2, v.c });

	g_app.quads.emplace_back(Vertex{ v.x2, v.y2, v.z2, v.c });
	g_app.quads.emplace_back(Vertex{ v.x1, v.y2, v.z2, v.c });
	g_app.quads.emplace_back(Vertex{ v.x2, v.y1, v.z1, v.c });
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

	//LOAD_WREN_MODULE(meta, WREN_MODULE_SRC(Meta));
	//LOAD_WREN_MODULE(random, WREN_MODULE_SRC(Random));
	//
	//LOAD_WREN_MODULE(core, ENGINE_MODULE_SRC(core));
	//LOAD_WREN_MODULE(device, ENGINE_MODULE_SRC(device));
	//LOAD_WREN_MODULE(math, ENGINE_MODULE_SRC(math));
	//LOAD_WREN_MODULE(graphics, ENGINE_MODULE_SRC(graphics));
	//LOAD_WREN_MODULE(physics, ENGINE_MODULE_SRC(physics));
	//LOAD_WREN_MODULE(audio, ENGINE_MODULE_SRC(audio));
	//LOAD_WREN_MODULE(ecs, ENGINE_MODULE_SRC(ecs));
	//LOAD_WREN_MODULE(framework, ENGINE_MODULE_SRC(framework));
	//
	//String moduleName = name;
	//if (s_wrenModulesSource.Find(moduleName) == s_wrenModulesSource.end())
	//{
	//	String filepath;
	//	if (!File::Find("[assets]", String(name) + ".wren", filepath))
	//	{
	//		Log::Error("Module '{}' can not be found!", name);
	//		return res;
	//	}
	//
	//	String moduleSrc = File::ReadTextFile(filepath);
	//	s_wrenModulesSource.Insert(moduleName, moduleSrc);
	//}
	//
	//res.source = s_wrenModulesSource[moduleName].c_str();
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

static void APIENTRY opengl_debug_callback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// Ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	LOGW("GL Severity [ID]: Message");// -Source:({}) - Type : ({}) - Severity : ({})\n{}", id, GetGlSource(source), GetGlType(type), GetGlSeverity(severity), message);
}

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

	//i32 flags;
	//glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	//if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	//{
	//	// Initialize debug output
	//	glEnable(GL_DEBUG_OUTPUT);
	//	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//	glDebugMessageCallback(DebugCallback, nullptr);
	//	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	//
	//	return true;
	//}

	static const char* vert_src =
		"#version 330 core\n"
		"precision highp float;\n"
		"layout (location = 0) in vec3 Position;\n"
		"layout (location = 1) in vec4 Color;\n"
		"out vec4 Frag_Color;\n"
		"uniform mat4 ProjMtx;\n"
		"void main()\n"
		"{\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * vec4(Position, 1);\n"
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
#define SCRIPT_ARGS_ARR(s, n, t) f32 v[n]; for (u8 i = s; i < n; ++i) v[i] = GetSlot##t##(vm, i + 1);

	BindMethod("app", "App", true, "setCamera(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 10);
			SCRIPT_ARGS_ARR(0, 16, Float);
			SetCamera(
				v[0], v[1], v[2], v[3],
				v[4], v[5], v[6], v[7],
				v[8], v[9], v[10], v[11],
				v[12], v[13], v[14], v[15]);
		});

	BindMethod("app", "App", true, "drawPoint2(_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 2, Float);
			DrawPoint2(v[0], v[1], GetSlotUInt(vm, 3));
		});

	BindMethod("app", "App", true, "drawLine2(_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 4, Float);
			DrawLine2(v[0], v[1], v[2], v[3], GetSlotUInt(vm, 5));
		});

	BindMethod("app", "App", true, "drawQuad2(_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 4, Float);
			DrawQuad2(v[0], v[1], v[2], v[3], GetSlotUInt(vm, 5));
		});

	BindMethod("app", "App", true, "drawPoint3(_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(0, 3, Float);
			DrawPoint3(v[0], v[1], v[2], GetSlotUInt(vm, 4));
		});

	BindMethod("app", "App", true, "drawLine3(_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 7);
			SCRIPT_ARGS_ARR(0, 6, Float);
			DrawLine3(v[0], v[1], v[2], v[3], v[4], v[5], GetSlotUInt(vm, 7));
		});

	BindMethod("app", "App", true, "drawQuad3(_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			EnsureSlots(vm, 7);
			SCRIPT_ARGS_ARR(0, 6, Float);
			DrawQuad3(v[0], v[1], v[2], v[3], v[4], v[5], GetSlotUInt(vm, 7));
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

void App::Update(f64 dt)
{
	glfwPollEvents();

	if (!g_app.error && !g_app.paused)
	{
		wrenEnsureSlots(g_app.vm, 2);
		wrenSetSlotHandle(g_app.vm, 0, g_app.gameClass);
		wrenSetSlotDouble(g_app.vm, 1, dt);
		wrenCall(g_app.vm, g_app.updateMethod);
	}
}

static void app_render_graphics()
{
	u8* ptr = g_app.cmds.data();
	while (ptr != g_app.cmds.data() + g_app.cmds.size())
	{
		if (cmd_execute<Camera>(ptr))
			continue;
		if (cmd_execute<Point2>(ptr))
			continue;
		if (cmd_execute<Line2>(ptr))
			continue;
		if (cmd_execute<Quad2>(ptr))
			continue;
		if (cmd_execute<Point3>(ptr))
			continue;
		if (cmd_execute<Line3>(ptr))
			continue;
		if (cmd_execute<Quad3>(ptr))
			continue;

		break;
	}
	g_app.cmds.clear();

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
		LOGE("OpenGL error: %d", err);
}

static void app_render_gui()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

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
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(((log.color >> 24) & 0xFF) / 255.0f, ((log.color >> 16) & 0xFF) / 255.0f, ((log.color >> 8) & 0xFF) / 255.0f, (log.color & 0xFF) / 255.0f));
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

	//ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::Render()
{
	glViewport(0, 0, GetWidth(), GetHeight());
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	app_render_graphics();
	app_render_gui();

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