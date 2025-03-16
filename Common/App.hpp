#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>

// Util
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

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
using AppConfigureFn = struct AppConfig(*)();
using AppBindApiFn = void (*)();

enum struct WindowMode { WINDOW, BORDERLESS, FULLSCREEN };
enum struct FrameOp { NONE, RELOAD, NEXT, PREV };

struct AppConfig
{
	i32 width{ 800 }, height{ 600 };
	const char* title{ nullptr };
	WindowMode windowMode{ WindowMode::WINDOW };

	i32 msaa{ 8 };

	const char** paths{ nullptr };
	std::size_t pathCount{ 0 };

	AppBindApiFn bindApiFn{ nullptr };
};

class App
{
private:
	static bool Initialize(const AppConfig& config);
	static void Shutdown();

	static void Prev(const AppConfig& config);
	static void Next(const AppConfig& config);

	static void Reload(const AppConfig& config);

	static void Update(f64 dt);
	static void Render();

public:
	static int Run(AppConfigureFn configFn);

	static void SetFrameOp(FrameOp op);

	// Util
	static void Log(bool verbose, const char* file, i32 line, const char* func, u32 color, const char* format, ...);
	static bool DebugBool(const char* label, bool v);
	static i32 DebugInt(const char* label, i32 i);
	static i32 DebugInt(const char* label, i32 i, i32 min, i32 max);
	static f32 DebugFloat(const char* label, f32 v);
	static void DebugSeparator(const char* label);
	static bool DebugButton(const char* label);

	static f32 GetFps();
	static f32 GetAvgFrameTime();

	// Window
	static void SetWindowMode(WindowMode windowMode);

	static i32 GetWidth();
	static i32 GetHeight();

	static f64 GetMouseX();
	static f64 GetMouseY();
	static bool GetButton(i32 b);
	static bool GetKey(i32 k);

	static void Close();

	// Graphics
	static u32 CreateShader(const char* path);
	static void DestroyShader(u32 shader);
	static void SetShader(u32 shader);

	static void BeginDraw(bool alpha, bool ztest, f32 pointSize, f32 lineWidth);
	static void EndDraw(u32 mode);

	static void SetViewport(u32 x, u32 y, u32 w, u32 h);
	static void ClearScreen(f32 r, f32 g, f32 b, f32 a, f32 d, f32 s, u32 flags);

	static void SetUniform(const char* name);

	static void SetFloat(f32 x);

	static void SetVec2F(f32 x, f32 y);
	static void SetVec3F(f32 x, f32 y, f32 z);
	static void SetVec4F(f32 x, f32 y, f32 z, f32 w);

	static void SetMat2x2F(
		f32 m00, f32 m01,
		f32 m10, f32 m11);

	static void SetMat3x2F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12);

	static void SetMat2x3F(
		f32 m00, f32 m01,
		f32 m10, f32 m11,
		f32 m20, f32 m21);

	static void SetMat3x3F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12,
		f32 m20, f32 m21, f32 m22);

	static void SetMat4x3F(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23);

	static void SetMat3x4F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12,
		f32 m20, f32 m21, f32 m22,
		f32 m30, f32 m31, f32 m32);

	static void SetMat4x4F(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23,
		f32 m30, f32 m31, f32 m32, f32 m33);

	static void AddVertex(f32 x, f32 y, f32 z, u32 c);

	// Script
	static void ParseFile(const char* moduleName, const char* filepath);
	static void ParseSource(const char* moduleName, const char* source);

	static void BindClass(const char* moduleName, const char* className, ScriptClass scriptClass);
	static void BindMethod(const char* moduleName, const char* className, bool isStatic, const char* signature, ScriptMethodFn scriptMethod);

	static void EnsureSlots(ScriptVM* vm, i32 count);
	static void GetVariable(ScriptVM* vm, const char* moduleName, const char* className, i32 slot);

	static bool GetSlotBool(ScriptVM* vm, i32 slot);
	static u32 GetSlotUInt(ScriptVM* vm, i32 slot);
	static i32 GetSlotInt(ScriptVM* vm, i32 slot);
	static f32 GetSlotFloat(ScriptVM* vm, i32 slot);
	static f64 GetSlotDouble(ScriptVM* vm, i32 slot);
	static const char* GetSlotString(ScriptVM* vm, i32 slot);
	static const char* GetSlotBytes(ScriptVM* vm, i32 slot, i32* length);
	static void* GetSlotObject(ScriptVM* vm, i32 slot);

	template <typename T>
	static T* GetSlotObjectT(ScriptVM* vm, i32 slot)
	{
		return (T*)GetSlotObject(vm, slot);
	}

	static void SetSlotBool(ScriptVM* vm, i32 slot, bool value);
	static void SetSlotUInt(ScriptVM* vm, i32 slot, u32 value);
	static void SetSlotInt(ScriptVM* vm, i32 slot, i32 value);
	static void SetSlotFloat(ScriptVM* vm, i32 slot, f32 value);
	static void SetSlotDouble(ScriptVM* vm, i32 slot, f64 value);
	//static const char* GetSlotString(ScriptVM* vm, i32 slot);
	//static void* GetSlotObject(ScriptVM* vm, i32 slot);
	//static const char* GetSlotBytes(ScriptVM* vm, i32 slot, i32* length);
	static void* SetSlotNewObject(ScriptVM* vm, i32 slot, i32 classSlot, size_t size);

	template <typename T>
	static T* SetSlotNewObjectT(ScriptVM* vm, i32 slot, i32 classSlot)
	{
		return (T*)SetSlotNewObject(vm, slot, classSlot, sizeof(T));
	}
};