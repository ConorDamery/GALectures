#include <App.hpp>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <backends/imgui_impl_glfw.h>

struct State
{
	// Window
	GLFWwindow* window{ nullptr };
	WindowMode winMode{ WindowMode::WINDOWED };
	int winX{ 0 }, winY{ 0 };
	int winWidth{ 0 }, winHeight{ 0 };
};

static State g_state{};

static void glfw_error_callback(int i, const char* c)
{
	LOGE("GLFW Error [%s]: ", c);
}

bool App::WinInitialize(const AppConfig& config)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		LOGE("Failed to initialize GLFW!");
		return false;
	}

	g_state.winMode = config.windowMode;

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

	if (g_state.winMode == WindowMode::FULLSCREEN)
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

	g_state.window = glfwCreateWindow(width, height, config.title, pMonitor, NULL);
	if (g_state.window == NULL)
	{
		LOGE("Failed to create GLFW window!");
		return false;
	}

	glfwMakeContextCurrent(g_state.window);
	glfwSwapInterval(1);

	if (g_state.winMode == WindowMode::BORDERLESS)
	{
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

		glfwSetWindowAttrib(g_state.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(g_state.window, 0, 0);
		glfwSetWindowSize(g_state.window, mode->width, mode->height);
	}

	glfwSetInputMode(g_state.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	// Set icon
	GLFWimage icon;
	u32 img = App::GlLoadImage("Assets/App/GASandbox.png", false);

	if (img)
	{
		icon.width = App::GlImageWidth(img);
		icon.height = App::GlImageHeight(img);
		icon.pixels = App::GlImageData(img);

		glfwSetWindowIcon(g_state.window, 1, &icon);
		App::GlDestroyImage(img);
	}
	else
	{
		LOGE("Failed to load window icon!");
	}

	return true;
}

void App::WinShutdown()
{
	glfwDestroyWindow(g_state.window);
	glfwTerminate();
}

void App::WinReload()
{
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

	WrenBindMethod("app", "App", true, "winAlwaysOnTop(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WinAlwaysOnTop(WrenGetSlotBool(vm, 1));
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
}

// Window
bool App::GuiWinInitialize()
{
	return ImGui_ImplGlfw_InitForOpenGL(g_state.window, true);
}

void App::GuiWinShutdown()
{
	ImGui_ImplGlfw_Shutdown();
}

void App::GuiWinNewFrame()
{
	ImGui_ImplGlfw_NewFrame();
}

void App::WinPollEvents()
{
	glfwPollEvents();
}

bool App::WinShouldClose()
{
	return glfwWindowShouldClose(g_state.window);
}

void App::WinSwapBuffers()
{
	glfwSwapBuffers(g_state.window);
}

f64 App::GetTime()
{
	return glfwGetTime();
}

void* App::WinGetProcAddress(const char* procname)
{
	return glfwGetProcAddress(procname);
}

void App::WinMode(i32 mode)
{
	auto winMode = (WindowMode)mode;
	if (winMode == g_state.winMode)
		return;

	GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidmode = glfwGetVideoMode(pMonitor);

	g_state.winMode = winMode;
	switch (g_state.winMode)
	{
	case WindowMode::FULLSCREEN:
		glfwGetWindowPos(g_state.window, &g_state.winX, &g_state.winY);
		glfwGetWindowSize(g_state.window, &g_state.winWidth, &g_state.winHeight);

		glfwSetWindowMonitor(g_state.window, pMonitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
		break;

	case WindowMode::BORDERLESS:
		glfwGetWindowPos(g_state.window, &g_state.winX, &g_state.winY);
		glfwGetWindowSize(g_state.window, &g_state.winWidth, &g_state.winHeight);

		glfwSetWindowAttrib(g_state.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(g_state.window, 0, 0);
		glfwSetWindowSize(g_state.window, vidmode->width, vidmode->height);
		break;

	case WindowMode::UNDECORATED:
		glfwSetWindowAttrib(g_state.window, GLFW_DECORATED, GLFW_FALSE);
		break;

	default:
		glfwSetWindowMonitor(g_state.window, nullptr, g_state.winX, g_state.winY, g_state.winWidth, g_state.winHeight, 0);
		glfwSetWindowAttrib(g_state.window, GLFW_DECORATED, GLFW_TRUE);
	}
}

void App::WinCursor(i32 cursor)
{
	glfwSetInputMode(g_state.window, GLFW_CURSOR, cursor);
}

void App::WinAlwaysOnTop(bool enabled)
{
	glfwSetWindowAttrib(g_state.window, GLFW_FLOATING, enabled ? GLFW_TRUE : GLFW_FALSE);
}

i32 App::WinWidth()
{
	i32 w, h;
	glfwGetWindowSize(g_state.window, &w, &h);
	return w;
}

i32 App::WinHeight()
{
	i32 w, h;
	glfwGetWindowSize(g_state.window, &w, &h);
	return h;
}

f64 App::WinMouseX()
{
	f64 x, y;
	glfwGetCursorPos(g_state.window, &x, &y);
	return x;
}

f64 App::WinMouseY()
{
	f64 x, y;
	glfwGetCursorPos(g_state.window, &x, &y);
	return y;
}

bool App::WinButton(i32 b)
{
	return glfwGetMouseButton(g_state.window, b) == GLFW_PRESS;
}

bool App::WinKey(i32 k)
{
	GLFW_KEY_W;
	return glfwGetKey(g_state.window, k) == GLFW_PRESS;
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
	glfwSetWindowShouldClose(g_state.window, GLFW_TRUE);
}