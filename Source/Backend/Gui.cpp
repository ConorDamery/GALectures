#include <App.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

bool App::GuiInitialize(const AppConfig& config)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	if (!GuiWinInitialize())
	{
		LOGE("Failed to initialize ImGui window!");
		return false;
	}

	if (!GuiGlInitialize())
	{
		LOGE("Failed to initialize ImGui graphics!");
		return false;
	}

	ImGui::StyleColorsDark();

	io.Fonts->AddFontFromFileTTF(PATH("Assets/App/Fonts/UbuntuMono-Regular.ttf"), 20);

	io.IniFilename = "imgui.ini";
	return true;
}

void App::GuiShutdown()
{
	GuiGlShutdown();
	GuiWinShutdown();
	ImGui::DestroyContext();
}

void App::GuiReload()
{
	// Gui
	WrenBindMethod("app", "App", true, "guiPushItemWidth(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiPushItemWidth(WrenGetSlotFloat(vm, 1));
		});

	WrenBindMethod("app", "App", true, "guiPopItemWidth()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiPopItemWidth();
		});

	WrenBindMethod("app", "App", true, "guiText(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiText(WrenGetSlotString(vm, 1));
		});

	WrenBindMethod("app", "App", true, "guiBool(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotBool(vm, 0, GuiBool(WrenGetSlotString(vm, 1), WrenGetSlotBool(vm, 2)));
		});

	WrenBindMethod("app", "App", true, "guiInt(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotInt(vm, 0, GuiInt(WrenGetSlotString(vm, 1), WrenGetSlotInt(vm, 2)));
		});

	WrenBindMethod("app", "App", true, "guiInt(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			WrenSetSlotInt(vm, 0, GuiInt(WrenGetSlotString(vm, 1), WrenGetSlotInt(vm, 2), WrenGetSlotInt(vm, 3), WrenGetSlotInt(vm, 4)));
		});

	WrenBindMethod("app", "App", true, "guiFloat(_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotFloat(vm, 0, GuiFloat(WrenGetSlotString(vm, 1), WrenGetSlotFloat(vm, 2)));
		});

	WrenBindMethod("app", "App", true, "guiFloat(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 2);
			WrenSetSlotFloat(vm, 0, GuiFloat(WrenGetSlotString(vm, 1), WrenGetSlotFloat(vm, 2), WrenGetSlotFloat(vm, 3), WrenGetSlotFloat(vm, 4)));
		});

	WrenBindMethod("app", "App", true, "guiSeparator(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiSeparator(WrenGetSlotString(vm, 1));
		});

	WrenBindMethod("app", "App", true, "guiButton(_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotBool(vm, 0, GuiButton(WrenGetSlotString(vm, 1)));
		});

	WrenBindMethod("app", "App", true, "guiSameLine()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiSameLine();
		});

	WrenBindMethod("app", "App", true, "guiContentAvailWidth()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotFloat(vm, 0, GuiContentAvailWidth());
		});

	WrenBindMethod("app", "App", true, "guiContentAvailHeight()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			WrenSetSlotFloat(vm, 0, GuiContentAvailHeight());
		});

	WrenBindMethod("app", "App", true, "guiBeginChild(_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 3);
			WrenSetSlotBool(vm, 0, GuiBeginChild(WrenGetSlotString(vm, 1), WrenGetSlotFloat(vm, 2), WrenGetSlotFloat(vm, 3)));
		});

	WrenBindMethod("app", "App", true, "guiEndChild()",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 1);
			GuiEndChild();
		});
}

// Gui
void App::GuiPushItemWidth(f32 w)
{
	ImGui::PushItemWidth(w);
}

void App::GuiPopItemWidth()
{
	ImGui::PopItemWidth();
}

void App::GuiText(const char* text)
{
	ImGui::Text(text);
}

bool App::GuiBool(const char* label, bool v)
{
	ImGui::Checkbox(label, &v);
	return v;
}

i32 App::GuiInt(const char* label, i32 i)
{
	ImGui::InputInt(label, &i);
	return i;
}

i32 App::GuiInt(const char* label, i32 i, i32 min, i32 max)
{
	ImGui::SliderInt(label, &i, min, max);
	return i;
}

f32 App::GuiFloat(const char* label, f32 v)
{
	ImGui::DragFloat(label, &v, 0.1f);
	return v;
}

f32 App::GuiFloat(const char* label, f32 v, f32 min, f32 max)
{
	ImGui::SliderFloat(label, &v, min, max);
	return v;
}

void App::GuiSeparator(const char* label)
{
	ImGui::SeparatorText(label);
}

bool App::GuiButton(const char* label)
{
	return ImGui::Button(label);
}

void App::GuiSameLine()
{
	ImGui::SameLine();
}

f32 App::GuiContentAvailWidth()
{
	return ImGui::GetContentRegionAvail().x;
}

f32 App::GuiContentAvailHeight()
{
	return ImGui::GetContentRegionAvail().y;
}

bool App::GuiBeginChild(const char* label, f32 w, f32 h)
{
	return ImGui::BeginChild(label, ImVec2(w, h));
}

void App::GuiEndChild()
{
	ImGui::EndChild();
}