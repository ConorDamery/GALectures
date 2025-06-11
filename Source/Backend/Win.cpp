#include <App.hpp>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <backends/imgui_impl_glfw.h>

using namespace GASandbox;

struct WinGlobal
{
	// Window
	GLFWwindow* window{ nullptr };
	eWinMode winMode{ eWinMode::WINDOWED };
	i32 winX{ 0 }, winY{ 0 };
	i32 winWidth{ 0 }, winHeight{ 0 };
};
static WinGlobal g{};

static void glfw_error_callback(int i, cstring c)
{
	LOGE("GLFW Error [%s]: ", c);
}

bool App::WinInitialize(const sAppConfig& config)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		LOGE("Failed to initialize GLFW!");
		return false;
	}

	g.winMode = config.windowMode;

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

	if (g.winMode == eWinMode::FULLSCREEN)
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

	g.window = glfwCreateWindow(width, height, config.title, pMonitor, NULL);
	if (g.window == NULL)
	{
		LOGE("Failed to create GLFW window!");
		return false;
	}

	glfwMakeContextCurrent(g.window);
	glfwSwapInterval(1);

	if (g.winMode == eWinMode::BORDERLESS)
	{
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

		glfwSetWindowAttrib(g.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(g.window, 0, 0);
		glfwSetWindowSize(g.window, mode->width, mode->height);
	}

	glfwSetInputMode(g.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	// Set icon
	GLFWimage icon;
	u32 img = App::GlLoadImage("Assets/App/GASandbox.png", false);

	if (img)
	{
		icon.width = App::GlImageWidth(img);
		icon.height = App::GlImageHeight(img);
		icon.pixels = App::GlImageData(img);

		glfwSetWindowIcon(g.window, 1, &icon);
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
	glfwDestroyWindow(g.window);
	glfwTerminate();
}

void App::WinReload()
{
	// Window
	CodeBindMethod("app", "App", true, "winMode(_)",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 1);
			WinMode((eWinMode)CodeGetSlotInt(vm, 1));
		});

	CodeBindMethod("app", "App", true, "winCursor(_)",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 1);
			WinCursor((eWinCursor)CodeGetSlotInt(vm, 1));
		});

	CodeBindMethod("app", "App", true, "winAlwaysOnTop(_)",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 1);
			WinAlwaysOnTop(CodeGetSlotBool(vm, 1));
		});

	CodeBindMethod("app", "App", true, "winWidth",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 0);
			CodeSetSlotInt(vm, 0, WinWidth());
		});

	CodeBindMethod("app", "App", true, "winHeight",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 0);
			CodeSetSlotInt(vm, 0, WinHeight());
		});

	CodeBindMethod("app", "App", true, "winMouseX",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 0);
			CodeSetSlotDouble(vm, 0, WinMouseX());
		});

	CodeBindMethod("app", "App", true, "winMouseY",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 0);
			CodeSetSlotDouble(vm, 0, WinMouseY());
		});

	CodeBindMethod("app", "App", true, "winButton(_)",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 1);
			CodeSetSlotBool(vm, 0, WinButton(CodeGetSlotInt(vm, 1)));
		});

	CodeBindMethod("app", "App", true, "winKey(_)",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 1);
			CodeSetSlotBool(vm, 0, WinKey(CodeGetSlotInt(vm, 1)));
		});

	CodeBindMethod("app", "App", true, "winPadCount()",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 1);
			CodeSetSlotInt(vm, 0, WinPadCount());
		});

	CodeBindMethod("app", "App", true, "winPadButton(_,_)",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 2);
			CodeSetSlotBool(vm, 0, WinPadButton(CodeGetSlotInt(vm, 1), CodeGetSlotInt(vm, 2)));
		});

	CodeBindMethod("app", "App", true, "winPadAxis(_,_)",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 2);
			CodeSetSlotFloat(vm, 0, WinPadAxis(CodeGetSlotInt(vm, 1), CodeGetSlotInt(vm, 2)));
		});

	CodeBindMethod("app", "App", true, "winClose()",
		[](sCodeVM* vm)
		{
			CodeEnsureSlots(vm, 0);
			WinClose();
		});
}

// Window
bool App::GuiWinInitialize()
{
	return ImGui_ImplGlfw_InitForOpenGL(g.window, true);
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
	return glfwWindowShouldClose(g.window);
}

void App::WinSwapBuffers()
{
	glfwSwapBuffers(g.window);
}

f64 App::GetTime()
{
	return glfwGetTime();
}

void* App::WinGetProcAddress(cstring procname)
{
	return glfwGetProcAddress(procname);
}

void App::WinMode(eWinMode mode)
{
	auto winMode = mode;
	if (winMode == g.winMode)
		return;

	GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidmode = glfwGetVideoMode(pMonitor);

	g.winMode = winMode;
	switch (g.winMode)
	{
	case eWinMode::FULLSCREEN:
		glfwGetWindowPos(g.window, &g.winX, &g.winY);
		glfwGetWindowSize(g.window, &g.winWidth, &g.winHeight);

		glfwSetWindowMonitor(g.window, pMonitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
		break;

	case eWinMode::BORDERLESS:
		glfwGetWindowPos(g.window, &g.winX, &g.winY);
		glfwGetWindowSize(g.window, &g.winWidth, &g.winHeight);

		glfwSetWindowAttrib(g.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowPos(g.window, 0, 0);
		glfwSetWindowSize(g.window, vidmode->width, vidmode->height);
		break;

	case eWinMode::UNDECORATED:
		glfwSetWindowAttrib(g.window, GLFW_DECORATED, GLFW_FALSE);
		break;

	default:
		glfwSetWindowMonitor(g.window, nullptr, g.winX, g.winY, g.winWidth, g.winHeight, 0);
		glfwSetWindowAttrib(g.window, GLFW_DECORATED, GLFW_TRUE);
	}
}

void App::WinCursor(eWinCursor cursor)
{
	glfwSetInputMode(g.window, GLFW_CURSOR, (i32)cursor);
}

void App::WinAlwaysOnTop(bool enabled)
{
	glfwSetWindowAttrib(g.window, GLFW_FLOATING, enabled ? GLFW_TRUE : GLFW_FALSE);
}

i32 App::WinWidth()
{
	i32 w, h;
	glfwGetWindowSize(g.window, &w, &h);
	return w;
}

i32 App::WinHeight()
{
	i32 w, h;
	glfwGetWindowSize(g.window, &w, &h);
	return h;
}

f64 App::WinMouseX()
{
	f64 x, y;
	glfwGetCursorPos(g.window, &x, &y);
	return x;
}

f64 App::WinMouseY()
{
	f64 x, y;
	glfwGetCursorPos(g.window, &x, &y);
	return y;
}

bool App::WinButton(i32 b)
{
	return glfwGetMouseButton(g.window, b) == GLFW_PRESS;
}

bool App::WinKey(i32 k)
{
	GLFW_KEY_W;
	return glfwGetKey(g.window, k) == GLFW_PRESS;
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
	glfwSetWindowShouldClose(g.window, GLFW_TRUE);
}