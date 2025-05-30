#include <App.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

extern "C" {
#include <wren.h>
#include <wren_vm.h>
}

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>

struct PathInfo
{
	std::string path{};
	std::string name{};
	std::string ext{};

	std::size_t pathHash{ 0 };
	std::size_t nameHash{ 0 };
	std::size_t extHash{ 0 };
};

struct LogData
{
	u32 color{ 0xFFFFFFFF };
	std::string message{};
	std::size_t hash{ 0 };
	std::size_t count{ 0 };
};

struct AppData
{
	// App
	std::vector<PathInfo> index{};
	std::vector<PathInfo> manifest{};

	int currentIndex{ 0 };

	std::vector<LogData> logs{};
	std::unordered_map<std::size_t, std::size_t> logsHash{};
	std::mutex logMutex{};

	u32 frames{ 0 };
	f64 time{ 0 };
	f64 fps{ 0 };
	f64 spf{ 0 };

	bool headless{ false };

	// Window
	GLFWwindow* window{ nullptr };
	WindowMode winMode{ WindowMode::WINDOWED };
	int winX{ 0 }, winY{ 0 };
	int winWidth{ 0 }, winHeight{ 0 };

	// Gui
	f32 fontSize{ 1.0f };
	bool showImGuiDemo{ false };
	bool showConsole{ false };

	// Script
	WrenVM* vm{ nullptr };
	WrenHandle* mainClass{ nullptr };
	WrenHandle* initMethod{ nullptr };
	WrenHandle* updateMethod{ nullptr };
	WrenHandle* renderMethod{ nullptr };
	WrenHandle* netcodeMethod{ nullptr };
	bool error{ false };
	bool paused{ false };

	FrameOp frameOp{ FrameOp::RELOAD };

	std::unordered_map<size_t, ScriptClass> classes{};
	std::unordered_map<size_t, ScriptMethodFn> methods{};
};
static AppData g_app;

// Static utils
static std::size_t str_hash(const std::string& str)
{
	static std::hash<std::string> g_hash{};
	return g_hash(str);
}

static PathInfo file_path_info(const std::string& fullPath)
{
	PathInfo info;
	info.path = fullPath;

	// Find last '/' or '\' (for both Windows and Unix-style paths)
	std::size_t lastSlash = fullPath.find_last_of("/\\");
	std::size_t lastDot = fullPath.find_last_of('.');

	// Extract filename and extension
	if (lastSlash != std::string::npos)
		info.name = fullPath.substr(lastSlash + 1, (lastDot != std::string::npos ? lastDot - lastSlash - 1 : std::string::npos));
	else
		info.name = fullPath.substr(0, lastDot); // No folder, just the name

	if (lastDot != std::string::npos && lastDot > lastSlash)
		info.ext = fullPath.substr(lastDot + 1);

	// Hash values
	static std::hash<std::string> g_hash;
	info.pathHash = g_hash(info.path);
	info.nameHash = g_hash(info.name);
	info.extHash = g_hash(info.ext);

	return info;
}

static void file_parse_lines(const std::string& fileContent, std::vector<PathInfo>& container)
{
	std::istringstream stream(fileContent);
	std::string line;

	while (std::getline(stream, line))
	{
		if (!line.empty()) // Avoid empty lines
		{
			container.push_back(file_path_info(line));
		}
	}
}

const char* App::FilePath(const char* filepath)
{
#if _DEBUG
	thread_local std::string pathStr{};
	pathStr = PROJECT_PATH + std::string(filepath);
	return pathStr.c_str();
#else
	return filepath;
#endif
}

std::string App::FileLoad(const char* filepath)
{
	const char* path = FilePath(filepath);
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

void App::FileSave(const char* filepath, const std::string& src)
{
	const char* path = FilePath(filepath);
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

	g_app.winMode = config.windowMode;

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

	if (g_app.winMode == WindowMode::FULLSCREEN)
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

	if (g_app.winMode == WindowMode::BORDERLESS)
	{
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

		glfwSetWindowAttrib(g_app.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(g_app.window, 0, 0);
		glfwSetWindowSize(g_app.window, mode->width, mode->height);
	}

	glfwSetInputMode(g_app.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	// Set icon
	GLFWimage icon;
	u32 img = App::GlLoadImage("Assets/App/GASandbox.png", false);
	
	if (img)
	{
		icon.width = App::GlImageWidth(img);
		icon.height = App::GlImageHeight(img);
		icon.pixels = App::GlImageData(img);

		glfwSetWindowIcon(g_app.window, 1, &icon);
		App::GlDestroyImage(img);
	}
	else
	{
		LOGE("Failed to load window icon!");
	}

	return true;
}

static void glfw_shutdown()
{
	glfwDestroyWindow(g_app.window);
	glfwTerminate();
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

	io.Fonts->AddFontFromFileTTF(PATH("Assets/App/Fonts/UbuntuMono-Regular.ttf"), 20);

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

static void* wren_reallocate(void* ptr, std::size_t newSize, void* _)
{
	if (newSize == 0)
	{
		free(ptr);
		return NULL;
	}

	return realloc(ptr, newSize);
}

static const char* wren_resolve_module(WrenVM* vm, const char* importer, const char* name)
{
	return nullptr;
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
	config.reallocateFn = wren_reallocate;
	//config.resolveModuleFn = wren_resolve_module;
	g_app.vm = wrenNewVM(&config);

	return true;
}

static void wren_shutdown()
{
	if (g_app.mainClass) wrenReleaseHandle(g_app.vm, g_app.mainClass);
	if (g_app.initMethod) wrenReleaseHandle(g_app.vm, g_app.initMethod);
	if (g_app.updateMethod) wrenReleaseHandle(g_app.vm, g_app.updateMethod);
	if (g_app.netcodeMethod) wrenReleaseHandle(g_app.vm, g_app.netcodeMethod);
	if (g_app.vm) wrenFreeVM(g_app.vm);

	g_app.vm = nullptr;
	g_app.mainClass = nullptr;
	g_app.initMethod = nullptr;
	g_app.updateMethod = nullptr;
	g_app.netcodeMethod = nullptr;
}

// App
void App::SetFrameOp(FrameOp op)
{
	g_app.frameOp = op;
}

void App::Log(bool verbose, const char* file, i32 line, const char* func, u32 color, const char* format, ...)
{
	static std::string buffer; // Persistent buffer to reduce allocations

	std::lock_guard<std::mutex> lock(g_app.logMutex);

	// Get timestamp
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::ostringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %H:%M:%S]");

	std::string timestamp = ss.str();

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

	const char* filename = std::max(strrchr(file, '/'), strrchr(file, '\\'));
	filename = filename ? filename + 1 : file; // Move past '/' or '\' if found

	// We have the same log already, skip to avoid many repeating logs
	static std::hash<std::string> g_hash{};
	std::size_t hash = g_hash(func) ^ g_hash(filename) ^ line ^ g_hash(buffer);
	auto it = g_app.logsHash.find(hash);
	if (it != g_app.logsHash.end())
	{
		g_app.logs[it->second].count++;
		std::swap(g_app.logs[it->second], g_app.logs.back());
		return;
	}
	g_app.logsHash.insert(std::make_pair(hash, g_app.logs.size()));

	// Create log entry
	LogData log{};
	log.color = color;
	log.hash = hash;

	if (verbose)
	{
		// Format metadata in the same buffer
		size = std::snprintf(nullptr, 0, "%s [%s] (%s:%d)\n%s", timestamp.c_str(), func, filename, line, buffer.c_str());

		std::string temp;
		temp.resize(size + 1);

		std::snprintf(&temp[0], temp.size(), "%s [%s] (%s:%d)\n%s", timestamp.c_str(), func, filename, line, buffer.c_str());

		log.message = std::move(temp);
	}
	else
	{
		log.message = buffer;
	}

#if _DEBUG
	std::cout << log.message << std::endl;
#else
	// Write to file immediately
	std::ofstream logFile("log.txt", std::ios::app); // Append mode
	if (logFile.is_open())
	{
		logFile << log.message << std::endl;

		// Ensure write is committed immediately
		logFile.flush();
		logFile.close();
	}
#endif

	// Store log message
	g_app.logs.emplace_back(std::move(log));

	// Limit log size
	if (g_app.logs.size() > 1024)
	{
		g_app.logsHash.erase(g_app.logs.begin()->hash);
		g_app.logs.erase(g_app.logs.begin());
	}
}

void App::LogClear()
{
	g_app.logs.clear();
	g_app.logsHash.clear();
}

void App::Wait(u32 ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

bool App::IsHeadless()
{
	return g_app.headless;
}

// Window
void* App::WinGetProcAddress(const char* procname)
{
	return glfwGetProcAddress(procname);
}

void App::WinMode(i32 mode)
{
	auto winMode = (WindowMode)mode;
	if (winMode == g_app.winMode)
		return;

	GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidmode = glfwGetVideoMode(pMonitor);

	g_app.winMode = winMode;
	switch(g_app.winMode)
	{
	case WindowMode::FULLSCREEN:
		glfwGetWindowPos(g_app.window, &g_app.winX, &g_app.winY);
		glfwGetWindowSize(g_app.window, &g_app.winWidth, &g_app.winHeight);

		glfwSetWindowMonitor(g_app.window, pMonitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
		break;

	case WindowMode::BORDERLESS:
		glfwGetWindowPos(g_app.window, &g_app.winX, &g_app.winY);
		glfwGetWindowSize(g_app.window, &g_app.winWidth, &g_app.winHeight);

		glfwSetWindowAttrib(g_app.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(g_app.window, 0, 0);
		glfwSetWindowSize(g_app.window, vidmode->width, vidmode->height);
		break;

	case WindowMode::UNDECORATED:
		glfwSetWindowAttrib(g_app.window, GLFW_DECORATED, GLFW_FALSE);
		break;

	default:
		glfwSetWindowMonitor(g_app.window, nullptr, g_app.winX, g_app.winY, g_app.winWidth, g_app.winHeight, 0);
		glfwSetWindowAttrib(g_app.window, GLFW_DECORATED, GLFW_TRUE);
	}
}

void App::WinCursor(i32 cursor)
{
	glfwSetInputMode(g_app.window, GLFW_CURSOR, cursor);
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

// Gui
void App::GuiPushItemWidth(f32 w)
{
	ImGui::PushItemWidth(w);
}

void App::GuiPopItemWidth()
{
	ImGui::PopItemWidth();
}

void App::GuiText(const char* text)
{
	ImGui::Text(text);
}

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

f32 App::GuiFloat(const char* label, f32 v, f32 min, f32 max)
{
	ImGui::SliderFloat(label, &v, min, max);
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

f32 App::GuiContentAvailWidth()
{
	return ImGui::GetContentRegionAvail().x;
}

f32 App::GuiContentAvailHeight()
{
	return ImGui::GetContentRegionAvail().y;
}

bool App::GuiBeginChild(const char* label, f32 w, f32 h)
{
	return ImGui::BeginChild(label, ImVec2(w, h));
}

void App::GuiEndChild()
{
	ImGui::EndChild();
}

// Script
void App::WrenParseFile(const char* moduleName, const char* filepath)
{
	auto src = FileLoad(filepath);
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
	if (!config.headless)
	{
		if (!glfw_initialize(config))
			return false;

		if (!GlInitialize(config))
			return false;

		if (!imgui_initialize(config))
			return false;
	}

	if (!SfxInitialize())
		return false;

	if (!NetInitialize(NetRelay))
		return false;

	std::string indexSrc = FileLoad("Assets/index.txt");
	file_parse_lines(indexSrc, g_app.index);

	std::string manifestSrc = FileLoad("Assets/manifest.txt");
	file_parse_lines(manifestSrc, g_app.manifest);

	g_app.headless = config.headless;

	return true;
}

void App::Shutdown()
{
	wren_shutdown();

	NetShutdown();
	SfxShutdown();
	
	if (!g_app.headless)
	{
		imgui_shutdown();
		GlShutdown();
		glfw_shutdown();
	}
}

void App::Update(f64 dt)
{
	if (!g_app.error)
		wrenCollectGarbage(g_app.vm);

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
		try
		{
			wrenEnsureSlots(g_app.vm, 2);
			wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
			wrenSetSlotDouble(g_app.vm, 1, dt);
			wrenCall(g_app.vm, g_app.updateMethod);
		}
		catch (const std::exception& e)
		{
			LOGE("Script exception: %s", e.what());
			wrenSetSlotString(g_app.vm, 0, e.what());
			wrenAbortFiber(g_app.vm, 0);

			g_app.error = true;
		}
	}
}

void App::Render()
{
	GlViewport(0, 0, WinWidth(), WinHeight());
	GlClear(0, 0, 0, 1, 1, 0, (u32)GlClearFlags::ALL);

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::BeginMenu("App"))
			{
				int idx = 0;
				for (const auto& i : g_app.index)
				{
					if (ImGui::MenuItem(i.path.c_str()))
						break;
					idx++;
				}
				if (idx != g_app.index.size())
				{
					g_app.currentIndex = idx;
					SetFrameOp(FrameOp::RELOAD);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("ImGui Demo"))
					g_app.showImGuiDemo = !g_app.showImGuiDemo;

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
							WinMode((i32)WindowMode::WINDOWED);
						if (ImGui::MenuItem("Undecorated"))
							WinMode((i32)WindowMode::UNDECORATED);
						if (ImGui::MenuItem("Borderless"))
							WinMode((i32)WindowMode::BORDERLESS);
						if (ImGui::MenuItem("Fullscreen"))
							WinMode((i32)WindowMode::FULLSCREEN);

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}
				
				if (ImGui::BeginMenu("GUI"))
				{
					ImGui::SliderFloat("Font Size", &g_app.fontSize, 0.5f, 2.0f);

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				WinClose();

			ImGui::EndMenu();
		}

		ImGui::Text(" | ");

		if (ImGui::MenuItem("Reload"))
			g_app.frameOp = FrameOp::RELOAD;

		if (ImGui::MenuItem(g_app.paused ? "Play" : "Pause"))
			g_app.paused = !g_app.paused;

		const std::size_t bytesAllocated = g_app.error ? 0 : g_app.vm->bytesAllocated;
		ImGui::Text(" |  v%s  | %5.0f fps | %6.2f ms | %6.2f mb", VERSION_STR, g_app.fps, g_app.spf * 1000, bytesAllocated / 100000.f);
		ImGui::EndMainMenuBar();
	}

	if (g_app.showImGuiDemo)
	{
		ImGui::ShowDemoWindow(&g_app.showImGuiDemo);
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
					LogClear();
				
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
				if (log.count > 0)
					ImGui::Text("(%d) %s", log.count, log.message.c_str());
				else
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
	if (!g_app.error)
	{
		try
		{
			wrenEnsureSlots(g_app.vm, 1);
			wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
			wrenCall(g_app.vm, g_app.renderMethod);
		}
		catch (const std::exception& e)
		{
			LOGE("Script exception: %s", e.what());
			wrenSetSlotString(g_app.vm, 0, e.what());
			wrenAbortFiber(g_app.vm, 0);

			g_app.error = true;
		}
	}
	ImGui::SetWindowFontScale(1.0f);

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(g_app.window);
}

void App::NetRelay(bool server, u32 client, u32 event, u32 peer, u32 channel, u32 packet)
{
	if (!g_app.error && !g_app.paused)
	{
		try
		{
			wrenEnsureSlots(g_app.vm, 6);
			wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
			WrenSetSlotBool(g_app.vm, 1, server);
			WrenSetSlotUInt(g_app.vm, 2, client);
			WrenSetSlotUInt(g_app.vm, 3, event);
			WrenSetSlotUInt(g_app.vm, 4, peer);
			WrenSetSlotUInt(g_app.vm, 5, channel);
			WrenSetSlotUInt(g_app.vm, 6, packet);
			wrenCall(g_app.vm, g_app.netcodeMethod);
		}
		catch (const std::exception& e)
		{
			LOGE("Script exception: %s", e.what());
			wrenSetSlotString(g_app.vm, 0, e.what());
			wrenAbortFiber(g_app.vm, 0);

			g_app.error = true;
		}
	}
}

void App::Netcode()
{
	NetPollEvents();
}

int App::Run(const AppConfig& config)
{
	LOGD("App initializing ...");
	if (!Initialize(config))
	{
		return EXIT_FAILURE;
	}
	LOGD("App initialized.");

	f64 lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(g_app.window))
	{
		switch (g_app.frameOp)
		{
		case FrameOp::RELOAD: Reload(); break;
		}
		g_app.frameOp = FrameOp::NONE;

		f64 currentTime = glfwGetTime();
		f64 deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Update(deltaTime);
		if (!config.headless)
			Render();
		Netcode();
	}

	LOGD("App shutting down ...");
	Shutdown();
	LOGD("App shutdown.");

	return EXIT_SUCCESS;
}

void App::Reload()
{
	// Clear logs
	LogClear();

	LOGD("App reloading ...");

	// Reload subsystems
	GlReload();
	SfxReload();
	NetReload();

	// Reload wren vm
	if (g_app.vm != nullptr)
		wren_shutdown();
	wren_initialize();

	// Application
	WrenBindMethod("app", "App", true, "wait(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			Wait(WrenGetSlotUInt(vm, 1));
		});

	WrenBindMethod("app", "App", true, "isHeadless",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotBool(vm, 0, IsHeadless());
		});

	// Window
	WrenBindMethod("app", "App", true, "winMode(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WinMode(WrenGetSlotInt(vm, 1));
		});

	WrenBindMethod("app", "App", true, "winCursor(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WinCursor(WrenGetSlotInt(vm, 1));
		});

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
#define SCRIPT_ARGS_ARR(l, s, n, p, t) p l[n]; for (u8 i = 0; i < n; ++i) l[i] = WrenGetSlot##t##(vm, s + i + 1);

	WrenBindMethod("app", "App", true, "glViewport(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			SCRIPT_ARGS_ARR(vi, 0, 2, i32, Int);
			SCRIPT_ARGS_ARR(vu, 3, 2, u32, UInt);
			GlViewport(vi[0], vi[1], vu[0], vu[1]);
		});

	WrenBindMethod("app", "App", true, "glScissor(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			SCRIPT_ARGS_ARR(vi, 0, 2, i32, Int);
			SCRIPT_ARGS_ARR(vu, 3, 2, u32, UInt);
			GlScissor(vi[0], vi[1], vu[0], vu[1]);
		});

	WrenBindMethod("app", "App", true, "glClear(_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 7);
			SCRIPT_ARGS_ARR(v, 0, 4, f32, Float);
			GlClear(v[0], v[1], v[2], v[3], WrenGetSlotDouble(vm, 7), WrenGetSlotInt(vm, 7), WrenGetSlotUInt(vm, 7));
		});

	WrenBindMethod("app", "App", true, "glLoadShader(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto shader = GlLoadShader(WrenGetSlotString(vm, 1));
			WrenSetSlotUInt(vm, 0, shader);
		});

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

	WrenBindMethod("app", "App", true, "glLoadImage(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			auto image = GlLoadImage(WrenGetSlotString(vm, 1), WrenGetSlotBool(vm, 2));
			WrenSetSlotUInt(vm, 0, image);
		});

	//WrenBindMethod("app", "App", true, "glCreateImage(_,_)",
	//	[](ScriptVM* vm)
	//	{
	//		WrenEnsureSlots(vm, 2);
	//		auto image = GlCreateImage(WrenGetSlotString(vm, 1), WrenGetSlotBool(vm, 2));
	//		WrenSetSlotUInt(vm, 0, image);
	//	});

	WrenBindMethod("app", "App", true, "glDestroyImage(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto image = WrenGetSlotUInt(vm, 1);
			GlDestroyImage(image);
		});

	WrenBindMethod("app", "App", true, "glImageWidth(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto image = WrenGetSlotUInt(vm, 1);
			WrenSetSlotInt(vm, 0, GlImageWidth(image));
		});

	WrenBindMethod("app", "App", true, "glImageHeight(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto image = WrenGetSlotUInt(vm, 1);
			WrenSetSlotInt(vm, 0, GlImageHeight(image));
		});

	WrenBindMethod("app", "App", true, "glImageChannels(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto image = WrenGetSlotUInt(vm, 1);
			WrenSetSlotInt(vm, 0, GlImageChannels(image));
		});

	WrenBindMethod("app", "App", true, "glLoadModel(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto model = GlLoadModel(WrenGetSlotString(vm, 1));
			WrenSetSlotUInt(vm, 0, model);
		});

	WrenBindMethod("app", "App", true, "glDestroyModel(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto model = WrenGetSlotUInt(vm, 1);
			GlDestroyModel(model);
		});

	WrenBindMethod("app", "App", true, "glCreateTexture(_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 7);
			SCRIPT_ARGS_ARR(v, 0, 6, u32, UInt);
			auto texture = GlCreateTexture(v[0], (TextureFormat)v[1], (TextureFilter)v[2], (TextureFilter)v[3], (TextureWrap)v[4], (TextureWrap)v[5], WrenGetSlotBool(vm, 7));
			WrenSetSlotUInt(vm, 0, texture);
		});

	WrenBindMethod("app", "App", true, "glDestroyTexture(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto texture = WrenGetSlotUInt(vm, 1);
			GlDestroyTexture(texture);
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

	WrenBindMethod("app", "App", true, "glSetUniform(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			const char* value = WrenGetSlotString(vm, 1);
			GlSetUniform(value);
		});

	WrenBindMethod("app", "App", true, "glSetTex2D(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			GlSetTex2D(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2));
		});

	WrenBindMethod("app", "App", true, "glSetFloat(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			f32 value = WrenGetSlotFloat(vm, 1);
			GlSetFloat(value);
		});

	WrenBindMethod("app", "App", true, "glSetVec2f(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			SCRIPT_ARGS_ARR(v, 0, 2, f32, Float);
			GlSetVec2F(v[0], v[1]);
		});

	WrenBindMethod("app", "App", true, "glSetVec3f(_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 3);
			SCRIPT_ARGS_ARR(v, 0, 3, f32, Float);
			GlSetVec3F(v[0], v[1], v[2]);
		});

	WrenBindMethod("app", "App", true, "glSetVec4f(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			SCRIPT_ARGS_ARR(v, 0, 4, f32, Float);
			GlSetVec4F(v[0], v[1], v[2], v[3]);
		});

	WrenBindMethod("app", "App", true, "glSetMat2x2f(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			SCRIPT_ARGS_ARR(v, 0, 4, f32, Float);
			GlSetMat2x2F(v[0], v[1], v[2], v[3]);
		});

	WrenBindMethod("app", "App", true, "glSetMat2x3f(_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 6);
			SCRIPT_ARGS_ARR(v, 0, 6, f32, Float);
			GlSetMat2x3F(v[0], v[1], v[2], v[3], v[4], v[5]);
		});

	WrenBindMethod("app", "App", true, "glSetMat2x4f(_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 8);
			SCRIPT_ARGS_ARR(v, 0, 8, f32, Float);
			GlSetMat2x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
		});

	WrenBindMethod("app", "App", true, "glSetMat3x2f(_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 6);
			SCRIPT_ARGS_ARR(v, 0, 6, f32, Float);
			GlSetMat3x2F(v[0], v[1], v[2], v[3], v[4], v[5]);
		});

	WrenBindMethod("app", "App", true, "glSetMat3x3f(_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 9);
			SCRIPT_ARGS_ARR(v, 0, 9, f32, Float);
			GlSetMat3x3F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
		});

	WrenBindMethod("app", "App", true, "glSetMat3x4f(_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 12);
			SCRIPT_ARGS_ARR(v, 0, 12, f32, Float);
			GlSetMat3x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11]);
		});

	WrenBindMethod("app", "App", true, "glSetMat4x2f(_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 8);
			SCRIPT_ARGS_ARR(v, 0, 8, f32, Float);
			GlSetMat4x2F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
		});

	WrenBindMethod("app", "App", true, "glSetMat4x3f(_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 12);
			SCRIPT_ARGS_ARR(v, 0, 12, f32, Float);
			GlSetMat4x3F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11]);
		});

	WrenBindMethod("app", "App", true, "glSetMat4x4f(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 16);
			SCRIPT_ARGS_ARR(v, 0, 16, f32, Float);
			GlSetMat4x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
		});

	WrenBindMethod("app", "App", true, "glAddVertex(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 5);
			SCRIPT_ARGS_ARR(p, 0, 4, f32, Float);
			SCRIPT_ARGS_ARR(c, 4, 4, u32, UInt);
			SCRIPT_ARGS_ARR(v, 8, 8, f32, Float);
			GlAddVertex(
				p[0], p[1], p[2], p[3], c[0], c[1], c[2], c[3],
				v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
		});

	// Gui
	WrenBindMethod("app", "App", true, "guiPushItemWidth(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiPushItemWidth(WrenGetSlotFloat(vm, 1));
		});

	WrenBindMethod("app", "App", true, "guiPopItemWidth()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiPopItemWidth();
		});

	WrenBindMethod("app", "App", true, "guiText(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiText(WrenGetSlotString(vm, 1));
		});

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

	WrenBindMethod("app", "App", true, "guiFloat(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotFloat(vm, 0, GuiFloat(WrenGetSlotString(vm, 1), WrenGetSlotFloat(vm, 2), WrenGetSlotFloat(vm, 3), WrenGetSlotFloat(vm, 4)));
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

	WrenBindMethod("app", "App", true, "guiContentAvailWidth()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotFloat(vm, 0, GuiContentAvailWidth());
		});

	WrenBindMethod("app", "App", true, "guiContentAvailHeight()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotFloat(vm, 0, GuiContentAvailHeight());
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

	// Audio
	WrenBindMethod("app", "App", true, "sfxLoadAudio(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			u32 audio = SfxLoadAudio(WrenGetSlotString(vm, 1));
			WrenSetSlotUInt(vm, 0, audio);
		});

	WrenBindMethod("app", "App", true, "sfxDestroyAudio(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			SfxDestroyAudio(WrenGetSlotUInt(vm, 1));
		});

	WrenBindMethod("app", "App", true, "sfxCreateChannel(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			u32 channel = SfxCreateChannel(WrenGetSlotFloat(vm, 1));
			WrenSetSlotUInt(vm, 0, channel);
		});

	WrenBindMethod("app", "App", true, "sfxDestroyChannel(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			SfxDestroyChannel(WrenGetSlotUInt(vm, 1));
		});

	WrenBindMethod("app", "App", true, "sfxSetChannelVolume(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			SfxSetChannelVolume(WrenGetSlotUInt(vm, 1), WrenGetSlotFloat(vm, 2));
		});

	WrenBindMethod("app", "App", true, "sfxPlay(_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 3);
			SfxPlay(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2), WrenGetSlotBool(vm, 3));
		});

	WrenBindMethod("app", "App", true, "sfxStop(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			SfxStop(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2));
		});

	// Net
	WrenBindMethod("app", "App", true, "netStartServer(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			NetStartServer(WrenGetSlotString(vm, 1), WrenGetSlotUInt(vm, 2), WrenGetSlotUInt(vm, 3), WrenGetSlotUInt(vm, 4));
		});

	WrenBindMethod("app", "App", true, "netStopServer()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			NetStopServer();
		});

	WrenBindMethod("app", "App", true, "netConnectClient(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			u32 client = NetConnectClient(WrenGetSlotString(vm, 1), WrenGetSlotUInt(vm, 2), WrenGetSlotUInt(vm, 3), WrenGetSlotUInt(vm, 4));
			WrenSetSlotUInt(vm, 0, client);
		});

	WrenBindMethod("app", "App", true, "netDisconnectClient(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			NetDisconnectClient(WrenGetSlotUInt(vm, 1));
		});

	WrenBindMethod("app", "App", true, "netMakeUuid()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotUInt(vm, 0, NetMakeUUID());
		});

	WrenBindMethod("app", "App", true, "netIsServer()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotBool(vm, 0, NetIsServer());
		});

	WrenBindMethod("app", "App", true, "netIsClient(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			wrenSetSlotBool(vm, 0, NetIsClient(WrenGetSlotUInt(vm, 1)));
		});

	WrenBindMethod("app", "App", true, "netCreatePacket(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			auto packet = NetCreatePacket(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2));
			WrenSetSlotUInt(vm, 0, packet);
		});

	WrenBindMethod("app", "App", true, "netPacketId(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			auto id = NetPacketId(WrenGetSlotUInt(vm, 1));
			WrenSetSlotUInt(vm, 0, id);
		});

	WrenBindMethod("app", "App", true, "netBroadcast(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			NetBroadcast(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2));
		});

	WrenBindMethod("app", "App", true, "netSend(_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 3);
			NetSend(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2), WrenGetSlotUInt(vm, 3));
		});

#define SCRIPT_NET_GET(Type)																					\
	WrenBindMethod("app", "App", true, "netGet"#Type"(_,_)",											\
	[](ScriptVM* vm)																					\
		{																								\
			WrenEnsureSlots(vm, 2);																		\
			WrenSetSlot##Type(vm, 0, NetGet##Type(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2)));		\
		});

	SCRIPT_NET_GET(Bool);
	SCRIPT_NET_GET(UInt);
	SCRIPT_NET_GET(Int);
	SCRIPT_NET_GET(Float);
	SCRIPT_NET_GET(Double);

	WrenBindMethod("app", "App", true, "netGetString(_,_)", 
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			wrenSetSlotString(vm, 0, NetGetString(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2)));
		});

#define SCRIPT_NET_SET(Type)																					\
	WrenBindMethod("app", "App", true, "netSet"#Type"(_,_,_)",											\
	[](ScriptVM* vm)																					\
		{																								\
			WrenEnsureSlots(vm, 3);																		\
			NetSet##Type(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2), WrenGetSlot##Type(vm, 3));		\
		});

	SCRIPT_NET_SET(Bool);
	SCRIPT_NET_SET(UInt);
	SCRIPT_NET_SET(Int);
	SCRIPT_NET_SET(Float);
	SCRIPT_NET_SET(Double);

	WrenBindMethod("app", "App", true, "netSetString(_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 3);
			NetSetString(WrenGetSlotUInt(vm, 1), WrenGetSlotUInt(vm, 2), WrenGetSlotString(vm, 3));
		});

	for (const auto& path : g_app.manifest)
	{
		static std::size_t main_hash = str_hash("main");
		if (path.nameHash == main_hash)
			continue;

		static std::size_t wren_hash = str_hash("wren");
		if (path.extHash == wren_hash)
			WrenParseFile(path.name.c_str(), path.path.c_str());
	}

	// Main callbacks
	const auto& index = g_app.index[g_app.currentIndex];
	WrenParseFile(index.name.c_str(), index.path.c_str());

	if (!g_app.error)
	{
		g_app.initMethod = wrenMakeCallHandle(g_app.vm, "init()");
		g_app.updateMethod = wrenMakeCallHandle(g_app.vm, "update(_)");
		g_app.renderMethod = wrenMakeCallHandle(g_app.vm, "render()");
		g_app.netcodeMethod = wrenMakeCallHandle(g_app.vm, "netcode(_,_,_,_,_,_)");

		wrenEnsureSlots(g_app.vm, 1);
		wrenGetVariable(g_app.vm, "main", "Main", 0);
		g_app.mainClass = wrenGetSlotHandle(g_app.vm, 0);
		wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);

		try
		{
			wrenEnsureSlots(g_app.vm, 1);
			wrenSetSlotHandle(g_app.vm, 0, g_app.mainClass);
			wrenCall(g_app.vm, g_app.initMethod);
		}
		catch (const std::exception& e)
		{
			LOGE("Script exception: %s", e.what());
			wrenSetSlotString(g_app.vm, 0, e.what());
			wrenAbortFiber(g_app.vm, 0);

			g_app.error = true;
		}
	}
	else
	{
		wren_shutdown();
	}

	LOGD("App reloaded.");
}