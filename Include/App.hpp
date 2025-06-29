#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <type_traits>
#include <tuple>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>

// Macros
#ifndef _DEBUG
  #ifndef NDEBUG
    #define _DEBUG
  #endif
#endif

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
	// Aliases
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

	template <typename... T>
	using tuple = std::tuple<T...>;

	template <typename T, size_type N>
	using array = std::array<T, N>;

	template <typename T>
	using list = std::vector<T>;

	template <typename K, typename V>
	using hashmap = std::unordered_map<K, V>;

	// Traits
	namespace meta
	{
		template<typename...>
		using void_type = void;

		using false_type = std::false_type;
		using true_type = std::true_type;

		template <bool B, typename T = void>
		using enable_if = std::enable_if<B, T>;

		template <bool B, typename T = void>
		using enable_if_t = typename enable_if<B, T>::type;

		template <typename T>
		using is_enum = std::is_enum<T>;

		template <typename T, typename Enable = void>
		struct underlying_type;

		template <typename T>
		struct underlying_type<T, enable_if_t<is_enum<T>::value>>
		{
			using type = typename std::underlying_type<T>::type;
		};

		template <typename T>
		struct underlying_type<T, enable_if_t<!is_enum<T>::value>>
		{
			using type = void;
		};

		template <typename T>
		using underlying_type_t = typename underlying_type<T>::type;

		template<typename T, T... Ints>
		struct integer_sequence
		{
			using value_type = T;
			static constexpr size_type size() { return sizeof...(Ints); }
		};

		template<size_type... Ints>
		using index_sequence = integer_sequence<size_type, Ints...>;

		template<typename T, size_type N, T... Is>
		struct make_integer_sequence : make_integer_sequence<T, N - 1, N - 1, Is...> {};

		template<typename T, T... Is>
		struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...> {};

		template<size_type N>
		using make_index_sequence = make_integer_sequence<size_type, N>;

		template<typename... T>
		using index_sequence_for = make_index_sequence<sizeof...(T)>;

		template<class T>
		using remove_reference_t = typename std::remove_reference<T>::type;

		template<size_type I, class T>
		using tuple_element_t = typename std::tuple_element<I, T>::type;

		template< class T >
		using decay_t = typename std::decay<T>::type;

		template<class F>
		struct function_traits;

		// function pointer
		template<class R, class... Args>
		struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)> {};

		// member function pointer
		template<class C, class R, class... Args>
		struct function_traits<R(C::*)(Args...)> : public function_traits<R(Args...)> {};

		// const member function pointer
		template<class C, class R, class... Args>
		struct function_traits<R(C::*)(Args...) const> : public function_traits<R(Args...)> {};

		template<class R, class... Args>
		struct function_traits<R(Args...)>
		{
			using return_type = R;

			static constexpr size_type arity = sizeof...(Args);

			template <size_type N>
			struct argument
			{
				static_assert(N < arity, "Invalid parameter index.");
				using type = tuple_element_t<N, tuple<Args...>>;
			};

			template<size_type N>
			using argument_t = typename argument<N>::type;
		};

		template<typename... Args>
		struct parameter_pack_traits
		{
			static constexpr size_type count = sizeof...(Args);

			template<size_type N>
			struct parameter
			{
				static_assert(N < count, "Invalid parameter index");
				using type = tuple_element_t<N, tuple<Args...>>;
			};

			template<size_type N>
			using parameter_t = typename parameter<N>::type;
		};

		template<typename T>
		struct identity { using type = T; };

		template<typename T>
		T&& forward(typename identity<T>::type&& param)
		{
			return static_cast<typename identity<T>::type&&>(param);
		}

		template<typename Dst>
		Dst implicit_cast(typename identity<Dst>::type t) { return t; }
	}

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
	typedef void (*fWinGlProc)(void);

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

	struct sCodeModuleSource
	{
		char moduleName[64]{};
		cstring moduleSource{ nullptr };
	};

	struct sCodeClassInfo
	{
		char moduleName[64]{};
		char className[64]{};
		u32 typeId{ 0 };
		sCodeHandle handle{ nullptr };
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
		static bool Configure(i32 argc, char** args, sAppConfig& config);

		static bool Initialize(const sAppConfig& config);
		static void Shutdown();

		// File
		static bool FileInitialize(const sAppConfig& config);
		static void FileShutdown();

		static const list<sFileInfo>& FileGetIndex();
		static const list<sFileInfo>& FileGetManifest();

		// Window
		static bool WinInitialize(const sAppConfig& config);
		static void WinShutdown();

		static void WinPollEvents();
		static void WinSwapBuffers();
		static bool WinShouldClose();

		// Graphics
		static bool GlInitialize(const sAppConfig& config);
		static void GlShutdown();

		// Gui
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

		// Net
		static bool NetInitialize(const sAppConfig& config);
		static void NetShutdown();

		// Audio
		static bool SfxInitialize(const sAppConfig& config);
		static void SfxShutdown();

		static void SfxUpdate(f64 dt);

		static bool SfxWriteSample(f32 sample);
		static bool SfxReadSample(f32& sampleOut);
		static size_type SfxSampleCount();
		static void SfxClearSamples();

		// Code
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

		// Application
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
		static i32 Run(i32 argc, char** args);

		// Util
		static void QueueReload();
		static void Log(bool verbose, cstring file, i32 line, cstring func, u32 color, cstring format, ...);
		static void LogClear();

		static f64 GetTime();
		static void Wait(u32 ms);
		static void Breakpoint(bool cond);
		static bool IsHeadless();

		static size_type Hash(cstring str);
		static size_type Hash(const string& str);

		// File
		static sFileInfo FileGetInfo(cstring filepath);
		static cstring FilePath(cstring filepath);
		static string FileLoad(cstring filepath);
		static void FileSave(cstring filepath, const string& src);

		// Window
		static fWinGlProc WinGetProcAddress(cstring procname);

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

		static void CodeBeginModule(cstring moduleName);
		static cstring CodeGetCurrentModule();
		static void CodeEndModule();

		static void CodeBeginClass(cstring className, u32 typeId);
		static cstring CodeGetCurrentClass();
		static void CodeEndClass();

		static const sCodeClassInfo& GetClassInfo(u32 typeId);
		static void CodeSetClass(sCodeVM vm, i32 slot, u32 typeId);

		template<typename T>
		void CodeSetClass(sCodeVM vm, i32 slot)
		{
			SetClass(vm, slot, Type<T>::Id());
		}

		template <typename T>
		void CodeBeginClass(cstring className);

		template <typename T, typename U = std::underlying_type_t<T>>
		void CodeBeginEnum(cstring className);

		static void CodeBindConstructor(cstring signature, fCodeMethod func);
		static void CodeBindDestructor(fCodeFinalizer func);

		template<typename T, typename... Args, size_type... Index>
		void CodeConstruct(sCodeVM vm, void* memory, meta::index_sequence<Index...>);

		template<typename T, typename... Args>
		void CodeAllocate(sCodeVM vm);

		template<typename T>
		void CodeFinalize(void* data);

		template <typename T, typename... Args>
		void CodeBindConstructor(cstring signature);

		template <typename T>
		void CodeBindDestructor();

		static void CodeBindFunction(bool isStatic, cstring signature, fCodeMethod func);

		template<typename Fn, Fn F>
		void CodeBindFunction(bool isStatic, cstring signature);

		template<typename T, typename U, U T::* Field>
		void CodeBindGetter(cstring signature);

		template<typename T, typename U, U T::* Field>
		void CodeBindSetter(cstring signature);

		template <typename T, T Val>
		void CodeBindEnumVal(cstring signature);

		// Getters
		static bool CodeGetSlotBool(sCodeVM vm, i32 slot);
		static u8 CodeGetSlotU8(sCodeVM vm, i32 slot);
		static u16 CodeGetSlotU16(sCodeVM vm, i32 slot);
		static u32 CodeGetSlotU32(sCodeVM vm, i32 slot);
		static u64 CodeGetSlotU64(sCodeVM vm, i32 slot);
		static i8 CodeGetSlotI8(sCodeVM vm, i32 slot);
		static i16 CodeGetSlotI16(sCodeVM vm, i32 slot);
		static i32 CodeGetSlotI32(sCodeVM vm, i32 slot);
		static i64 CodeGetSlotI64(sCodeVM vm, i32 slot);
		static f32 CodeGetSlotF32(sCodeVM vm, i32 slot);
		static f64 CodeGetSlotF64(sCodeVM vm, i32 slot);
		static cstring CodeGetSlotString(sCodeVM vm, i32 slot);
		static void* CodeGetSlotObject(sCodeVM vm, i32 slot);
		static sCodeHandle CodeGetSlotHandle(sCodeVM vm, i32 slot);

		static i32 CodeGetListCount(sCodeVM vm, i32 slot);
		static void CodeGetListElement(sCodeVM vm, i32 listSlot, i32 index, i32 elementSlot);

		template <typename T>
		T CodeGetSlotArg(sCodeVM vm, i32 slot);

		// Setters
		static void CodeSetSlotBool(sCodeVM vm, i32 slot, bool value);
		static void CodeSetSlotU8(sCodeVM vm, i32 slot, u8 value);
		static void CodeSetSlotU16(sCodeVM vm, i32 slot, u16 value);
		static void CodeSetSlotU32(sCodeVM vm, i32 slot, u32 value);
		static void CodeSetSlotU64(sCodeVM vm, i32 slot, u64 value);
		static void CodeSetSlotI8(sCodeVM vm, i32 slot, i8 value);
		static void CodeSetSlotI16(sCodeVM vm, i32 slot, i16 value);
		static void CodeSetSlotI32(sCodeVM vm, i32 slot, i32 value);
		static void CodeSetSlotI64(sCodeVM vm, i32 slot, i64 value);
		static void CodeSetSlotF32(sCodeVM vm, i32 slot, f32 value);
		static void CodeSetSlotF64(sCodeVM vm, i32 slot, f64 value);
		static void CodeSetSlotString(sCodeVM vm, i32 slot, cstring text);
		static void* CodeSetSlotNewObject(sCodeVM vm, i32 slot, i32 classSlot, size_type size);
		static void CodeSetSlotHandle(sCodeVM vm, i32 slot, sCodeHandle handle);

		static void CodeSetSlotNewList(sCodeVM vm, i32 slot);
		static void CodeSetListElement(sCodeVM vm, i32 listSlot, i32 index, i32 elementSlot);
		static void CodeInsertInList(sCodeVM vm, i32 listSlot, i32 index, i32 elementSlot);

		template <typename T>
		void CodeSetSlotArg(sCodeVM vm, i32 slot, T&& val);

		// Script handles
		static sCodeHandle CodeMakeCallHandle(sCodeVM vm, cstring signature);
		static sCodeHandle CodeMakeClassHandle(sCodeVM vm, cstring moduleName, cstring className);
		static void CodeReleaseHandle(sCodeVM vm, sCodeHandle handle);
		static u64 CodeGetHandleValue(sCodeHandle handle);

		static void CodeEnsureSlots(sCodeVM vm, i32 numSlots);
		static void CodeCallFunction(sCodeVM vm, sCodeHandle handle);
	};

	// Code template API inspired by: https://github.com/Nelarius/wrenpp

	template<typename T>
	struct CodeArg;

#define CODE_PRIM_ARG(Type, GetFunc, SetFunc)																								\
	template<> struct CodeArg<Type>																											\
	{																																		\
		static Type Get(sCodeVM vm, i32 slot) { return App::Code##GetFunc(vm, slot); }														\
		static void Set(sCodeVM vm, i32 slot, Type val) { App::Code##SetFunc(vm, slot, val); }												\
	};																																		\
	template<> struct CodeArg<Type&> { static void Set(sCodeVM vm, i32 slot, Type val) { CodeArg<Type>::Set(vm, slot, val); } };			\
	template<> struct CodeArg<const Type&> { static void Set(sCodeVM vm, i32 slot, Type val) { CodeArg<Type>::Set(vm, slot, val); } };

	// Basic types
	CODE_PRIM_ARG(bool, GetSlotBool, SetSlotBool)
	CODE_PRIM_ARG(u8, GetSlotU8, SetSlotU8)
	CODE_PRIM_ARG(u16, GetSlotU16, SetSlotU16)
	CODE_PRIM_ARG(u32, GetSlotU32, SetSlotU32)
	CODE_PRIM_ARG(u64, GetSlotU64, SetSlotU64)
	CODE_PRIM_ARG(i8, GetSlotI8, SetSlotI8)
	CODE_PRIM_ARG(i16, GetSlotI16, SetSlotI16)
	CODE_PRIM_ARG(i32, GetSlotI32, SetSlotI32)
	CODE_PRIM_ARG(i64, GetSlotI64, SetSlotI64)
	CODE_PRIM_ARG(f32, GetSlotF32, SetSlotF32)
	CODE_PRIM_ARG(f64, GetSlotF64, SetSlotF64)

	// String support
	CODE_PRIM_ARG(cstring, GetSlotString, SetSlotString)

	/*template<size_type N>
	struct CodeArg<CString<N>>
	{
		static CString<N> Get(sCodeVM vm, i32 slot) { return App::CodeGetSlotString(vm, slot); }
		static void Set(sCodeVM vm, i32 slot, CString<N> val) { App::CodeSetSlotString(vm, slot, val.c_str()); }
	};
	template<size_type N>
	struct CodeArg<CString<N>&>
	{
		static void Set(sCodeVM vm, i32 slot, CString<N> val) { CodeArg<cstring>::Set(vm, slot, val); }
	};
	template<size_type N>
	struct CodeArg<const CString<N>&>
	{
		static void Set(sCodeVM vm, i32 slot, CString<N> val) { CodeArg<cstring>::Set(vm, slot, val); }
	};*/

	// Containers support
	//template<class T, size_type Size>
	//struct CodeArgArray
	//{
	//	static Array<T, Size> Get(sCodeVM vm, i32 slot)
	//	{
	//		Array<T, Size> val;
	//		i32 count = CodeArg<void*>::CodeGetListCount(vm, slot);
	//		ENGINE_ASSERT(Size == count, "List does not match size of array!");
	//		for (i32 i; i < count; i++)
	//		{
	//			CodeArg<void*>::CodeGetListElement(vm, slot, i, 0);
	//			val[i] = CodeArg<T>::Get(vm, 0);
	//		}
	//		return val;
	//	}
	//
	//	//static void Set(sCodeVM vm, i32 slot, const Array<f32, Size>& val)
	//	//{
	//	//	new (CodeArg<void*>::Set(vm, slot, slot, sizeof(CodeObjPtr<T>))) CodeObjPtr<T>(val);
	//	//}
	//};
	//
	//template<class T, size_type Size>
	//struct CodeArg<Array<T, Size>>
	//{
	//	static Array<T, Size> Get(sCodeVM vm, i32 slot) { return CodeArgArray<T, Size>::Get(vm, slot); }
	//};
	//
	//template<class T, size_type Size>
	//struct CodeArg<Array<T, Size>&>
	//{
	//	static Array<T, Size> Get(sCodeVM vm, i32 slot) { return CodeArgArray<T, Size>::Get(vm, slot); }
	//};
	//
	//template <class T, size_type Size>
	//struct CodeArg<const Array<T, Size>&>
	//{
	//	static Array<T, Size> Get(sCodeVM vm, i32 slot) { return CodeArgArray<T, Size>::Get(vm, slot); }
	//};

	template<class T>
	struct CodeArgList
	{
		static list<T> Get(sCodeVM vm, i32 slot)
		{
			App::CodeEnsureSlots(vm, slot + 2);

			list<T> val;
			i32 count = App::CodeGetListCount(vm, slot);
			val.resize(count);
			for (i32 i; i < count; i++)
			{
				App::CodeGetListElement(vm, slot, i, slot + 1);
				val[i] = CodeArg<T>::Get(vm, slot + 1);
			}
			return val;
		}

		static void Set(sCodeVM vm, i32 slot, const list<T>& val)
		{
			App::CodeSetSlotNewList(vm, slot);
			for (const auto& v : val)
			{
				CodeArg<T>::Set(vm, slot + 1, v);
				App::CodeInsertInList(vm, slot, -1, slot + 1);
			}
		}
	};

	template<class T>
	struct CodeArg<list<T>>
	{
		static list<T> Get(sCodeVM vm, i32 slot) { return CodeArgList<T>::Get(vm, slot); }
		static void Set(sCodeVM vm, i32 slot, const list<T>& val) { return CodeArgList<T>::Set(vm, slot, val); }
	};

	template<class T>
	struct CodeArg<list<T>&>
	{
		static list<T> Get(sCodeVM vm, i32 slot) { return CodeArgList<T>::Get(vm, slot); }
		static void Set(sCodeVM vm, i32 slot, const list<T>& val) { return CodeArgList<T>::Set(vm, slot, val); }
	};

	template <class T>
	struct CodeArg<const list<T>&>
	{
		static list<T> Get(sCodeVM vm, i32 slot) { return CodeArgList<T>::Get(vm, slot); }
		static void Set(sCodeVM vm, i32 slot, const list<T>& val) { return CodeArgList<T>::Set(vm, slot, val); }
	};

	// Base class for script objects
	struct CodeObj
	{
		virtual ~CodeObj() = default;
		virtual void* Ptr() = 0;
	};

	// Values live within VM
	template<typename T>
	struct CodeObjVal : public CodeObj
	{
		explicit CodeObjVal() : obj() {}
		static ~CodeObjVal() { static_cast<T*>(Ptr())->~T(); }
		void* Ptr() override { return &obj; }
		typename std::aligned_storage<sizeof(T), alignof(T)>::type obj;
	};

	// Pointers live within host
	template<typename T>
	struct CodeObjPtr : public CodeObj
	{
		explicit CodeObjPtr(T* obj) : obj{ obj } {}
		static ~CodeObjPtr() = default;
		void* Ptr() override { return obj; }
		T* obj;
	};

	template<typename T>
	struct CodeArg
	{
		static T Get(sCodeVM vm, i32 slot)
		{
			CodeObj* obj = static_cast<CodeObj*>(App::CodeGetSlotObject(vm, slot));
			return *static_cast<T*>(obj->Ptr());
		}

		static void Set(sCodeVM vm, i32 slot, T val)
		{
			App::CodeSetClass<T>(vm, slot);
			CodeObj* obj = new (App::CodeSetSlotNewObject(vm, slot, slot, sizeof(CodeObjVal<T>))) CodeObjVal<T>();
			new (obj->Ptr()) T(val);
		}
	};

	template<typename T>
	struct CodeArg<T&>
	{
		static T& Get(sCodeVM vm, i32 slot)
		{
			CodeObj* obj = static_cast<CodeObj*>(App::CodeGetSlotObject(vm, slot));
			return *static_cast<T*>(obj->Ptr());
		}

		static void Set(sCodeVM vm, i32 slot, T& val)
		{
			App::CodeSetClass<T>(vm, slot);
			new (App::CodeSetSlotNewObject(vm, slot, slot, sizeof(CodeObjPtr<T>))) CodeObjPtr<T>(&val);
		}
	};

	template<typename T>
	struct CodeArg<const T&>
	{
		static const T& Get(sCodeVM vm, i32 slot)
		{
			CodeObj* obj = static_cast<CodeObj*>(App::CodeGetSlotObject(vm, slot));
			return *static_cast<T*>(obj->Ptr());
		}

		static void Set(sCodeVM vm, i32 slot, const T& val)
		{
			App::CodeSetClass<T>(vm, slot);
			new (App::CodeSetSlotNewObject(vm, slot, slot, sizeof(CodeObjPtr<T>))) CodeObjPtr<T>(const_cast<T*>(&val));
		}
	};

	template<typename T>
	struct CodeArg<T*>
	{
		static T* Get(sCodeVM vm, i32 slot)
		{
			CodeObj* obj = static_cast<CodeObj*>(App::CodeGetSlotObject(vm, slot));
			return static_cast<T*>(obj->Ptr());
		}

		static void Set(sCodeVM vm, i32 slot, T* val)
		{
			App::CodeSetClass<T>(vm, slot);
			new (App::CodeSetSlotNewObject(vm, slot, slot, sizeof(CodeObjPtr<T>))) CodeObjPtr<T>(val);
		}
	};

	template<typename T>
	struct CodeArg<const T*>
	{
		static const T* Get(sCodeVM vm, i32 slot)
		{
			CodeObj* obj = static_cast<CodeObj*>(App::CodeGetSlotObject(vm, slot));
			return static_cast<T*>(obj->Ptr());
		}

		static void Set(sCodeVM vm, i32 slot, const T* val)
		{
			App::CodeSetClass<T>(vm, slot);
			new (App::CodeSetSlotNewObject(vm, slot, slot, sizeof(CodeObjPtr<T>))) CodeObjPtr<T>(const_cast<T*>(val));
		}
	};

	// Invoke meta programming
	class CodeInvokeImpl
	{
	private:
		template<bool RetVoid>
		friend class CodeInvoke;

		// function pointer
		template<typename R, typename... Args>
		static R CallWithArguments(sCodeVM vm, R(*F)(Args...))
		{
			constexpr size_type arity = meta::function_traits<decltype(F)>::arity;
			App::CodeEnsureSlots(vm, arity);
			return CallImpl(vm, F, meta::make_index_sequence<arity>{});
		}

		// member function pointer
		template<typename R, typename C, typename... Args>
		static R CallWithArguments(sCodeVM vm, R(C::* F)(Args...))
		{
			constexpr size_type arity = meta::function_traits<decltype(F)>::arity;
			App::CodeEnsureSlots(vm, arity);
			return CallImpl(vm, F, meta::make_index_sequence<arity>{});
		}

		// const member function pointer
		template<typename R, typename C, typename... Args>
		static R CallWithArguments(sCodeVM vm, R(C::* F)(Args...) const)
		{
			constexpr size_type arity = meta::function_traits<decltype(F)>::arity;
			App::CodeEnsureSlots(vm, arity);
			return CallImpl(vm, F, meta::make_index_sequence<arity>{});
		}

		// function pointer
		template<typename R, typename... Args, size_type... index>
		static R CallImpl(sCodeVM vm, R(*F)(Args...), meta::index_sequence<index...>)
		{
			using Traits = meta::function_traits<meta::remove_reference_t<decltype(F)>>;
			return F(CodeArg<typename Traits::template argument_t<index>>::Get(vm, index + 1)...);
		}

		// member function pointer
		template<typename R, typename C, typename... Args, size_type... index>
		static R CallImpl(sCodeVM vm, R(C::* F)(Args...), meta::index_sequence<index...>)
		{
			using Traits = meta::function_traits<decltype(F)>;
			CodeObj* obj = static_cast<CodeObj*>(App::CodeGetSlotObject(vm, 0));
			C* ptr = static_cast<C*>(obj->Ptr());
			return (ptr->*F)(CodeArg<typename Traits::template argument_t<index>>::Get(vm, index + 1)...);
		}

		// const member function pointer
		template<typename R, typename C, typename... Args, size_type... index>
		static R CallImpl(sCodeVM vm, R(C::* F)(Args...) const, meta::index_sequence<index...>)
		{
			using Traits = meta::function_traits<decltype(F)>;
			CodeObj* obj = static_cast<CodeObj*>(App::CodeGetSlotObject(vm, 0));
			C* ptr = static_cast<C*>(obj->Ptr());
			return (ptr->*F)(CodeArg<typename Traits::template argument_t<index>>::Get(vm, index + 1)...);
		}
	};

	template<bool RetVoid>
	class CodeInvoke;

	template<>
	class CodeInvoke<true>
	{
	private:
		template<typename Signature, Signature>
		friend class CodeInvokeWrapper;

		// function pointer
		template<typename R, typename... Args>
		static void Call(sCodeVM vm, R(*F)(Args...))
		{
			CodeInvokeImpl::CallWithArguments(vm, std::forward<R(*)(Args...)>(F));
		}

		// member function pointer
		template<typename R, typename C, typename... Args>
		static void Call(sCodeVM vm, R(C::* F)(Args...))
		{
			CodeInvokeImpl::CallWithArguments(vm, std::forward<R(C::*)(Args...)>(F));
		}

		// const member function pointer
		template<typename R, typename C, typename... Args>
		static void Call(sCodeVM vm, R(C::* F)(Args...) const)
		{
			CodeInvokeImpl::CallWithArguments(vm, std::forward<R(C::*)(Args...) const>(F));
		}
	};

	template<>
	class CodeInvoke<false>
	{
	private:
		template<typename Signature, Signature>
		friend class CodeInvokeWrapper;

		// function pointer
		template<typename R, typename... Args>
		static void Call(sCodeVM vm, R(*F)(Args...))
		{
			using ReturnType = typename meta::function_traits<meta::remove_reference_t<decltype(F)>>::return_type;
			CodeArg<ReturnType>::Set(vm, 0, CodeInvokeImpl::CallWithArguments(vm, std::forward<R(*)(Args...)>(F)));
		}

		// member function pointer
		template<typename R, typename C, typename... Args>
		static void Call(sCodeVM vm, R(C::* F)(Args...))
		{
			using ReturnType = typename meta::function_traits<meta::remove_reference_t<decltype(F)>>::return_type;
			CodeArg<ReturnType>::Set(vm, 0, CodeInvokeImpl::CallWithArguments(vm, std::forward<R(C::*)(Args...)>(F)));
		}

		// const member function pointer
		template<typename R, typename C, typename... Args>
		static void Call(sCodeVM vm, R(C::* F)(Args...) const)
		{
			using ReturnType = typename meta::function_traits<meta::remove_reference_t<decltype(F)>>::return_type;
			CodeArg<ReturnType>::Set(vm, 0, CodeInvokeImpl::CallWithArguments(vm, std::forward<R(C::*)(Args...) const>(F)));
		}
	};

	template<typename Signature, Signature>
	class CodeInvokeWrapper;

	template<typename R, typename... Args, R(*F)(Args...)>
	class CodeInvokeWrapper<R(*)(Args...), F>
	{
	private:
		friend class App;

		static void Call(sCodeVM vm)
		{
			CodeInvoke<std::is_void<R>::value>::Call(vm, F);
		}
	};

	template<typename R, typename C, typename... Args, R(C::* F)(Args...)>
	class CodeInvokeWrapper<R(C::*)(Args...), F>
	{
	private:
		friend class App;

		static void Call(sCodeVM vm)
		{
			CodeInvoke<std::is_void<R>::value>::Call(vm, F);
		}
	};

	template<typename R, typename C, typename... Args, R(C::* F)(Args...) const>
	class CodeInvokeWrapper<R(C::*)(Args...) const, F>
	{
	private:
		friend class App;

		static void Call(sCodeVM vm)
		{
			CodeInvoke<std::is_void<R>::value>::Call(vm, F);
		}
	};

	template<typename T, typename U, U T::* Field>
	class ScriptProperty
	{
	private:
		friend class App;

		static void Get(sCodeVM vm)
		{
			T* obj = CodeArg<T*>::Get(vm, 0);
			CodeArg<U>::Set(vm, 0, obj->*Field);
		}

		static void Set(sCodeVM vm)
		{
			T* obj = CodeArg<T*>::Get(vm, 0);
			obj->*Field = CodeArg<U>::Get(vm, 1);
		}
	};

	template<typename T, T Val>
	class ScriptEnumVal
	{
	private:
		friend class App;

		static void Get(sCodeVM vm)
		{
			CodeArg<T>::Set(vm, 0, Val);
		}
	};

	// Script implementation
	template<typename T>
	void App::CodeBeginClass(cstring className)
	{
		CodeBeginClass(className, Type<T>::Id());
	}

	template <typename T, typename U>
	void App::CodeBeginEnum(cstring className)
	{
		CodeBeginClass(className, Type<T>::Id());

		CodeBindConstructor("",
			[](sCodeVM vm)
			{
				void* memory = App::CodeSetSlotNewObject(vm, 0, 0, sizeof(CodeObjVal<T>));
				App::CodeEnsureSlots(vm, 1);
				CodeObjVal<T>* obj = new (memory) CodeObjVal<T>{};
				new (obj->Ptr()) T(Enum::as_type<T>(CodeArg<U>::Get(vm, 1)));
			});

		CodeBindDestructor(
			[](void* data)
			{
				CodeObj* obj = static_cast<CodeObj*>(data);
				obj->~CodeObj();
			});
	}

	template<typename T, typename... Args, size_type... Index>
	void App::CodeConstruct(sCodeVM vm, void* memory, meta::index_sequence<Index...>)
	{
		using Traits = meta::parameter_pack_traits<Args...>;
		App::CodeEnsureSlots(vm, Traits::count);
		CodeObjVal<T>* obj = new (memory) CodeObjVal<T>{};
		new (obj->Ptr()) T{ CodeArg<typename Traits::template parameter_t<Index>>::Get(vm, Index + 1)... };
	}

	template<typename T, typename... Args>
	void App::CodeAllocate(sCodeVM vm)
	{
		void* memory = App::CodeSetSlotNewObject(vm, 0, 0, sizeof(CodeObjVal<T>));
		CodeConstruct<T, Args...>(vm, memory, meta::make_index_sequence<meta::parameter_pack_traits<Args...>::count>{});
	}

	template<typename T>
	void App::CodeFinalize(void* data)
	{
		CodeObj* obj = static_cast<CodeObj*>(data);
		obj->~CodeObj();
	}

	template<typename T, typename... Args>
	void App::CodeBindConstructor(cstring signature)
	{
		fCodeMethod allocate = &Allocate<T, Args...>;
		CodeBindConstructor(signature, allocate);
	}

	template<typename T>
	void App::CodeBindDestructor()
	{
		fCodeFinalizer finalize = &Finalize<T>;
		CodeBindDestructor(finalize);
	}

	template<typename Fn, Fn F>
	void App::CodeBindFunction(bool isStatic, cstring signature)
	{
		CodeBindFunction(isStatic, signature, CodeInvokeWrapper<decltype(F), F>::Call);
	}

	template<typename T, typename U, U T::* Field>
	void App::CodeBindGetter(cstring signature)
	{
		CodeBindFunction(false, signature, ScriptProperty<T, U, Field>::Get);
	}

	template<typename T, typename U, U T::* Field>
	void App::CodeBindSetter(cstring signature)
	{
		CodeBindFunction(false, signature, ScriptProperty<T, U, Field>::Set);
	}

	template <typename T, T Val>
	void App::CodeBindEnumVal(cstring signature)
	{
		CodeBindFunction(true, signature, ScriptEnumVal<T, Val>::Get);
	}

	template <typename T>
	T App::CodeGetSlotArg(sCodeVM vm, i32 slot)
	{
		return CodeArg<T>::Get(vm, slot);
	}

	template <typename T>
	void App::CodeSetSlotArg(sCodeVM vm, i32 slot, T&& val)
	{
		CodeArg<T>::Set(vm, slot, val);
	}
}