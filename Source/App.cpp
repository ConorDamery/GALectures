#include <App.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

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

using namespace GASandbox;

struct LogData
{
	u32 color{ 0xFFFFFFFF };
	string message{};
	size_type hash{ 0 };
	size_type count{ 0 };
};

struct AppGlobal
{
	// App
	list<LogData> logs{};
	hashmap<size_type, size_type> logsHash{};
	std::mutex logMutex{};

	u32 frames{ 0 };
	f64 time{ 0 };
	f64 fps{ 0 };
	f64 spf{ 0 };

	int currentIndex{ 0 };

	bool reload{ true };
	bool headless{ false };

	// Gui
	f32 fontSize{ 1.0f };
	bool showImGuiDemo{ false };
	bool showConsole{ false };
	bool winAlwaysOnTop{ false };
};
static AppGlobal g;

// Static utils
static size_type str_hash(const string& str)
{
	static std::hash<string> g_hash{};
	return g_hash(str);
}

// App
void App::QueueReload()
{
	g.reload = true;
}

void App::Log(bool verbose, cstring file, i32 line, cstring func, u32 color, cstring format, ...)
{
	static string buffer; // Persistent buffer to reduce allocations

	std::lock_guard<std::mutex> lock(g.logMutex);

	// Get timestamp
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::ostringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %H:%M:%S]");

	string timestamp = ss.str();

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

	cstring filename = std::max(strrchr(file, '/'), strrchr(file, '\\'));
	filename = filename ? filename + 1 : file; // Move past '/' or '\' if found

	// We have the same log already, skip to avoid many repeating logs
	static std::hash<string> g_hash{};
	size_type hash = g_hash(func) ^ g_hash(filename) ^ line ^ g_hash(buffer);
	auto it = g.logsHash.find(hash);
	if (it != g.logsHash.end())
	{
		g.logs[it->second].count++;
		std::swap(g.logs[it->second], g.logs.back());
		return;
	}
	g.logsHash.insert(std::make_pair(hash, g.logs.size()));

	// Create log entry
	LogData log{};
	log.color = color;
	log.hash = hash;

	if (verbose)
	{
		// Format metadata in the same buffer
		size = std::snprintf(nullptr, 0, "%s [%s] (%s:%d)\n%s", timestamp.c_str(), func, filename, line, buffer.c_str());

		string temp;
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
	g.logs.emplace_back(std::move(log));

	// Limit log size
	if (g.logs.size() > 1024)
	{
		g.logsHash.erase(g.logs.begin()->hash);
		g.logs.erase(g.logs.begin());
	}
}

void App::LogClear()
{
	g.logs.clear();
	g.logsHash.clear();
}

void App::Wait(u32 ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

bool App::IsHeadless()
{
	return g.headless;
}

size_type App::Hash(cstring str)
{
	return Hash(string{ str });
}

size_type App::Hash(const string& str)
{
	static std::hash<string> h{};
	return h(str);
}

bool App::Initialize(const sAppConfig& config)
{
	if (!config.headless)
	{
		if (!WinInitialize(config))
			return false;

		if (!GlInitialize(config))
			return false;

		if (!GuiInitialize(config))
			return false;
	}

	if (!SfxInitialize(config))
		return false;

	if (!NetInitialize(config))
		return false;

	if (!FileInitialize(config))
		return false;

	g.headless = config.headless;

	return true;
}

void App::Shutdown()
{
	FileShutdown();

	ScriptShutdown();

	NetShutdown();
	SfxShutdown();
	
	if (!g.headless)
	{
		GuiShutdown();
		GlShutdown();
		WinShutdown();
	}
}

void App::Update(f64 dt)
{
	ScriptCollectGarbage();
	WinPollEvents();
	NetPollEvents();

	g.frames++;
	g.time += dt;
	if (g.time >= 1.f)
	{
		g.fps = g.frames / g.time;
		g.spf = g.time / g.frames;
		g.frames = 1;
		g.time = std::fmodf(g.time, 1.f);
	}

	ScriptUpdate(dt);
}

void App::Render()
{
	GlViewport(0, 0, WinWidth(), WinHeight());
	GlClear(0, 0, 0, 1, 1, 0, eGlClearFlags::ALL);

	GuiWinNewFrame();
	GuiGlNewFrame();
	ImGui::NewFrame();

	GuiRender();

	ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

	ImGui::Begin("##Overlay", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

	ImGui::SetWindowFontScale(g.fontSize);
	ScriptRender();
	ImGui::SetWindowFontScale(1.0f);

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor();
	ImGui::End();

	GuiGlRender();

	WinSwapBuffers();
}

void App::GuiRender()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::BeginMenu("App"))
			{
				const auto& index = FileGetIndex();
				int idx = 0;
				for (const auto& i : index)
				{
					if (ImGui::MenuItem(i.path.c_str()))
						break;
					idx++;
				}
				if (idx != index.size())
				{
					g.currentIndex = idx;
					QueueReload();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::MenuItem("Console"))
					g.showConsole = !g.showConsole;

#ifdef _DEBUG
				ImGui::Separator();
				if (ImGui::MenuItem("ImGui Demo"))
					g.showImGuiDemo = !g.showImGuiDemo;
#endif

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::BeginMenu("Window"))
				{
					if (ImGui::Checkbox("Always On Top", &g.winAlwaysOnTop))
						WinAlwaysOnTop(g.winAlwaysOnTop);

					if (ImGui::BeginMenu("Mode"))
					{
						if (ImGui::MenuItem("Windowed"))
							WinMode(eWinMode::WINDOWED);
						if (ImGui::MenuItem("Undecorated"))
							WinMode(eWinMode::UNDECORATED);
						if (ImGui::MenuItem("Borderless"))
							WinMode(eWinMode::BORDERLESS);
						if (ImGui::MenuItem("Fullscreen"))
							WinMode(eWinMode::FULLSCREEN);

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("GUI"))
				{
					if (ImGui::MenuItem("Load Style"))
						GuiLoadStyle();
					if (ImGui::MenuItem("Save Style"))
						GuiSaveStyle();
					if (ImGui::MenuItem("Reset Style"))
						GuiResetStyle();

					ImGui::Separator();
					ImGui::SliderFloat("Font Size", &g.fontSize, 0.5f, 2.0f);

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
			QueueReload();

		if (ImGui::MenuItem(ScriptIsPaused() ? "Play" : "Pause"))
			ScriptTogglePaused();

		const size_type bytesAllocated = ScriptBytesAllocated();
		ImGui::Text(" |  v%s  | %5.0f fps | %6.2f ms | %6.2f mb", VERSION_STR, g.fps, g.spf * 1000, bytesAllocated / 100000.f);
		ImGui::EndMainMenuBar();
	}

	if (g.showImGuiDemo)
	{
		ImGui::ShowDemoWindow(&g.showImGuiDemo);
	}

	if (g.showConsole)
	{
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.05f, ImGui::GetFrameHeight() + (ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()) * 0.25f), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.9f, (ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()) * 0.5f), ImGuiCond_Appearing);

		ImGui::Begin("Console", &g.showConsole, ImGuiWindowFlags_NoSavedSettings);

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
		{
			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::Selectable("Clear"))
					LogClear();

				if (ImGui::Selectable("Copy"))
				{
					static string clipboard;
					clipboard.clear();
					clipboard.reserve(4096);

					for (const auto& log : g.logs)
					{
						size_type length = log.message.size();
						if (length > 0 && log.message[length - 1] == '\0') --length;
						clipboard.append(log.message, 0, length).append("\n");
					}

					ImGui::SetClipboardText(clipboard.c_str());
				}

				ImGui::EndPopup();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
			for (const auto& log : g.logs)
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
}

bool App::Configure(int argc, char** args, sAppConfig& config)
{
	config.title = "GA Sandbox";
	config.width = 800;
	config.height = 600;
	config.windowMode = eWinMode::WINDOWED;
	config.msaa = 8;
	config.headless = false;

	return true;
}

int App::Run(int argc, char** args)
{
	sAppConfig config{};
	LOGD("App configuring ...");
	if (!Configure(argc, args, config))
	{
		return EXIT_FAILURE;
	}
	LOGD("App configured.");

	LOGD("App initializing ...");
	if (!Initialize(config))
	{
		return EXIT_FAILURE;
	}
	LOGD("App initialized.");

	f64 lastTime = GetTime();
	while (!WinShouldClose())
	{
		if (g.reload)
			Reload(config);

		f64 currentTime = GetTime();
		f64 deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Update(deltaTime);
		if (!config.headless)
			Render();
	}

	LOGD("App shutting down ...");
	Shutdown();
	LOGD("App shutdown.");

	return EXIT_SUCCESS;
}

void App::Reload(const sAppConfig& config)
{
	g.reload = false;

	// Clear logs
	LogClear();

	LOGD("App reloading ...");

	// Reload wren vm
	ScriptShutdown();
	ScriptInitialize(config);

	// Reload subsystems
	WinReload();
	GlReload();
	GuiReload();
	SfxReload();
	NetReload();

	// Application API
	CodeBindMethod("app", "App", true, "wait(_)",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 1);
			Wait(CodeGetSlotUInt(vm, 1));
		});

	CodeBindMethod("app", "App", true, "isHeadless",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 1);
			CodeSetSlotBool(vm, 0, IsHeadless());
		});

	// Load scripts in manifest
	const auto& manifest = FileGetManifest();
	for (const auto& path : manifest)
	{
		static size_type main_hash = str_hash("main");
		if (path.nameHash == main_hash)
			continue;

		static size_type wren_hash = str_hash("wren");
		if (path.extHash == wren_hash)
			CodeParseFile(path.name.c_str(), path.path.c_str());
	}

	// Main callbacks
	const auto& index = FileGetIndex();
	const auto& current = index[g.currentIndex];
	CodeParseFile(current.name.c_str(), current.path.c_str());

	WrenReload();
	
	LOGD("App reloaded.");
}