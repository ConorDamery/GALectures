#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>

// Macros
#define XSTR(x) #x
#define STR(x) XSTR(x)

#define VERSION_DEV 1
#define VERSION_MINOR 0
#define VERSION_MAJOR 0

#define VERSION_STR STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_DEV)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BIT(n) (1u << (n - 1))

#define LOGD(format, ...) App::Log(true, __FILE__, __LINE__, __func__, 0xFFA0A0A0, format, ##__VA_ARGS__)
#define LOGI(format, ...) App::Log(true, __FILE__, __LINE__, __func__, 0xFFFFFFFF, format, ##__VA_ARGS__)
#define LOGW(format, ...) App::Log(true, __FILE__, __LINE__, __func__, 0xFF00FFFF, format, ##__VA_ARGS__)
#define LOGE(format, ...) App::Log(true, __FILE__, __LINE__, __func__, 0xFF0000FF, format, ##__VA_ARGS__)

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
			LOGE("Assertion failed: %s", message); \
            std::abort(); \
        } \
    } while (false)

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

// Window
enum struct WindowMode { WINDOWED = 0, UNDECORATED = 1, BORDERLESS = 2, FULLSCREEN = 3 };

enum struct WindowCursor : i32
{
	NORMAL = 0x00034001,
	HIDDEN = 0x00034002,
	DISABLED = 0x00034003,
	CAPTURED = 0x00034004
};

// Graphics
enum struct TextureFormat : u32 { R8 = 0, RG8 = 1, RGB8 = 2, RGBA8 = 3 };

enum struct TextureFilter : u32 { NEAREST = 0, LINEAR = 1 };

enum struct TextureWrap : u32 { REPEAT = 0, CLAMP_TO_EDGE = 1 };

enum struct GlTopology : u32
{
	POINTS = BIT(1),
	LINES = BIT(2),
	LINE_LOOP = BIT(3),
	LINE_STRIP = BIT(4),
	TRIANGLES = BIT(5),
	TRIANGLE_STRIP = BIT(6),
	TRIANGLE_FAN = BIT(7)
};

// Net
enum struct NetEvent : u32 { CONNECT = 0, RECEIVE = 1, DISCONNECT = 2, TIMEOUT = 3 };
enum struct NetPacketMode : u32 { RELIABLE = BIT(1), UNSEQUENCED = BIT(2), UNREALIABLE = BIT(4) };

using NetRelayFn = void (*)(bool, u32, u32, u32, u32, u32);

// Script
using ScriptVM = struct WrenVM;
using ScriptHandle = struct WrenHandle;
using ScriptMethodFn = void (*)(ScriptVM* vm);
using ScriptFinalizerFn = void (*)(void* data);

struct ScriptClass
{
	ScriptMethodFn allocate;
	ScriptFinalizerFn finalize;
};

// Application
enum struct FrameOp { NONE, RELOAD };

struct AppConfig
{
	i32 width{ 800 }, height{ 600 };
	const char* title{ nullptr };
	WindowMode windowMode{ WindowMode::WINDOWED };
	i32 msaa{ 8 };
	bool headless{ false };
};

class App
{
private:
	static bool Initialize(const AppConfig& config);
	static void Shutdown();

	static bool NetInitialize(NetRelayFn relayFn);
	static void NetShutdown();

	static void Reload();
	static void NetReload();

	static void Update(f64 dt);
	static void Render();
	static void Netcode();

	static void NetRelay(bool server, u32 client, u32 event, u32 peer, u32 channel, u32 packet);

public:
	static int Run(const AppConfig& config);

	// Util
	static void SetFrameOp(FrameOp op);
	static void Log(bool verbose, const char* file, i32 line, const char* func, u32 color, const char* format, ...);
	static void LogClear();

	static void Wait(u32 ms);
	static bool IsHeadless();

	// Window
	static void WinMode(i32 mode);
	static void WinCursor(i32 cursor);

	static i32 WinWidth();
	static i32 WinHeight();

	static f64 WinMouseX();
	static f64 WinMouseY();
	static bool WinButton(i32 b);
	static bool WinKey(i32 k);

	static i32 WinPadCount();
	static bool WinPadButton(i32 i, i32 b);
	static f32 WinPadAxis(i32 i, i32 a);

	static void WinClose();

	// Graphics
	static u32 GlCreateShader(const char* filepath);
	static void GlDestroyShader(u32 shader);
	static void GlSetShader(u32 shader);

	static u32 GlCreateImage(const char* filepath, bool flipY);
	static void GlDestroyImage(u32 image);
	static i32 GlImageWidth(u32 image);
	static i32 GlImageHeight(u32 image);
	static i32 GlImageChannels(u32 image);

	static u32 GlCreateTexture(
		u32 image, TextureFormat format,
		TextureFilter minFilter, TextureFilter magFilter,
		TextureWrap wrapS, TextureWrap wrapT,
		bool genMipmaps);
	static void GlDestroyTexture(u32 texture);

	static void GlBegin(bool alpha, bool ztest, f32 pointSize, f32 lineWidth);
	static void GlEnd(u32 mode);

	static void GlViewport(u32 x, u32 y, u32 w, u32 h);
	static void GlClear(f32 r, f32 g, f32 b, f32 a, f32 d, f32 s, u32 flags);

	static void GlUniform(const char* name);

	static void GlTex2D(u32 i, u32 texture);

	static void GlFloat(f32 x);

	static void GlVec2F(f32 x, f32 y);
	static void GlVec3F(f32 x, f32 y, f32 z);
	static void GlVec4F(f32 x, f32 y, f32 z, f32 w);

	static void GlMat2x2F(
		f32 m00, f32 m01,
		f32 m10, f32 m11);

	static void GlMat2x3F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12);

	static void GlMat2x4F(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13);

	static void GlMat3x2F(
		f32 m00, f32 m01,
		f32 m10, f32 m11,
		f32 m20, f32 m21);

	static void GlMat3x3F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12,
		f32 m20, f32 m21, f32 m22);

	static void GlMat3x4F(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23);

	static void GlMat4x2F(
		f32 m00, f32 m01,
		f32 m10, f32 m11,
		f32 m20, f32 m21,
		f32 m30, f32 m31);

	static void GlMat4x3F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12,
		f32 m20, f32 m21, f32 m22,
		f32 m30, f32 m31, f32 m32);

	static void GlMat4x4F(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23,
		f32 m30, f32 m31, f32 m32, f32 m33);

	static void GlVertex(
		f32 x, f32 y, f32 z, f32 w,
		u32 c0, u32 c1, u32 i0, u32 i1,
		f32 v0, f32 v1, f32 v2, f32 v3,
		f32 v4, f32 v5, f32 v6, f32 v7);

	// Gui
	static void GuiPushItemWidth(f32 w);
	static void GuiPopItemWidth();
	static void GuiText(const char* text);
	static bool GuiBool(const char* label, bool v);
	static i32 GuiInt(const char* label, i32 i);
	static i32 GuiInt(const char* label, i32 i, i32 min, i32 max);
	static f32 GuiFloat(const char* label, f32 v);
	static f32 GuiFloat(const char* label, f32 v, f32 min, f32 max);
	static void GuiSeparator(const char* label);
	static bool GuiButton(const char* label);
	static void GuiSameLine();
	static f32 GuiContentAvailWidth();
	static f32 GuiContentAvailHeight();
	static bool GuiBeginChild(const char* label, f32 w, f32 h);
	static void GuiEndChild();

	// Net
	static void NetStartServer(const char* ip, u32 port, u32 peerCount, u32 channelLimit);
	static void NetStopServer();

	static u32 NetConnectClient(const char* ip, u32 port, u32 peerCount, u32 channelLimit);
	static void NetDisconnectClient(u32 client);

	static u32 NetMakeUUID();
	static bool NetIsServer();
	static bool NetIsClient(u32 client);

	static u32 NetCreatePacket(const char* id, u32 size);

	static void NetBroadcast(u32 packet, u32 mode);
	static void NetSend(u32 client, u32 packet, u32 mode);

	static void NetPollEvents();

	static bool NetGetBool(u32 packet, u32 offset);
	static u32 NetGetUInt(u32 packet, u32 offset);
	static i32 NetGetInt(u32 packet, u32 offset);
	static f32 NetGetFloat(u32 packet, u32 offset);
	static f64 NetGetDouble(u32 packet, u32 offset);

	static void NetSetBool(u32 packet, u32 offset, bool v);
	static void NetSetUInt(u32 packet, u32 offset, u32 v);
	static void NetSetInt(u32 packet, u32 offset, i32 v);
	static void NetSetFloat(u32 packet, u32 offset, f32 v);
	static void NetSetDouble(u32 packet, u32 offset, f64 v);

	// Script
	static void WrenParseFile(const char* moduleName, const char* filepath);
	static void WrenParseSource(const char* moduleName, const char* source);

	static void WrenBindClass(const char* moduleName, const char* className, ScriptClass scriptClass);
	static void WrenBindMethod(const char* moduleName, const char* className, bool isStatic, const char* signature, ScriptMethodFn scriptMethod);

	static void WrenEnsureSlots(ScriptVM* vm, i32 count);
	static void WrenGetVariable(ScriptVM* vm, const char* moduleName, const char* className, i32 slot);

	static bool WrenGetSlotBool(ScriptVM* vm, i32 slot);
	static u32 WrenGetSlotUInt(ScriptVM* vm, i32 slot);
	static i32 WrenGetSlotInt(ScriptVM* vm, i32 slot);
	static f32 WrenGetSlotFloat(ScriptVM* vm, i32 slot);
	static f64 WrenGetSlotDouble(ScriptVM* vm, i32 slot);
	static const char* WrenGetSlotString(ScriptVM* vm, i32 slot);
	static const char* WrenGetSlotBytes(ScriptVM* vm, i32 slot, i32* length);
	static void* WrenGetSlotObject(ScriptVM* vm, i32 slot);

	template <typename T>
	static T* WrenGetSlotObjectT(ScriptVM* vm, i32 slot)
	{
		return (T*)WrenGetSlotObject(vm, slot);
	}

	static void WrenSetSlotBool(ScriptVM* vm, i32 slot, bool value);
	static void WrenSetSlotUInt(ScriptVM* vm, i32 slot, u32 value);
	static void WrenSetSlotInt(ScriptVM* vm, i32 slot, i32 value);
	static void WrenSetSlotFloat(ScriptVM* vm, i32 slot, f32 value);
	static void WrenSetSlotDouble(ScriptVM* vm, i32 slot, f64 value);
	//static const char* GetSlotString(ScriptVM* vm, i32 slot);
	//static void* GetSlotObject(ScriptVM* vm, i32 slot);
	//static const char* GetSlotBytes(ScriptVM* vm, i32 slot, i32* length);
	static void* WrenSetSlotNewObject(ScriptVM* vm, i32 slot, i32 classSlot, size_t size);

	template <typename T>
	static T* WrenSetSlotNewObjectT(ScriptVM* vm, i32 slot, i32 classSlot)
	{
		return (T*)WrenSetSlotNewObject(vm, slot, classSlot, sizeof(T));
	}
};