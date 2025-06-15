#include <App.hpp>

extern "C" {
#include <wren.h>
#include <wren_vm.h>
}

using namespace GASandbox;

struct WrenGlobal
{
	// Script
	WrenVM* vm{ nullptr };
	WrenHandle* mainClass{ nullptr };
	WrenHandle* initMethod{ nullptr };
	WrenHandle* updateMethod{ nullptr };
	WrenHandle* renderMethod{ nullptr };
	WrenHandle* netcodeMethod{ nullptr };
	WrenHandle* audioMethod{ nullptr };
	bool error{ false };
	bool paused{ false };

	hashmap<size_type, sCodeClass> classes{};
	hashmap<size_type, fCodeMethod> methods{};
};
static WrenGlobal g;

static size_type wren_class_hash(cstring moduleName, cstring className)
{
	return App::Hash(moduleName) ^ App::Hash(className);
}

static size_type wren_method_hash(cstring moduleName, cstring className, bool isStatic, cstring signature)
{
	return App::Hash(moduleName) ^ App::Hash(className) ^ App::Hash(isStatic ? "s" : "") ^ App::Hash(signature);
}

static WrenForeignMethodFn wren_bind_method(WrenVM* vm, cstring moduleName, cstring className, bool isStatic, cstring signature)
{
	if (strcmp(moduleName, "random") == 0)
		return nullptr;
	if (strcmp(moduleName, "meta") == 0)
		return nullptr;

	const size_type hash = wren_method_hash(moduleName, className, isStatic, signature);
	const auto it = g.methods.find(hash);
	if (it != g.methods.end())
		return (WrenForeignMethodFn)it->second;
	return nullptr;
}

static WrenForeignMethodFn wren_allocate(const size_type classHash)
{
	const auto it = g.classes.find(classHash);
	if (it != g.classes.end())
		return (WrenForeignMethodFn)it->second.allocate;
	return nullptr;
}

static WrenFinalizerFn wren_finalizer(const size_type classHash)
{
	const auto it = g.classes.find(classHash);
	if (it != g.classes.end())
		return it->second.finalize;
	return nullptr;
}

static WrenForeignClassMethods wren_bind_class(WrenVM* vm, cstring moduleName, cstring className)
{
	WrenForeignClassMethods methods{};
	methods.allocate = nullptr;
	methods.finalize = nullptr;

	if (strcmp(moduleName, "random") == 0) return methods;
	if (strcmp(moduleName, "meta") == 0) return methods;

	const size_type hash = wren_class_hash(moduleName, className);
	methods.allocate = wren_allocate(hash);
	methods.finalize = wren_finalizer(hash);

	return methods;
}

static WrenLoadModuleResult wren_load_module(WrenVM* vm, cstring name)
{
	WrenLoadModuleResult res{};
	return res;
}

static void wren_write(WrenVM* vm, cstring text)
{
	if (strcmp(text, "\n") == 0)
		return;

	App::Log(false, "", 0, "", 0xFFFFFFFF, "%s", text);
}

static void wren_error(WrenVM* vm, WrenErrorType errorType, cstring module, const int line, cstring msg)
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

static void* wren_reallocate(void* ptr, size_type newSize, void* _)
{
	if (newSize == 0)
	{
		free(ptr);
		return NULL;
	}

	return realloc(ptr, newSize);
}

static cstring wren_resolve_module(WrenVM* vm, cstring importer, cstring name)
{
	return nullptr;
}

bool App::CodeInitialize(const sAppConfig& config)
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

void App::CodeShutdown()
{
	if (g.vm == nullptr)
		return;

	if (g.mainClass) wrenReleaseHandle(g.vm, g.mainClass);
	if (g.initMethod) wrenReleaseHandle(g.vm, g.initMethod);
	if (g.updateMethod) wrenReleaseHandle(g.vm, g.updateMethod);
	if (g.netcodeMethod) wrenReleaseHandle(g.vm, g.netcodeMethod);
	if (g.audioMethod) wrenReleaseHandle(g.vm, g.audioMethod);
	if (g.vm) wrenFreeVM(g.vm);

	g.vm = nullptr;
	g.mainClass = nullptr;
	g.initMethod = nullptr;
	g.updateMethod = nullptr;
	g.netcodeMethod = nullptr;
	g.audioMethod = nullptr;
}

void App::CodeCollectGarbage()
{
	if (!g.error)
		wrenCollectGarbage(g.vm);
}

size_type App::CodeBytesAllocated()
{
	return g.error ? 0 : g.vm->bytesAllocated;
}

bool App::CodeIsPaused()
{
	return g.paused;
}

void App::CodeTogglePaused()
{
	g.paused = !g.paused;
}

void App::CodeUpdate(f64 dt)
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

void App::CodeRender()
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

void App::CodeNetcode(bool server, u32 client, eNetEvent event, u16 peer, u32 channel, u32 packet)
{
	if (g.error || g.paused)
		return;

	try
	{
		wrenEnsureSlots(g.vm, 6);
		wrenSetSlotHandle(g.vm, 0, g.mainClass);
		CodeSetSlotBool(g.vm, 1, server);
		CodeSetSlotUInt(g.vm, 2, client);
		CodeSetSlotUInt(g.vm, 3, (u32)event);
		CodeSetSlotUInt(g.vm, 4, peer);
		CodeSetSlotUInt(g.vm, 5, channel);
		CodeSetSlotUInt(g.vm, 6, packet);
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

f32 App::CodeAudio(f64 sampleRate, f64 dt)
{
	if (g.error || g.paused || g.audioMethod == nullptr)
		return 0;

	f32 out = 0;
	try
	{
		wrenEnsureSlots(g.vm, 3);
		wrenSetSlotHandle(g.vm, 0, g.mainClass);
		CodeSetSlotDouble(g.vm, 1, sampleRate);
		CodeSetSlotDouble(g.vm, 2, dt);
		wrenCall(g.vm, g.audioMethod);
		out = CodeGetSlotFloat(g.vm, 0);
	}
	catch (const std::exception& e)
	{
		LOGE("Script exception: %s", e.what());
		wrenSetSlotString(g.vm, 0, e.what());
		wrenAbortFiber(g.vm, 0);

		g.error = true;
	}
	return out;
}

void App::CodeReload()
{
	if (g.error)
	{
		CodeShutdown();
		return;
	}

	g.initMethod = wrenMakeCallHandle(g.vm, "init()");
	g.updateMethod = wrenMakeCallHandle(g.vm, "update(_)");
	g.renderMethod = wrenMakeCallHandle(g.vm, "render()");
	g.netcodeMethod = wrenMakeCallHandle(g.vm, "netcode(_,_,_,_,_,_)");
	g.audioMethod = wrenMakeCallHandle(g.vm, "audio(_,_)");

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
void App::CodeParseFile(cstring moduleName, cstring filepath)
{
	auto src = FileLoad(filepath);
	CodeParseSource(moduleName, src.c_str());
}

void App::CodeParseSource(cstring moduleName, cstring source)
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

void App::CodeBindClass(cstring moduleName, cstring className, sCodeClass scriptClass)
{
	const size_type hash = wren_class_hash(moduleName, className);
	g.classes.insert(std::make_pair(hash, scriptClass));
}

void App::CodeBindMethod(cstring moduleName, cstring className, bool isStatic, cstring signature, fCodeMethod scriptMethod)
{
	const size_type hash = wren_method_hash(moduleName, className, isStatic, signature);
	g.methods.insert(std::make_pair(hash, scriptMethod));
}

void App::CodeEnsureSlots(sCodeVM vm, i32 count)
{
	wrenEnsureSlots((WrenVM*)vm, count);
}

void App::CodeGetVariable(sCodeVM vm, cstring moduleName, cstring className, i32 slot)
{
	wrenGetVariable((WrenVM*)vm, moduleName, className, slot);
}

bool App::CodeGetSlotBool(sCodeVM vm, i32 slot)
{
	return wrenGetSlotBool((WrenVM*)vm, slot);
}

u32 App::CodeGetSlotUInt(sCodeVM vm, i32 slot)
{
	return (u32)wrenGetSlotDouble((WrenVM*)vm, slot);
}

i32 App::CodeGetSlotInt(sCodeVM vm, i32 slot)
{
	return (i32)wrenGetSlotDouble((WrenVM*)vm, slot);
}

f32 App::CodeGetSlotFloat(sCodeVM vm, i32 slot)
{
	return (f32)wrenGetSlotDouble((WrenVM*)vm, slot);
}

f64 App::CodeGetSlotDouble(sCodeVM vm, i32 slot)
{
	return wrenGetSlotDouble((WrenVM*)vm, slot);
}

cstring App::CodeGetSlotString(sCodeVM vm, i32 slot)
{
	return wrenGetSlotString((WrenVM*)vm, slot);
}

void* App::CodeGetSlotObject(sCodeVM vm, i32 slot)
{
	return wrenGetSlotForeign((WrenVM*)vm, slot);
}

cstring App::CodeGetSlotBytes(sCodeVM vm, i32 slot, i32* length)
{
	return wrenGetSlotBytes((WrenVM*)vm, slot, length);
}

void App::CodeSetSlotBool(sCodeVM vm, i32 slot, bool value)
{
	wrenSetSlotBool((WrenVM*)vm, slot, value);
}

void App::CodeSetSlotUInt(sCodeVM vm, i32 slot, u32 value)
{
	wrenSetSlotDouble((WrenVM*)vm, slot, (f64)value);
}

void App::CodeSetSlotInt(sCodeVM vm, i32 slot, i32 value)
{
	wrenSetSlotDouble((WrenVM*)vm, slot, (f64)value);
}

void App::CodeSetSlotFloat(sCodeVM vm, i32 slot, f32 value)
{
	wrenSetSlotDouble((WrenVM*)vm, slot, (f64)value);
}

void App::CodeSetSlotDouble(sCodeVM vm, i32 slot, f64 value)
{
	wrenSetSlotDouble((WrenVM*)vm, slot, (f64)value);
}

void App::CodeSetSlotString(sCodeVM vm, i32 slot, cstring str)
{
	wrenSetSlotString((WrenVM*)vm, slot, str);
}

void* App::CodeSetSlotNewObject(sCodeVM vm, i32 slot, i32 classSlot, size_type size)
{
	return wrenSetSlotNewForeign((WrenVM*)vm, slot, classSlot, size);
}