#pragma once

#include <cstdint>
#include <cstddef>

// Util
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
using AppPostInitializeFn = void (*)();

struct AppConfig
{
	i32 width = 800, height = 600;
	const char* title = "";
	bool fullscreen = false;

	i32 msaa = 8;
};

class App
{
private:
	static bool Initialize(const AppConfig& config);
	static void Shutdown();

	static void Update();
	static void Render();

public:
	static int Run(AppConfigureFn Configure, AppPostInitializeFn PostInitialize);

	// Window
	static void GetSize(i32& w, i32& h);
	static void Close();

	// Input
	static void GetMouse(f64& x, f64& y);
	static bool GetButton(i32 b);
	static bool GetKey(i32 k);

	// Graphics
	static void SetCamera(f32 px, f32 py, f32 pz, f32 vx, f32 vy, f32 vz, f32 zn, f32 zf, bool ortho, f32 param);

	static void DrawLine2(f32 x1, f32 y1, f32 x2, f32 y2, u32 c);
	static void DrawLine3(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, u32 c);

	static void DrawQuad2(f32 x1, f32 y1, f32 x2, f32 y2, u32 c);
	static void DrawQuad3(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, u32 c);

	// Script
	static void Reload();
	static void ParseFile(const char* moduleName, const char* filename);
	static void ParseSource(const char* moduleName, const char* source);

	static void BindClass(const char* moduleName, const char* className, ScriptClass scriptClass);
	static void BindMethod(const char* moduleName, const char* className, bool isStatic, const char* signature, ScriptMethodFn scriptMethod);
};