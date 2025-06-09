#include <App.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/json.hpp>

#include <fstream>

struct GuiFont
{
	std::string path{};
	f32 size{ 20 };
};

struct GuiGlobal
{
	GuiFont font{};
};
static GuiGlobal g{};

template<class Archive>
void serialize(Archive& archive, GuiFont& m)
{
	archive(
		cereal::make_nvp("path", m.path),
		cereal::make_nvp("size", m.size));
}

template<class Archive>
void serialize(Archive& archive, ImVec2& m)
{
	archive(
		cereal::make_nvp("x", m.x),
		cereal::make_nvp("y", m.y));
}

template<class Archive>
void serialize(Archive& archive, ImVec4& m)
{
	archive(
		cereal::make_nvp("x", m.x),
		cereal::make_nvp("y", m.y),
		cereal::make_nvp("z", m.z),
		cereal::make_nvp("w", m.w));
}

template<class Archive>
void serialize(Archive& archive, ImGuiStyle& m)
{
	// Scalars & Vectors
	archive(
		cereal::make_nvp("Alpha", m.Alpha),
		cereal::make_nvp("DisabledAlpha", m.DisabledAlpha),
		cereal::make_nvp("WindowPadding", m.WindowPadding),
		cereal::make_nvp("WindowRounding", m.WindowRounding),
		cereal::make_nvp("WindowBorderSize", m.WindowBorderSize),
		cereal::make_nvp("WindowMinSize", m.WindowMinSize),
		cereal::make_nvp("WindowTitleAlign", m.WindowTitleAlign),
		cereal::make_nvp("WindowMenuButtonPosition", m.WindowMenuButtonPosition),
		cereal::make_nvp("ChildRounding", m.ChildRounding),
		cereal::make_nvp("ChildBorderSize", m.ChildBorderSize),
		cereal::make_nvp("PopupRounding", m.PopupRounding),
		cereal::make_nvp("PopupBorderSize", m.PopupBorderSize),
		cereal::make_nvp("FramePadding", m.FramePadding),
		cereal::make_nvp("FrameRounding", m.FrameRounding),
		cereal::make_nvp("FrameBorderSize", m.FrameBorderSize),
		cereal::make_nvp("ItemSpacing", m.ItemSpacing),
		cereal::make_nvp("ItemInnerSpacing", m.ItemInnerSpacing),
		cereal::make_nvp("CellPadding", m.CellPadding),
		cereal::make_nvp("TouchExtraPadding", m.TouchExtraPadding),
		cereal::make_nvp("IndentSpacing", m.IndentSpacing),
		cereal::make_nvp("ColumnsMinSpacing", m.ColumnsMinSpacing),
		cereal::make_nvp("ScrollbarSize", m.ScrollbarSize),
		cereal::make_nvp("ScrollbarRounding", m.ScrollbarRounding),
		cereal::make_nvp("GrabMinSize", m.GrabMinSize),
		cereal::make_nvp("GrabRounding", m.GrabRounding),
		cereal::make_nvp("LogSliderDeadzone", m.LogSliderDeadzone),
		cereal::make_nvp("TabRounding", m.TabRounding),
		cereal::make_nvp("TabBorderSize", m.TabBorderSize),
		cereal::make_nvp("TabMinWidthForCloseButton", m.TabMinWidthForCloseButton),
		cereal::make_nvp("TabBarBorderSize", m.TabBarBorderSize),
		cereal::make_nvp("TableAngledHeadersAngle", m.TableAngledHeadersAngle),
		cereal::make_nvp("TableAngledHeadersTextAlign", m.TableAngledHeadersTextAlign),
		cereal::make_nvp("ColorButtonPosition", m.ColorButtonPosition),
		cereal::make_nvp("ButtonTextAlign", m.ButtonTextAlign),
		cereal::make_nvp("SelectableTextAlign", m.SelectableTextAlign),
		cereal::make_nvp("SeparatorTextBorderSize", m.SeparatorTextBorderSize),
		cereal::make_nvp("SeparatorTextAlign", m.SeparatorTextAlign),
		cereal::make_nvp("SeparatorTextPadding", m.SeparatorTextPadding),
		cereal::make_nvp("DisplayWindowPadding", m.DisplayWindowPadding),
		cereal::make_nvp("DisplaySafeAreaPadding", m.DisplaySafeAreaPadding),
		cereal::make_nvp("MouseCursorScale", m.MouseCursorScale),
		cereal::make_nvp("AntiAliasedLines", m.AntiAliasedLines),
		cereal::make_nvp("AntiAliasedLinesUseTex", m.AntiAliasedLinesUseTex),
		cereal::make_nvp("AntiAliasedFill", m.AntiAliasedFill),
		cereal::make_nvp("CurveTessellationTol", m.CurveTessellationTol),
		cereal::make_nvp("CircleTessellationMaxError", m.CircleTessellationMaxError)
	);

	// Colors (already in your code)
	archive(
		cereal::make_nvp("ImGuiCol_Text", m.Colors[ImGuiCol_Text]),
		cereal::make_nvp("ImGuiCol_TextDisabled", m.Colors[ImGuiCol_TextDisabled]),
		cereal::make_nvp("ImGuiCol_WindowBg", m.Colors[ImGuiCol_WindowBg]),
		cereal::make_nvp("ImGuiCol_ChildBg", m.Colors[ImGuiCol_ChildBg]),
		cereal::make_nvp("ImGuiCol_PopupBg", m.Colors[ImGuiCol_PopupBg]),
		cereal::make_nvp("ImGuiCol_Border", m.Colors[ImGuiCol_Border]),
		cereal::make_nvp("ImGuiCol_BorderShadow", m.Colors[ImGuiCol_BorderShadow]),
		cereal::make_nvp("ImGuiCol_FrameBg", m.Colors[ImGuiCol_FrameBg]),
		cereal::make_nvp("ImGuiCol_FrameBgHovered", m.Colors[ImGuiCol_FrameBgHovered]),
		cereal::make_nvp("ImGuiCol_FrameBgActive", m.Colors[ImGuiCol_FrameBgActive]),
		cereal::make_nvp("ImGuiCol_TitleBg", m.Colors[ImGuiCol_TitleBg]),
		cereal::make_nvp("ImGuiCol_TitleBgActive", m.Colors[ImGuiCol_TitleBgActive]),
		cereal::make_nvp("ImGuiCol_TitleBgCollapsed", m.Colors[ImGuiCol_TitleBgCollapsed]),
		cereal::make_nvp("ImGuiCol_MenuBarBg", m.Colors[ImGuiCol_MenuBarBg]),
		cereal::make_nvp("ImGuiCol_ScrollbarBg", m.Colors[ImGuiCol_ScrollbarBg]),
		cereal::make_nvp("ImGuiCol_ScrollbarGrab", m.Colors[ImGuiCol_ScrollbarGrab]),
		cereal::make_nvp("ImGuiCol_ScrollbarGrabHovered", m.Colors[ImGuiCol_ScrollbarGrabHovered]),
		cereal::make_nvp("ImGuiCol_ScrollbarGrabActive", m.Colors[ImGuiCol_ScrollbarGrabActive]),
		cereal::make_nvp("ImGuiCol_CheckMark", m.Colors[ImGuiCol_CheckMark]),
		cereal::make_nvp("ImGuiCol_SliderGrab", m.Colors[ImGuiCol_SliderGrab]),
		cereal::make_nvp("ImGuiCol_SliderGrabActive", m.Colors[ImGuiCol_SliderGrabActive]),
		cereal::make_nvp("ImGuiCol_Button", m.Colors[ImGuiCol_Button]),
		cereal::make_nvp("ImGuiCol_ButtonHovered", m.Colors[ImGuiCol_ButtonHovered]),
		cereal::make_nvp("ImGuiCol_ButtonActive", m.Colors[ImGuiCol_ButtonActive]),
		cereal::make_nvp("ImGuiCol_Header", m.Colors[ImGuiCol_Header]),
		cereal::make_nvp("ImGuiCol_HeaderHovered", m.Colors[ImGuiCol_HeaderHovered]),
		cereal::make_nvp("ImGuiCol_HeaderActive", m.Colors[ImGuiCol_HeaderActive]),
		cereal::make_nvp("ImGuiCol_Separator", m.Colors[ImGuiCol_Separator]),
		cereal::make_nvp("ImGuiCol_SeparatorHovered", m.Colors[ImGuiCol_SeparatorHovered]),
		cereal::make_nvp("ImGuiCol_SeparatorActive", m.Colors[ImGuiCol_SeparatorActive]),
		cereal::make_nvp("ImGuiCol_ResizeGrip", m.Colors[ImGuiCol_ResizeGrip]),
		cereal::make_nvp("ImGuiCol_ResizeGripHovered", m.Colors[ImGuiCol_ResizeGripHovered]),
		cereal::make_nvp("ImGuiCol_ResizeGripActive", m.Colors[ImGuiCol_ResizeGripActive]),
		cereal::make_nvp("ImGuiCol_Tab", m.Colors[ImGuiCol_Tab]),
		cereal::make_nvp("ImGuiCol_TabHovered", m.Colors[ImGuiCol_TabHovered]),
		cereal::make_nvp("ImGuiCol_TabActive", m.Colors[ImGuiCol_TabActive]),
		cereal::make_nvp("ImGuiCol_TabUnfocused", m.Colors[ImGuiCol_TabUnfocused]),
		cereal::make_nvp("ImGuiCol_TabUnfocusedActive", m.Colors[ImGuiCol_TabUnfocusedActive]),
		cereal::make_nvp("ImGuiCol_PlotLines", m.Colors[ImGuiCol_PlotLines]),
		cereal::make_nvp("ImGuiCol_PlotLinesHovered", m.Colors[ImGuiCol_PlotLinesHovered]),
		cereal::make_nvp("ImGuiCol_PlotHistogram", m.Colors[ImGuiCol_PlotHistogram]),
		cereal::make_nvp("ImGuiCol_PlotHistogramHovered", m.Colors[ImGuiCol_PlotHistogramHovered]),
		cereal::make_nvp("ImGuiCol_TableHeaderBg", m.Colors[ImGuiCol_TableHeaderBg]),
		cereal::make_nvp("ImGuiCol_TableBorderStrong", m.Colors[ImGuiCol_TableBorderStrong]),
		cereal::make_nvp("ImGuiCol_TableBorderLight", m.Colors[ImGuiCol_TableBorderLight]),
		cereal::make_nvp("ImGuiCol_TableRowBg", m.Colors[ImGuiCol_TableRowBg]),
		cereal::make_nvp("ImGuiCol_TableRowBgAlt", m.Colors[ImGuiCol_TableRowBgAlt]),
		cereal::make_nvp("ImGuiCol_TextSelectedBg", m.Colors[ImGuiCol_TextSelectedBg]),
		cereal::make_nvp("ImGuiCol_DragDropTarget", m.Colors[ImGuiCol_DragDropTarget]),
		cereal::make_nvp("ImGuiCol_NavHighlight", m.Colors[ImGuiCol_NavHighlight]),
		cereal::make_nvp("ImGuiCol_NavWindowingHighlight", m.Colors[ImGuiCol_NavWindowingHighlight]),
		cereal::make_nvp("ImGuiCol_NavWindowingDimBg", m.Colors[ImGuiCol_NavWindowingDimBg]),
		cereal::make_nvp("ImGuiCol_ModalWindowDimBg", m.Colors[ImGuiCol_ModalWindowDimBg])
	);
}

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

	GuiResetStyle();
	GuiLoadStyle();

	io.Fonts->AddFontFromFileTTF(App::FilePath(g.font.path.c_str()), g.font.size);

	io.IniFilename = PATH("Assets/App/imgui.ini");
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

	WrenBindMethod("app", "App", true, "guiAbsText(_,_,_,_)",
		[](ScriptVM* vm)
		{
			WrenEnsureSlots(vm, 4);
			GuiAbsText(WrenGetSlotString(vm, 1), WrenGetSlotFloat(vm, 2), WrenGetSlotFloat(vm, 3), WrenGetSlotUInt(vm, 4));
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

void App::GuiSaveStyle()
{
	try
	{
		std::ofstream os(PATH("/Assets/App/style.json"));
		cereal::JSONOutputArchive ar(os);
		ar(cereal::make_nvp("Font", g.font));
		ar(cereal::make_nvp("ImGuiStyle", ImGui::GetStyle()));
	}
	catch (std::exception e)
	{
		LOGW("Failed to save style: %s", e.what());
	}
}

void App::GuiLoadStyle()
{
	try
	{
		std::ifstream is(PATH("/Assets/App/style.json"));
		cereal::JSONInputArchive ar(is);
		ar(cereal::make_nvp("Font", g.font));
		ar(cereal::make_nvp("ImGuiStyle", ImGui::GetStyle()));
	}
	catch (std::exception e)
	{
		LOGW("Failed to load style: %s", e.what());
		GuiResetStyle();
		GuiSaveStyle();
	}
}

void App::GuiResetStyle()
{
	g.font.path = "Assets/App/Fonts/UbuntuMono-Regular.ttf";
	g.font.size = 20;
	ImGui::StyleColorsDark();
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

void App::GuiAbsText(const char* text, f32 x, f32 y, u32 c)
{
	ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
	drawlist->AddText(ImVec2(x, y), c, text);
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