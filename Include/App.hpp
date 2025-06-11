#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>

// Macros
#define XSTR(x) #x
#define STR(x) XSTR(x)

#define VERSION_DEV 1
#define VERSION_MINOR 0
#define VERSION_MAJOR 0

#define VERSION_STR STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_DEV)

#ifdef _DEBUG
#define PATH(x) PROJECT_PATH x
#else
#define PATH(x) x
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BIT(n) (1u << (n - 1))

#define LOGD(format, ...) GASandbox::App::Log(true, __FILE__, __LINE__, __func__, 0xFFA0A0A0, format, ##__VA_ARGS__)
#define LOGI(format, ...) GASandbox::App::Log(true, __FILE__, __LINE__, __func__, 0xFFFFFFFF, format, ##__VA_ARGS__)
#define LOGW(format, ...) GASandbox::App::Log(true, __FILE__, __LINE__, __func__, 0xFF00FFFF, format, ##__VA_ARGS__)
#define LOGE(format, ...) GASandbox::App::Log(true, __FILE__, __LINE__, __func__, 0xFF0000FF, format, ##__VA_ARGS__)

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
			LOGE("Assertion failed: %s", message); \
            std::abort(); \
        } \
    } while (false)

namespace GASandbox
{
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

	using size_type = std::size_t;

	using cstring = const char*;
	using string = std::string;

	template <typename T>
	using list = std::vector<T>;

	template <typename K, typename V>
	using hashmap = std::unordered_map<K, V>;

	// File
	struct sFileInfo
	{
		string path{};
		string name{};
		string ext{};

		size_type pathHash{ 0 };
		size_type nameHash{ 0 };
		size_type extHash{ 0 };
	};

	// Window
	enum struct eWinMode { WINDOWED = 0, UNDECORATED = 1, BORDERLESS = 2, FULLSCREEN = 3 };

	enum struct eWinCursor : i32
	{
		NORMAL = 0x00034001,
		HIDDEN = 0x00034002,
		DISABLED = 0x00034003,
		CAPTURED = 0x00034004
	};

	// Graphics
	enum struct eGlClearFlags : u32
	{
		COLOR = BIT(1),
		DEPTH = BIT(2),
		STENCIL = BIT(3),
		ALL = COLOR | DEPTH | STENCIL
	};

	enum struct eGlTextureFormat : u32 { R8 = 0, RG8 = 1, RGB8 = 2, RGBA8 = 3 };

	enum struct eGlTextureFilter : u32 { NEAREST = 0, LINEAR = 1 };

	enum struct eGlTextureWrap : u32 { REPEAT = 0, CLAMP_TO_EDGE = 1 };

	enum struct eGlBufferType { VERTEX_BUFFER, INDEX_BUFFER, UNIFORM_BUFFER, STORAGE_BUFFER };
	enum struct eGlBufferUsage { IMMUTABLE, DEFAULT, DYNAMIC };
	enum struct eGlBufferAccess { NONE, READ, WRITE };

	enum struct eGlTopology : u32
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
	enum struct eNetEvent : u32 { CONNECT = 0, RECEIVE = 1, DISCONNECT = 2, TIMEOUT = 3 };
	enum struct eNetPacketMode : u32 { RELIABLE = BIT(1), UNSEQUENCED = BIT(2), UNREALIABLE = BIT(4) };

	// Code
	using sCodeVM = void*;
	using sCodeHandle = void*;
	using fCodeMethod = void (*)(sCodeVM* vm);
	using fCodeFinalizer = void (*)(void* data);

	struct sCodeClass
	{
		fCodeMethod allocate;
		fCodeFinalizer finalize;
	};

	// Application
	struct sAppConfig
	{
		cstring title{ nullptr };
		i32 width{ 800 }, height{ 600 };
		eWinMode windowMode{ eWinMode::WINDOWED };
		i32 msaa{ 8 };
		bool headless{ false };
	};

	class App
	{
	private:
		static bool Configure(int argc, char** args, sAppConfig& config);

		static bool Initialize(const sAppConfig& config);
		static void Shutdown();

		static bool FileInitialize(const sAppConfig& config);
		static void FileShutdown();

		static const list<sFileInfo>& FileGetIndex();
		static const list<sFileInfo>& FileGetManifest();

		static bool WinInitialize(const sAppConfig& config);
		static void WinShutdown();

		static void WinPollEvents();
		static void WinSwapBuffers();
		static bool WinShouldClose();

		static bool GlInitialize(const sAppConfig& config);
		static void GlShutdown();

		static bool GuiInitialize(const sAppConfig& config);
		static void GuiShutdown();

		static bool GuiWinInitialize();
		static bool GuiGlInitialize();
		static void GuiWinShutdown();
		static void GuiGlShutdown();
		static void GuiWinNewFrame();
		static void GuiGlNewFrame();
		static void GuiGlRender();

		static void GuiSaveStyle();
		static void GuiLoadStyle();
		static void GuiResetStyle();
		static void GuiRender();

		static bool NetInitialize(const sAppConfig& config);
		static void NetShutdown();

		static bool SfxInitialize(const sAppConfig& config);
		static void SfxShutdown();

		static void SfxUpdate(f64 dt);

		static bool SfxWriteSample(f32 sample);
		static bool SfxReadSample(f32& sampleOut);
		static size_type SfxSampleCount();
		static void SfxClearSamples();

		static bool CodeInitialize(const sAppConfig& config);
		static void CodeShutdown();

		static void CodeCollectGarbage();
		static size_type CodeBytesAllocated();

		static bool CodeIsPaused();
		static void CodeTogglePaused();

		static void CodeUpdate(f64 dt);
		static void CodeRender();
		static void CodeNetcode(bool server, u32 client, eNetEvent event, u16 peer, u32 channel, u32 packet);
		static f32 CodeAudio(f64 sampleRate, f64 dt);

		static void Reload(const sAppConfig& config);
		static void WinReload();
		static void GlReload();
		static void GuiReload();
		static void NetReload();
		static void SfxReload();
		static void CodeReload();

		static void Update(f64 dt);
		static void Render();

	public:
		static int Run(int argc, char** args);

		// Util
		static void QueueReload();
		static void Log(bool verbose, cstring file, i32 line, cstring func, u32 color, cstring format, ...);
		static void LogClear();

		static f64 GetTime();
		static void Wait(u32 ms);
		static bool IsHeadless();

		static size_type Hash(cstring str);
		static size_type Hash(const string& str);

		static sFileInfo FileGetInfo(cstring filepath);
		static cstring FilePath(cstring filepath);
		static string FileLoad(cstring filepath);
		static void FileSave(cstring filepath, const string& src);

		// Window
		static void* WinGetProcAddress(cstring procname);

		static void WinMode(eWinMode mode);
		static void WinCursor(eWinCursor cursor);
		static void WinAlwaysOnTop(bool enabled);

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
		static void GlSetRenderTarget(u32 target, u32 depthStencil);
		static void GlReadPixels(u32 x, u32 y, u32 w, u32 h, void* pixelData, u32 target);

		static void GlViewport(i32 x, i32 y, u32 w, u32 h);
		static void GlScissor(i32 x, i32 y, u32 w, u32 h);
		static void GlClear(f32 r, f32 g, f32 b, f32 a, f64 d, i32 s, eGlClearFlags flags);

		static u32 GlLoadShader(cstring filepath);
		static u32 GlCreateShader(cstring source);
		static void GlDestroyShader(u32 shader);
		static void GlSetShader(u32 shader);

		static u32 GlLoadImage(cstring filepath, bool flipY);
		static u32 GlCreateImage(i32 w, i32 h, i32 c, u8* data);
		static void GlDestroyImage(u32 image);
		static i32 GlImageWidth(u32 image);
		static i32 GlImageHeight(u32 image);
		static i32 GlImageChannels(u32 image);
		static u8* GlImageData(u32 image);

		static u32 GlCreateTexture(
			u32 image, eGlTextureFormat format,
			eGlTextureFilter minFilter, eGlTextureFilter magFilter,
			eGlTextureWrap wrapS, eGlTextureWrap wrapT,
			bool genMipmaps);
		static void GlDestroyTexture(u32 texture);

		static u32 GlLoadModel(cstring filepath);
		static void GlDestroyModel(u32 model);
		//static i32 GlImageWidth(u32 image);
		//static i32 GlImageHeight(u32 image);
		//static i32 GlImageChannels(u32 image);

		static u32 GlCreateBuffer(u32 byteStride, eGlBufferType type, eGlBufferUsage usage, eGlBufferAccess access);
		static void GlDestroyBuffer(u32 buffer);
		static void GlBindBuffer(u32 buffer);
		static void GlSubmitBuffer(u32 buffer);

		static void GlAddVertex(
			f32 x, f32 y, f32 z, f32 w,
			u32 c0, u32 c1, u32 i0, u32 i1,
			f32 v0, f32 v1, f32 v2, f32 v3,
			f32 v4, f32 v5, f32 v6, f32 v7);

		static void GlBegin(bool alpha, bool ztest, f32 pointSize, f32 lineWidth);
		static void GlEnd(bool indexed, u32 mode, u32 count);

		static void GlSetUniform(cstring name);

		static void GlSetTex2D(u32 i, u32 texture);

		static void GlSetFloat(f32 x);

		static void GlSetVec2F(f32 x, f32 y);
		static void GlSetVec3F(f32 x, f32 y, f32 z);
		static void GlSetVec4F(f32 x, f32 y, f32 z, f32 w);

		static void GlSetMat2x2F(
			f32 m00, f32 m01,
			f32 m10, f32 m11);

		static void GlSetMat2x3F(
			f32 m00, f32 m01, f32 m02,
			f32 m10, f32 m11, f32 m12);

		static void GlSetMat2x4F(
			f32 m00, f32 m01, f32 m02, f32 m03,
			f32 m10, f32 m11, f32 m12, f32 m13);

		static void GlSetMat3x2F(
			f32 m00, f32 m01,
			f32 m10, f32 m11,
			f32 m20, f32 m21);

		static void GlSetMat3x3F(
			f32 m00, f32 m01, f32 m02,
			f32 m10, f32 m11, f32 m12,
			f32 m20, f32 m21, f32 m22);

		static void GlSetMat3x4F(
			f32 m00, f32 m01, f32 m02, f32 m03,
			f32 m10, f32 m11, f32 m12, f32 m13,
			f32 m20, f32 m21, f32 m22, f32 m23);

		static void GlSetMat4x2F(
			f32 m00, f32 m01,
			f32 m10, f32 m11,
			f32 m20, f32 m21,
			f32 m30, f32 m31);

		static void GlSetMat4x3F(
			f32 m00, f32 m01, f32 m02,
			f32 m10, f32 m11, f32 m12,
			f32 m20, f32 m21, f32 m22,
			f32 m30, f32 m31, f32 m32);

		static void GlSetMat4x4F(
			f32 m00, f32 m01, f32 m02, f32 m03,
			f32 m10, f32 m11, f32 m12, f32 m13,
			f32 m20, f32 m21, f32 m22, f32 m23,
			f32 m30, f32 m31, f32 m32, f32 m33);

		// Gui
		static void GuiPushItemWidth(f32 w);
		static void GuiPopItemWidth();
		static void GuiText(cstring text);
		static void GuiAbsText(cstring text, f32 x, f32 y, u32 c);
		static bool GuiBool(cstring label, bool v);
		static i32 GuiInt(cstring label, i32 i);
		static i32 GuiInt(cstring label, i32 i, i32 min, i32 max);
		static f32 GuiFloat(cstring label, f32 v);
		static f32 GuiFloat(cstring label, f32 v, f32 min, f32 max);
		static void GuiSeparator(cstring label);
		static bool GuiButton(cstring label);
		static void GuiSameLine();
		static f32 GuiContentAvailWidth();
		static f32 GuiContentAvailHeight();
		static bool GuiBeginChild(cstring label, f32 w, f32 h);
		static void GuiEndChild();

		// Audio
		static void SfxBindCallback();
		static void SfxUnbindCallback();
		static bool SfxIsCallbackBound();

		static u32 SfxLoadAudio(cstring filepath);
		static void SfxDestroyAudio(u32 audio);

		static u32 SfxCreateChannel(f32 volume);
		static void SfxDestroyChannel(u32 channel);
		static void SfxSetChannelVolume(u32 channel, f32 volume);

		static void SfxPlay(u32 audio, u32 channel, bool loop);
		static void SfxStop(u32 audio, u32 channel);

		// Net
		static void NetStartServer(cstring ip, u32 port, u32 peerCount, u32 channelLimit);
		static void NetStopServer();

		static u32 NetConnectClient(cstring ip, u32 port, u32 peerCount, u32 channelLimit);
		static void NetDisconnectClient(u32 client);

		static u32 NetMakeUUID();
		static bool NetIsServer();
		static bool NetIsClient(u32 client);

		static u32 NetCreatePacket(u32 id, u32 size);
		static u32 NetPacketId(u32 packet);

		static void NetBroadcast(u32 packet, eNetPacketMode mode);
		static void NetSend(u32 client, u32 packet, eNetPacketMode mode);

		static void NetPollEvents();

		static bool NetGetBool(u32 packet, u32 offset);
		static u32 NetGetUInt(u32 packet, u32 offset);
		static i32 NetGetInt(u32 packet, u32 offset);
		static f32 NetGetFloat(u32 packet, u32 offset);
		static f64 NetGetDouble(u32 packet, u32 offset);
		static cstring NetGetString(u32 packet, u32 offset);

		static void NetSetBool(u32 packet, u32 offset, bool v);
		static void NetSetUInt(u32 packet, u32 offset, u32 v);
		static void NetSetInt(u32 packet, u32 offset, i32 v);
		static void NetSetFloat(u32 packet, u32 offset, f32 v);
		static void NetSetDouble(u32 packet, u32 offset, f64 v);
		static void NetSetString(u32 packet, u32 offset, cstring v);

		// Code
		static void CodeParseFile(cstring moduleName, cstring filepath);
		static void CodeParseSource(cstring moduleName, cstring source);

		static void CodeBindClass(cstring moduleName, cstring className, sCodeClass scriptClass);
		static void CodeBindMethod(cstring moduleName, cstring className, bool isStatic, cstring signature, fCodeMethod scriptMethod);

		static void CodeEnsureSlots(sCodeVM vm, i32 count);
		static void CodeGetVariable(sCodeVM vm, cstring moduleName, cstring className, i32 slot);

		static bool CodeGetSlotBool(sCodeVM vm, i32 slot);
		static u32 CodeGetSlotUInt(sCodeVM vm, i32 slot);
		static i32 CodeGetSlotInt(sCodeVM vm, i32 slot);
		static f32 CodeGetSlotFloat(sCodeVM vm, i32 slot);
		static f64 CodeGetSlotDouble(sCodeVM vm, i32 slot);
		static cstring CodeGetSlotString(sCodeVM vm, i32 slot);
		static cstring CodeGetSlotBytes(sCodeVM vm, i32 slot, i32* length);
		static void* CodeGetSlotObject(sCodeVM vm, i32 slot);

		template <typename T>
		static T* CodeGetSlotObjectT(sCodeVM vm, i32 slot)
		{
			return (T*)CodeGetSlotObject(vm, slot);
		}

		static void CodeSetSlotBool(sCodeVM vm, i32 slot, bool value);
		static void CodeSetSlotUInt(sCodeVM vm, i32 slot, u32 value);
		static void CodeSetSlotInt(sCodeVM vm, i32 slot, i32 value);
		static void CodeSetSlotFloat(sCodeVM vm, i32 slot, f32 value);
		static void CodeSetSlotDouble(sCodeVM vm, i32 slot, f64 value);
		static void CodeSetSlotString(sCodeVM vm, i32 slot, cstring str);
		//static void* GetSlotObject(sScriptVM* vm, i32 slot);
		//static CString GetSlotBytes(sScriptVM* vm, i32 slot, i32* length);
		static void* CodeSetSlotNewObject(sCodeVM vm, i32 slot, i32 classSlot, size_type size);

		template <typename T>
		static T* CodeSetSlotNewObjectT(sCodeVM vm, i32 slot, i32 classSlot)
		{
			return (T*)CodeSetSlotNewObject(vm, slot, classSlot, sizeof(T));
		}
	};
}