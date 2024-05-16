#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>

// Util
#define PATH(x) (PROJECT_PATH x)
#define LOGI(format, ...) App::Log(true, __FILE__, __LINE__, __func__, 0xFFFFFFFF, format, ##__VA_ARGS__)
#define LOGW(format, ...) App::Log(true, __FILE__, __LINE__, __func__, 0xFFFF00FF, format, ##__VA_ARGS__)
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

struct AppConfig
{
	i32 width = 800, height = 600;
	const char* title = "";
	bool fullscreen = false;

	i32 msaa = 8;

	const char* gamefile = PATH("/Common/Game.wren");
};

class App
{
private:
	static bool Initialize(const AppConfig& config);
	static void Shutdown();

	static void Reload(AppBindApiFn BindApi);

	static void Update(f64 dt);
	static void Render();

public:
	static int Run(AppConfigureFn Configure, AppBindApiFn BindApi);

	// Util
	static void Log(bool verbose, const char* file, i32 line, const char* func, u32 color, const char* format, ...);

	// Window
	static i32 GetWidth();
	static i32 GetHeight();

	static f64 GetMouseX();
	static f64 GetMouseY();
	static bool GetButton(i32 b);
	static bool GetKey(i32 k);

	static void Close();

	// Graphics
	static void SetCamera(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23,
		f32 m30, f32 m31, f32 m32, f32 m33);

	static void DrawPoint2(f32 x1, f32 y1, u32 c);
	static void DrawLine2(f32 x1, f32 y1, f32 x2, f32 y2, u32 c);
	static void DrawQuad2(f32 x1, f32 y1, f32 x2, f32 y2, u32 c);

	static void DrawPoint3(f32 x1, f32 y1, f32 z1, u32 c);
	static void DrawLine3(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, u32 c);
	static void DrawQuad3(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, u32 c);

	// Script
	static void ParseFile(const char* moduleName, const char* filepath);
	static void ParseSource(const char* moduleName, const char* source);

	static void BindClass(const char* moduleName, const char* className, ScriptClass scriptClass);
	static void BindMethod(const char* moduleName, const char* className, bool isStatic, const char* signature, ScriptMethodFn scriptMethod);

	static void EnsureSlots(ScriptVM* vm, i32 count);

	static bool GetSlotBool(ScriptVM* vm, i32 slot);
	static u32 GetSlotUInt(ScriptVM* vm, i32 slot);
	static i32 GetSlotInt(ScriptVM* vm, i32 slot);
	static f32 GetSlotFloat(ScriptVM* vm, i32 slot);
	static f64 GetSlotDouble(ScriptVM* vm, i32 slot);
	static const char* GetSlotString(ScriptVM* vm, i32 slot);
	static void* GetSlotObject(ScriptVM* vm, i32 slot);
	static const char* GetSlotBytes(ScriptVM* vm, i32 slot, i32* length);

	static void SetSlotBool(ScriptVM* vm, i32 slot, bool value);
	static void SetSlotUInt(ScriptVM* vm, i32 slot, u32 value);
	static void SetSlotInt(ScriptVM* vm, i32 slot, i32 value);
	static void SetSlotFloat(ScriptVM* vm, i32 slot, f32 value);
	static void SetSlotDouble(ScriptVM* vm, i32 slot, f64 value);
	//static const char* GetSlotString(ScriptVM* vm, i32 slot);
	//static void* GetSlotObject(ScriptVM* vm, i32 slot);
	//static const char* GetSlotBytes(ScriptVM* vm, i32 slot, i32* length);
};