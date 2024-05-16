#include "App.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <wren.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

struct AppData
{
	// Window
	GLFWwindow* window = nullptr;

	// Graphics
	GLuint shader = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
	std::vector<u8> cmds;

	// Script
	WrenVM* vm = nullptr;
	WrenHandle* gameClass = nullptr;
	WrenHandle* updateMethod = nullptr;
	bool error = false;
	bool reload = true;

	std::string gameSource;
	std::unordered_map<size_t, ScriptClass> classes;
	std::unordered_map<size_t, ScriptMethodFn> methods;

	// Editor
	f32 fontSize = 1.0f;
};
static AppData g_app;

// Window
void App::GetSize(i32& w, i32& h)
{
	glfwGetWindowSize(g_app.window, &w, &h);
}

void App::Close()
{
	glfwSetWindowShouldClose(g_app.window, GLFW_TRUE);
}

// Input
void App::GetMouse(f64& x, f64& y)
{
	glfwGetCursorPos(g_app.window, &x, &y);
}

bool App::GetButton(i32 b)
{
	return glfwGetMouseButton(g_app.window, b) == GLFW_PRESS;
}

bool App::GetKey(i32 k)
{
	return glfwGetKey(g_app.window, k) == GLFW_PRESS;
}

// Graphics
struct Camera
{
	static constexpr u8 CmdId = 0;

	f32 px = 0, py = 0, pz = 0;
	f32 vx = 0, vy = 0, vz = 0;
	f32 zn = 0, zf = 0;
	bool ortho = false;
	f32 param = 0;
};

struct Line2
{
	static constexpr u8 CmdId = 1;

	f32 x1 = 0, y1 = 0;
	f32 x2 = 0, y2 = 0;
	u32 c = 0;
};

struct Line3
{
	static constexpr u8 CmdId = 2;

	f32 x1 = 0, y1 = 0, z1 = 0;
	f32 x2 = 0, y2 = 0, z2 = 0;
	u32 c = 0;
};

struct Quad2
{
	static constexpr u8 CmdId = 3;

	f32 x1 = 0, y1 = 0;
	f32 x2 = 0, y2 = 0;
	u32 c = 0;
};

struct Quad3
{
	static constexpr u8 CmdId = 4;

	f32 x1 = 0, y1 = 0, z1 = 0;
	f32 x2 = 0, y2 = 0, z2 = 0;
	u32 c = 0;
};

void App::SetCamera(f32 px, f32 py, f32 pz, f32 vx, f32 vy, f32 vz, f32 zn, f32 zf, bool ortho, f32 param)
{
	g_app.cmds.resize(1 + sizeof(Camera));
	g_app.cmds.emplace_back(Camera::CmdId);
}

void App::DrawLine2(f32 x1, f32 y1, f32 x2, f32 y2, u32 c)
{
}

void App::DrawLine3(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, u32 c)
{
}

void App::DrawQuad2(f32 x1, f32 y1, f32 x2, f32 y2, u32 c)
{
}
void App::DrawQuad3(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, u32 c)
{
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
	printf("%s", text);
}

static void wren_error(WrenVM* vm, WrenErrorType errorType, const char* module, const int line, const char* msg)
{
	switch (errorType)
	{
	case WREN_ERROR_COMPILE:
		std::cout << "Wren Error Compile" << std::endl;
		break;// Log::Error("[{} line {}] {}", module, line, msg); break;
	case WREN_ERROR_STACK_TRACE:
		std::cout << "Wren Error Stack Trace" << std::endl;
		break;//Log::Error("[{} line {}] in {}", module, line, msg); break;
	case WREN_ERROR_RUNTIME:
		std::cout << "Wren Error Runtime" << std::endl;
		break;//Log::Error("[Runtime] {}", msg); break;
	}

	//s_error = true;
}

void App::Reload()
{
	if (g_app.vm != nullptr)
	{
		wrenFreeVM(g_app.vm);
	}

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

	ParseSource("game", g_app.gameSource.c_str());
	if (!g_app.error)
	{
		wrenEnsureSlots(g_app.vm, 1);
		wrenGetVariable(g_app.vm, "game", "Game", 0);
		g_app.gameClass = wrenGetSlotHandle(g_app.vm, 0);
		wrenSetSlotHandle(g_app.vm, 0, g_app.gameClass);
		g_app.updateMethod = wrenMakeCallHandle(g_app.vm, "update()");
	}
}

void App::ParseFile(const char* moduleName, const char* filename)
{
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	ParseSource(moduleName, buffer.str().c_str());
}

void App::ParseSource(const char* moduleName, const char* source)
{
	switch (wrenInterpret(g_app.vm, moduleName, source))
	{
	case WREN_RESULT_COMPILE_ERROR:
		g_app.error = true;
		std::cout << "Wren compilation error on module: " << moduleName << std::endl;
		break;

	case WREN_RESULT_RUNTIME_ERROR:
		g_app.error = true;
		std::cout << "Wren runtime error on module: " << moduleName << std::endl;
		break;

	case WREN_RESULT_SUCCESS:
		std::cout << "Wren successfully compiled module: " << moduleName << std::endl;
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

// Setup
static void glfw_error_callback(int i, const char* c)
{
	std::cout << "GLFW Error [" << i << "]: " << c << std::endl;
}

static bool glfw_initialize(const AppConfig& config)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW!" << std::endl;
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
		std::cout << "Failed to create GLFW window!" << std::endl;
		return false;
	}

	glfwMakeContextCurrent(g_app.window);
	glfwSwapInterval(1);
}

static void APIENTRY opengl_debug_callback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// Ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	std::cout << "GL Severity [ID]: Message" << std::endl;// -Source:({}) - Type : ({}) - Severity : ({})\n{}", id, GetGlSource(source), GetGlType(type), GetGlSeverity(severity), message);
}

static bool opengl_initialize(const AppConfig& config)
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD!" << std::endl;
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
}

static bool imgui_initialize(const AppConfig& config)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	if (!ImGui_ImplGlfw_InitForOpenGL(g_app.window, true))
	{
		std::cout << "Failed to initialize ImGui GLFW!" << std::endl;
		return false;
	}

	if (!ImGui_ImplOpenGL3_Init("#version 330 core\n"))
	{
		std::cout << "Failed to initialize ImGui OpenGL!" << std::endl;
		return false;
	}

	ImGui::StyleColorsDark();
}

bool App::Initialize(const AppConfig& config)
{
	if (!glfw_initialize(config))
		return false;

	if (!opengl_initialize(config))
		return false;

	if (!imgui_initialize(config))
		return false;

	g_app.gameSource =
		"class Game {\n"
		"	static update() {\n"
		"		System.print(\"Update!\")\n"
		"	}\n"
		"}";

	return true;
}

void App::Shutdown()
{
	wrenFreeVM(g_app.vm);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_app.window);
	glfwTerminate();
}

void App::Update()
{
	glfwPollEvents();

	wrenEnsureSlots(g_app.vm, 1);
	wrenSetSlotHandle(g_app.vm, 0, g_app.gameClass);
	wrenCall(g_app.vm, g_app.updateMethod);
}

void App::Render()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Game", 0, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open")) {
				//LoadFile(filename, code);
			}
			if (ImGui::MenuItem("Save")) {
				//SaveFile(filename, code);
			}
			if (ImGui::MenuItem("Exit"))
				Close();
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGui::Button("Reload"))
		g_app.reload = true;

	ImGui::SameLine();

	ImGui::InputFloat("Font Size", &g_app.fontSize, 0.1f, 1.0f, "%.3f", 0);
	g_app.fontSize = std::max(0.5f, g_app.fontSize);

	ImGui::SetWindowFontScale(g_app.fontSize);
	ImGui::InputTextMultiline("##Source", &g_app.gameSource, ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_AllowTabInput);
	ImGui::SetWindowFontScale(1.0f);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(g_app.window);
}

int App::Run(AppConfigureFn Configure, AppPostInitializeFn PostInitialize)
{
	if (!Initialize(Configure()))
	{
		return 1;
	}

	while (!glfwWindowShouldClose(g_app.window))
	{
		if (g_app.reload)
		{
			g_app.reload = false;

			Reload();
			PostInitialize();
		}

		Update();
		Render();
	}

	Shutdown();
	return 0;
}