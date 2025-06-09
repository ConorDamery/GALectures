#include <App.hpp>

extern "C" {
#include <wren.h>
#include <wren_vm.h>
}

#include <unordered_map>

struct WrenGlobal
{
	// Script
	WrenVM* vm{ nullptr };
	WrenHandle* mainClass{ nullptr };
	WrenHandle* initMethod{ nullptr };
	WrenHandle* updateMethod{ nullptr };
	WrenHandle* renderMethod{ nullptr };
	WrenHandle* netcodeMethod{ nullptr };
	bool error{ false };
	bool paused{ false };

	std::unordered_map<size_t, ScriptClass> classes{};
	std::unordered_map<size_t, ScriptMethodFn> methods{};
};
static WrenGlobal g;

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
	const auto it = g.methods.find(hash);
	if (it != g.methods.end())
		return it->second;
	return nullptr;
}

static WrenForeignMethodFn wren_allocate(const size_t classHash)
{
	const auto it = g.classes.find(classHash);
	if (it != g.classes.end())
		return it->second.allocate;
	return nullptr;
}

static WrenFinalizerFn wren_finalizer(const size_t classHash)
{
	const auto it = g.classes.find(classHash);
	if (it != g.classes.end())
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

	g.error = true;
}

static void* wren_reallocate(void* ptr, SizeType newSize, void* _)
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

bool App::WrenInitialize(const AppConfig& config)
{
	WrenConfiguration wrenConfig;
	wrenInitConfiguration(&wrenConfig);
	wrenConfig.writeFn = wren_write;
	wrenConfig.errorFn = wren_error;
	wrenConfig.bindForeignMethodFn = wren_bind_method;
	wrenConfig.bindForeignClassFn = wren_bind_class;
	wrenConfig.loadModuleFn = wren_load_module;
	wrenConfig.initialHeapSize = 1024LL * 1024 * 32;
	wrenConfig.minHeapSize = 1024LL * 1024 * 16;
	wrenConfig.heapGrowthPercent = 80;
	wrenConfig.reallocateFn = wren_reallocate;
	//config.resolveModuleFn = wren_resolve_module;
	g.vm = wrenNewVM(&wrenConfig);

	return true;
}

void App::WrenShutdown()
{
	if (g.vm == nullptr)
		return;

	if (g.mainClass) wrenReleaseHandle(g.vm, g.mainClass);
	if (g.initMethod) wrenReleaseHandle(g.vm, g.initMethod);
	if (g.updateMethod) wrenReleaseHandle(g.vm, g.updateMethod);
	if (g.netcodeMethod) wrenReleaseHandle(g.vm, g.netcodeMethod);
	if (g.vm) wrenFreeVM(g.vm);

	g.vm = nullptr;
	g.mainClass = nullptr;
	g.initMethod = nullptr;
	g.updateMethod = nullptr;
	g.netcodeMethod = nullptr;
}

void App::WrenCollectGarbage()
{
	if (!g.error)
		wrenCollectGarbage(g.vm);
}

SizeType App::WrenBytesAllocated()
{
	return g.error ? 0 : g.vm->bytesAllocated;
}

bool App::WrenIsPaused()
{
	return g.paused;
}

void App::WrenTogglePaused()
{
	g.paused = !g.paused;
}

void App::WrenUpdate(f64 dt)
{
	if (g.error || g.paused)
		return;

	try
	{
		wrenEnsureSlots(g.vm, 2);
		wrenSetSlotHandle(g.vm, 0, g.mainClass);
		wrenSetSlotDouble(g.vm, 1, dt);
		wrenCall(g.vm, g.updateMethod);
	}
	catch (const std::exception& e)
	{
		LOGE("Script exception: %s", e.what());
		wrenSetSlotString(g.vm, 0, e.what());
		wrenAbortFiber(g.vm, 0);

		g.error = true;
	}
}

void App::WrenRender()
{
	if (g.error)
		return;
	
	try
	{
		wrenEnsureSlots(g.vm, 1);
		wrenSetSlotHandle(g.vm, 0, g.mainClass);
		wrenCall(g.vm, g.renderMethod);
	}
	catch (const std::exception& e)
	{
		LOGE("Script exception: %s", e.what());
		wrenSetSlotString(g.vm, 0, e.what());
		wrenAbortFiber(g.vm, 0);

		g.error = true;
	}
}

void App::WrenNetcode(bool server, u32 client, u32 event, u32 peer, u32 channel, u32 packet)
{
	if (!g.error && !g.paused)
	{
		try
		{
			wrenEnsureSlots(g.vm, 6);
			wrenSetSlotHandle(g.vm, 0, g.mainClass);
			WrenSetSlotBool(g.vm, 1, server);
			WrenSetSlotUInt(g.vm, 2, client);
			WrenSetSlotUInt(g.vm, 3, event);
			WrenSetSlotUInt(g.vm, 4, peer);
			WrenSetSlotUInt(g.vm, 5, channel);
			WrenSetSlotUInt(g.vm, 6, packet);
			wrenCall(g.vm, g.netcodeMethod);
		}
		catch (const std::exception& e)
		{
			LOGE("Script exception: %s", e.what());
			wrenSetSlotString(g.vm, 0, e.what());
			wrenAbortFiber(g.vm, 0);

			g.error = true;
		}
	}
}

void App::WrenReload()
{
	if (g.error)
	{
		WrenShutdown();
		return;
	}

	g.initMethod = wrenMakeCallHandle(g.vm, "init()");
	g.updateMethod = wrenMakeCallHandle(g.vm, "update(_)");
	g.renderMethod = wrenMakeCallHandle(g.vm, "render()");
	g.netcodeMethod = wrenMakeCallHandle(g.vm, "netcode(_,_,_,_,_,_)");

	wrenEnsureSlots(g.vm, 1);
	wrenGetVariable(g.vm, "main", "Main", 0);
	g.mainClass = wrenGetSlotHandle(g.vm, 0);
	wrenSetSlotHandle(g.vm, 0, g.mainClass);

	try
	{
		wrenEnsureSlots(g.vm, 1);
		wrenSetSlotHandle(g.vm, 0, g.mainClass);
		wrenCall(g.vm, g.initMethod);
	}
	catch (const std::exception& e)
	{
		LOGE("Script exception: %s", e.what());
		wrenSetSlotString(g.vm, 0, e.what());
		wrenAbortFiber(g.vm, 0);

		g.error = true;
	}
}

// Script
void App::WrenParseFile(const char* moduleName, const char* filepath)
{
	auto src = FileLoad(filepath);
	WrenParseSource(moduleName, src.c_str());
}

void App::WrenParseSource(const char* moduleName, const char* source)
{
	switch (wrenInterpret(g.vm, moduleName, source))
	{
	case WREN_RESULT_COMPILE_ERROR:
		g.error = true;
		LOGE("Wren compilation error on module: %s", moduleName);
		break;

	case WREN_RESULT_RUNTIME_ERROR:
		g.error = true;
		LOGE("Wren runtime error on module: %s", moduleName);
		break;

	case WREN_RESULT_SUCCESS:
		g.error = false;
		LOGI("Wren successfully compiled module: %s", moduleName);
		break;
	}
}

void App::WrenBindClass(const char* moduleName, const char* className, ScriptClass scriptClass)
{
	const size_t hash = wren_class_hash(moduleName, className);
	g.classes.insert(std::make_pair(hash, scriptClass));
}

void App::WrenBindMethod(const char* moduleName, const char* className, bool isStatic, const char* signature, ScriptMethodFn scriptMethod)
{
	const size_t hash = wren_method_hash(moduleName, className, isStatic, signature);
	g.methods.insert(std::make_pair(hash, scriptMethod));
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

void App::WrenSetSlotString(ScriptVM* vm, i32 slot, const char* str)
{
	wrenSetSlotString(vm, slot, str);
}

void* App::WrenSetSlotNewObject(ScriptVM* vm, i32 slot, i32 classSlot, size_t size)
{
	return wrenSetSlotNewForeign(vm, slot, classSlot, size);
}