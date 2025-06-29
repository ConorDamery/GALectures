#include <App.hpp>

extern "C" {
#include <wren.h>
#include <wren_vm.h>
}

namespace GASandbox
{
	struct sWrenGlobal
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
	static sWrenGlobal g;

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
}

#ifdef CODE_IMPL
#include <engine/wren/script_wren.hpp>

#include <engine/hash.hpp>
#include <engine/macros.hpp>
#include <engine/time.hpp>
#include <engine/profiler.hpp>
#include <engine/file.hpp>
#include <engine/math.hpp>
#include <engine/string.hpp>
#include <engine/hash_map.hpp>

#include <cstdio>
#include <cstring>
#include <functional>

BX_MODULE_DEFINE(ScriptWren)
BX_MODULE_DEFINE_INTERFACE(Script, ScriptWren)

// Helper functions and Wren callbacks
static SizeType GetClassHash(StringView moduleName, StringView className)
{
	static Hash<StringView> m_strHash;
	return m_strHash(moduleName)
		^ m_strHash(className);
}

static SizeType GetMethodHash(StringView moduleName, StringView className, bool isStatic, StringView signature)
{
	static Hash<StringView> m_strHash;
	return m_strHash(moduleName)
		^ m_strHash(className)
		^ m_strHash(isStatic ? "s" : "")
		^ m_strHash(signature);
}

ScriptWrenUserData* ScriptWren::GetUserData(WrenVM* vm)
{
	auto userData = wrenGetUserData(vm);
	BX_ENSURE(userData != nullptr);
	return static_cast<ScriptWrenUserData*>(userData);
}

WrenForeignMethodFn ScriptWren::WrenBindForeignMethod(WrenVM* vm, const char* moduleName, const char* className, bool isStatic, const char* signature)
{
	auto userData = ScriptWren::Get().GetUserData(vm);

	const StringView moduleNameStr = moduleName;
	const StringView classNameStr = className;
	const StringView signatureStr = signature;

	if (moduleNameStr.compare("random") == 0)
		return nullptr;
	if (moduleNameStr.compare("meta") == 0)
		return nullptr;

	const SizeType hash = GetMethodHash(moduleNameStr, classNameStr, isStatic, signatureStr);
	const auto it = userData->ctx->m_foreignMethods.find(hash);
	if (it == userData->ctx->m_foreignMethods.end())
		return nullptr;
	return it->second;
}

WrenForeignMethodFn ScriptWren::WrenAllocate(WrenVM* vm, const SizeType classHash)
{
	auto userData = ScriptWren::Get().GetUserData(vm);

	const auto it = userData->ctx->m_foreignConstructors.find(classHash);
	if (it != userData->ctx->m_foreignConstructors.end())
		return it->second;
	return nullptr;
}

WrenFinalizerFn ScriptWren::WrenFinalizer(WrenVM* vm, const SizeType classHash)
{
	auto userData = ScriptWren::Get().GetUserData(vm);

	const auto it = userData->ctx->m_foreignDestructors.find(classHash);
	if (it != userData->ctx->m_foreignDestructors.end())
		return it->second;
	return nullptr;
}

WrenForeignClassMethods ScriptWren::WrenBindForeignClass(WrenVM* vm, const char* moduleName, const char* className)
{
	const StringView moduleNameStr = moduleName;
	const StringView classNameStr = className;

	WrenForeignClassMethods methods{};
	methods.allocate = nullptr;
	methods.finalize = nullptr;

	if (moduleNameStr.compare("random") == 0) return methods;
	if (moduleNameStr.compare("meta") == 0) return methods;

	const SizeType classHash = GetClassHash(moduleNameStr, classNameStr);
	methods.allocate = WrenAllocate(vm, classHash);
	methods.finalize = WrenFinalizer(vm, classHash);

	return methods;
}

WrenLoadModuleResult ScriptWren::WrenLoadModule(WrenVM* vm, const char* moduleName)
{
	WrenLoadModuleResult res{};

	auto userData = ScriptWren::Get().GetUserData(vm);

	const CString<64> moduleNameStr = moduleName;

	if (moduleNameStr.compare("random") == 0)
		return res;
	if (moduleNameStr.compare("meta") == 0)
		return res;

	for (const auto& src : userData->ctx->m_apiModuleSources)
	{
		if (!moduleNameStr.compare(src.moduleName))
			continue;

		res.source = src.moduleSource.data();
		return res;
	}

	CString<64> moduleNameFmt{};
	moduleNameFmt.format("{}.wren", moduleNameStr);

	if (userData->ctx->m_wrenModulesSource.find(moduleNameStr) == userData->ctx->m_wrenModulesSource.end())
	{
		FilePath filepath{};
		if (!File::Get().Find("[assets]", moduleNameFmt, filepath))
		{
			BX_LOGE(Script, "Module '{}' can not be found!", moduleNameFmt);
			return res;
		}

		String moduleSrc = File::Get().ReadText(filepath);
		userData->ctx->m_wrenModulesSource.insert(std::make_pair(moduleNameStr, moduleSrc));
	}

	BX_ENSURE(userData->ctx->m_wrenModulesSource.find(moduleNameStr) != userData->ctx->m_wrenModulesSource.end());
	res.source = userData->ctx->m_wrenModulesSource[moduleNameStr].c_str();

	return res;
}

void ScriptWren::WrenWrite(WrenVM* vm, const char* text)
{
	const StringView textStr = text;
	if (textStr.compare("\n") == 0)
		return;

	BX_LOGI(Script, textStr.data());
}

void ScriptWren::WrenError(WrenVM* vm, WrenErrorType errorType, const char* module, const i32 line, const char* msg)
{
	auto userData = ScriptWren::Get().GetUserData(vm);

	switch (errorType)
	{
	case WREN_ERROR_COMPILE:
		BX_LOGE(Script, "[Compile: {} line {}] {}", module, line, msg); break;
	case WREN_ERROR_STACK_TRACE:
		BX_LOGE(Script, "[StackTrace: {} line {}] in {}", module, line, msg); break;
	case WREN_ERROR_RUNTIME:
		BX_LOGE(Script, "[Runtime] {}", msg); break;
	}

	userData->error = true;
}

bool ScriptWren::Initialize()
{
	// Get the list of all classes derived from ScriptApiRegister
	auto derivedClasses = rttr::type::get<ScriptApiRegister>().get_derived_classes();
	for (const auto& derivedClass : derivedClasses)
	{
		const auto& derivedClassName = derivedClass.get_name();
		BX_LOGD(Script, "Registering script api: {}", derivedClassName.data());

		// Retrieve the "Register" method from the derived class
		auto registerMethod = derivedClass.get_method("Register");
		if (!registerMethod.is_valid())
		{
			BX_LOGE(Script, "Register method not found for class: {}", derivedClassName.data());
			continue;
		}

		rttr::instance instance;  // Create a default instance, we are calling a static function
		auto ret = registerMethod.invoke(instance);
		m_apiModuleSources.emplace_back(ret.get_value<ScriptModuleSource>());
	}

	return true;
}

void ScriptWren::Shutdown()
{
	m_wrenModulesSource.clear();

	m_foreignMethods.clear();
	m_foreignConstructors.clear();
	m_foreignDestructors.clear();

	m_foreignClassRegistry.clear();
	m_wrenTypeIdMap.clear();

	//m_resourceClassWrappers.clear();
	//m_componentClassWrappers.clear();
}

ScriptHandle ScriptWren::CreateVm(const ScriptVmInfo& info)
{
	auto userData = new ScriptWrenUserData();

	// Create vm
	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.reallocateFn = 0;
	config.resolveModuleFn = 0;
	config.loadModuleFn = ScriptWren::WrenLoadModule;
	config.bindForeignMethodFn = ScriptWren::WrenBindForeignMethod;
	config.bindForeignClassFn = ScriptWren::WrenBindForeignClass;
	config.writeFn = ScriptWren::WrenWrite;
	config.errorFn = ScriptWren::WrenError;
	config.initialHeapSize = info.initialHeapSize;
	config.minHeapSize = info.minHeapSize;
	config.heapGrowthPercent = info.heapGrowthPercent;
	config.userData = userData;
	auto vm = wrenNewVM(&config);

	// Setup user data
	userData->initialized = false;
	userData->error = false;
	userData->vm = vm;
	userData->ctx = this;

	// Compile API into vm
	auto handle = (ScriptHandle)vm;
	for (const auto& src : m_apiModuleSources)
	{
		CompileString(handle, src.moduleName, src.moduleSource);
	}

	//File::FindEach("[assets]", ".wren",
	//	[](const auto& path, const auto& name)
	//	{
	//		WrenCompile(m_vm, name.c_str(), File::ReadTextFile(path).c_str());
	//	});
	//
	//for (auto& it : m_foreignClassRegistry)
	//{
	//	auto& info = it.second;
	//
	//	wrenEnsureSlots(m_vm, 1);
	//	wrenGetVariable(m_vm, info.moduleName.data(), info.className.data(), 0);
	//	info.handle = wrenGetSlotHandle(m_vm, 0);
	//	auto objClass = wrenGetClass(m_vm, info.handle->value);
	//
	//	m_wrenTypeIdMap.insert(std::make_pair(objClass->name->hash, info.typeId));
	//}

	return handle;
}

void ScriptWren::DestroyVm(ScriptHandle vm)
{
	auto wrenVm = (WrenVM*)vm;

	auto userData = GetUserData(wrenVm);
	userData->initialized = false;
	userData->error = false;
	userData->ctx = nullptr;
	userData->vm = nullptr;

	if (wrenVm)
	{
		wrenFreeVM(wrenVm);
		wrenVm = nullptr;
	}
}

void ScriptWren::SetUserData(ScriptHandle vm, void* data)
{
	auto userData = GetUserData((WrenVM*)vm);
	userData->data = data;
}

void* ScriptWren::GetUserData(ScriptHandle vm)
{
	auto userData = GetUserData((WrenVM*)vm);
	return userData->data;
}

bool ScriptWren::CompileString(ScriptHandle vm, StringView moduleName, StringView string)
{
	BeginModule(moduleName);

	// TODO: Move m_wrenModulesSource to the vm context

	bool success = true;

	auto wrenVm = (WrenVM*)vm;
	auto userData = GetUserData(wrenVm);

	const CString<64> moduleNameStr = moduleName;
	//if (m_wrenModulesSource.find(moduleNameStr) == m_wrenModulesSource.end())
	{
		switch (wrenInterpret(wrenVm, moduleNameStr.c_str(), string.data()))
		{
		case WREN_RESULT_COMPILE_ERROR:
			success = false;
			userData->error = true;
			BX_LOGE(Script, "Wren compilation error on module: {}", moduleNameStr);
			break;

		case WREN_RESULT_RUNTIME_ERROR:
			success = false;
			userData->error = true;
			BX_LOGE(Script, "Wren runtime error on module: {}", moduleNameStr);
			break;

		case WREN_RESULT_SUCCESS:
			userData->initialized = true;
			BX_LOGI(Script, "Wren successfully compiled module: {}", moduleNameStr);
			break;
		}

		//if (success)
		//	m_wrenModulesSource.insert(std::make_pair(moduleNameStr, string.data()));

		if (m_wrenModulesSource.find(moduleNameStr) == m_wrenModulesSource.end())
			m_wrenModulesSource.insert(std::make_pair(moduleNameStr, string.data()));
	}

	EndModule();
	return success;
}

bool ScriptWren::CompileFile(ScriptHandle vm, StringView moduleName, StringView filepath)
{
	auto src = File::Get().ReadText(filepath);
	return CompileString(vm, moduleName, src);
}

bool ScriptWren::HasError(ScriptHandle vm)
{
	//return m_error;
	return false;
}

void ScriptWren::ClearError(ScriptHandle vm)
{
	//m_error = false;
}

ScriptHandle ScriptWren::MakeCallHandle(ScriptHandle vm, StringView signature)
{
	auto wrenVm = (WrenVM*)vm;
	auto handle = wrenMakeCallHandle(wrenVm, signature.data());
	return handle;
}

ScriptHandle ScriptWren::MakeClassHandle(ScriptHandle vm, StringView moduleName, StringView className)
{
	auto wrenVm = (WrenVM*)vm;
	wrenGetVariable(wrenVm, moduleName.data(), className.data(), 0);
	return wrenGetSlotHandle(wrenVm, 0);
}

void ScriptWren::ReleaseHandle(ScriptHandle vm, ScriptHandle function)
{
	wrenReleaseHandle((WrenVM*)vm, (WrenHandle*)function);
}

u64 ScriptWren::GetHandleValue(ScriptHandle handle)
{
	auto wrenHandle = (WrenHandle*)handle;
	return wrenHandle->value;
}

void ScriptWren::EnsureSlots(ScriptHandle vm, i32 numSlots)
{
	wrenEnsureSlots((WrenVM*)vm, numSlots);
}

void ScriptWren::CallFunction(ScriptHandle vm, ScriptHandle handle)
{
	auto wrenVm = (WrenVM*)vm;
	auto wrenHandle = (WrenHandle*)handle;
	wrenCall(wrenVm, wrenHandle);
	//wrenCallFunction();
}

void ScriptWren::BeginModule(StringView moduleName)
{
	m_moduleName = moduleName;
}

StringView ScriptWren::GetCurrentModule() const
{
	return m_moduleName;
}

void ScriptWren::EndModule()
{
	m_moduleName = nullptr;
}

void ScriptWren::BeginClass(StringView className, TypeId typeId)
{
	m_className = className;

	if (m_foreignClassRegistry.find(typeId) == m_foreignClassRegistry.end())
	{
		ScriptClassInfo info;
		info.moduleName = m_moduleName;
		info.className = m_className;
		info.typeId = typeId;

		m_foreignClassRegistry.insert(std::make_pair(typeId, info));
	}
}

StringView ScriptWren::GetCurrentClass() const
{
	return m_className;
}

void ScriptWren::EndClass()
{
	m_className = nullptr;
}

const ScriptClassInfo& ScriptWren::GetClassInfo(TypeId typeId)
{
	auto it = m_foreignClassRegistry.find(typeId);
	BX_ASSERT(it != m_foreignClassRegistry.end(), "Class is not registered!");
	return it->second;
}

void ScriptWren::SetClass(ScriptHandle vm, i32 slot, TypeId typeId)
{
	const auto& info = ScriptWren::GetClassInfo(typeId);

	EnsureSlots(vm, slot + 1);
	wrenGetVariable((WrenVM*)vm, info.moduleName.data(), info.className.data(), slot);

	// TODO: Test this
	//auto it = m_foreignClassRegistry.find(typeId);
	//BX_ASSERT(it != m_foreignClassRegistry.end(), "Class is not registered!");
	//const auto& info = it->second;
	//wrenSetSlotHandle(vm, slot, info.handle);
}

void ScriptWren::BindConstructor(StringView signature, ScriptMethodFn func)
{
	const SizeType hash = GetClassHash(m_moduleName, m_className);
	m_foreignConstructors.insert(std::make_pair(hash, (WrenForeignMethodFn)func));
}

void ScriptWren::BindDestructor(ScriptFinalizerFn func)
{
	const SizeType hash = GetClassHash(m_moduleName, m_className);
	m_foreignDestructors.insert(std::make_pair(hash, (WrenFinalizerFn)func));
}

void ScriptWren::BindFunction(bool isStatic, StringView signature, ScriptMethodFn func)
{
	const SizeType hash = GetMethodHash(m_moduleName, m_className, isStatic, signature);
	m_foreignMethods.insert(std::make_pair(hash, (WrenForeignMethodFn)func));
}

bool ScriptWren::GetSlotBool(ScriptHandle vm, i32 slot)
{
	return wrenGetSlotBool((WrenVM*)vm, slot);
}

#define DECLARE_GET_SLOT_NUM(Num, Name)									\
Num ScriptWren::GetSlot##Name(ScriptHandle vm, i32 slot)				\
{																		\
	return static_cast<Num>(wrenGetSlotDouble((WrenVM*)vm, slot));		\
}

DECLARE_GET_SLOT_NUM(u8, U8)
DECLARE_GET_SLOT_NUM(u16, U16)
DECLARE_GET_SLOT_NUM(u32, U32)
DECLARE_GET_SLOT_NUM(u64, U64)
DECLARE_GET_SLOT_NUM(i8, I8)
DECLARE_GET_SLOT_NUM(i16, I16)
DECLARE_GET_SLOT_NUM(i32, I32)
DECLARE_GET_SLOT_NUM(i64, I64)
DECLARE_GET_SLOT_NUM(f32, F32)
DECLARE_GET_SLOT_NUM(f64, F64)

StringView ScriptWren::GetSlotString(ScriptHandle vm, i32 slot)
{
	return wrenGetSlotString((WrenVM*)vm, slot);
}

void* ScriptWren::GetSlotObject(ScriptHandle vm, i32 slot)
{
	return wrenGetSlotForeign((WrenVM*)vm, slot);
}

ScriptHandle ScriptWren::GetSlotHandle(ScriptHandle vm, i32 slot)
{
	return wrenGetSlotHandle((WrenVM*)vm, slot);
}

i32 ScriptWren::GetListCount(ScriptHandle vm, i32 slot)
{
	return wrenGetListCount((WrenVM*)vm, slot);
}

void ScriptWren::GetListElement(ScriptHandle vm, i32 listSlot, i32 index, i32 elementSlot)
{
	return wrenGetListElement((WrenVM*)vm, listSlot, index, elementSlot);
}

void ScriptWren::SetSlotBool(ScriptHandle vm, i32 slot, bool value)
{
	wrenSetSlotBool((WrenVM*)vm, slot, value);
}

#define DECLARE_SET_SLOT_NUM(Num, Name)									\
void ScriptWren::SetSlot##Name(ScriptHandle vm, i32 slot, Num value)	\
{																		\
	wrenSetSlotDouble((WrenVM*)vm, slot, static_cast<Num>(value));		\
}

// TODO: Wren only supports doubles, int 64s don't pack very well but we have no alternative for now...
DECLARE_SET_SLOT_NUM(u8, U8)
DECLARE_SET_SLOT_NUM(u16, U16)
DECLARE_SET_SLOT_NUM(u32, U32)
DECLARE_SET_SLOT_NUM(u64, U64)
DECLARE_SET_SLOT_NUM(i8, I8)
DECLARE_SET_SLOT_NUM(i16, I16)
DECLARE_SET_SLOT_NUM(i32, I32)
DECLARE_SET_SLOT_NUM(i64, I64)
DECLARE_SET_SLOT_NUM(f32, F32)
DECLARE_SET_SLOT_NUM(f64, F64)

void ScriptWren::SetSlotString(ScriptHandle vm, i32 slot, StringView text)
{
	wrenSetSlotString((WrenVM*)vm, slot, text.data());
}

void* ScriptWren::SetSlotNewObject(ScriptHandle vm, i32 slot, i32 classSlot, SizeType size)
{
	return wrenSetSlotNewForeign((WrenVM*)vm, slot, classSlot, size);
}

void ScriptWren::SetSlotHandle(ScriptHandle vm, i32 slot, ScriptHandle handle)
{
	wrenSetSlotHandle((WrenVM*)vm, slot, (WrenHandle*)handle);
}

void ScriptWren::SetSlotNewList(ScriptHandle vm, i32 slot)
{
	wrenSetSlotNewList((WrenVM*)vm, slot);
}

void ScriptWren::SetListElement(ScriptHandle vm, i32 listSlot, i32 index, i32 elementSlot)
{
	wrenSetListElement((WrenVM*)vm, listSlot, index, elementSlot);
}

void ScriptWren::InsertInList(ScriptHandle vm, i32 listSlot, i32 index, i32 elementSlot)
{
	wrenInsertInList((WrenVM*)vm, listSlot, index, elementSlot);
}
#endif