#include <App.hpp>
#include "MVec2.hpp"

#include <iostream>

static AppConfig Configure()
{
	AppConfig config{};
	config.width = 800;
	config.height = 600;
	config.title = "GA Lectures Pt2";
	config.fullscreen = false;
	config.msaa = 8;
	config.gamefile = PATH("/Pt2/game.wren");

	return config;
}

static void BindApi()
{
	App::BindClass("mvec2", "MVec2",
		ScriptClass
		{
			[](ScriptVM* vm)
			{
				MVec2f32* mvec2 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
				new (mvec2) MVec2f32(App::GetSlotFloat(vm, 1), App::GetSlotFloat(vm, 2), App::GetSlotFloat(vm, 3), App::GetSlotFloat(vm, 4));
			},
			[](void* data)
			{
				auto obj = (MVec2f32*)data;
				obj->~MVec2f32();
			}
		});

	App::BindMethod("mvec2", "MVec2", false, "e0",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 0); App::SetSlotFloat(vm, 0, App::GetSlotObjectT<MVec2f32>(vm, 0)->GetE0()); });
	App::BindMethod("mvec2", "MVec2", false, "e1",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 0); App::SetSlotFloat(vm, 0, App::GetSlotObjectT<MVec2f32>(vm, 0)->GetE1()); });
	App::BindMethod("mvec2", "MVec2", false, "e2",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 0); App::SetSlotFloat(vm, 0, App::GetSlotObjectT<MVec2f32>(vm, 0)->GetE2()); });
	App::BindMethod("mvec2", "MVec2", false, "e12",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 0); App::SetSlotFloat(vm, 0, App::GetSlotObjectT<MVec2f32>(vm, 0)->GetE12()); });
	App::BindMethod("mvec2", "MVec2", false, "e0=(_)",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 1); App::GetSlotObjectT<MVec2f32>(vm, 0)->SetE0(App::GetSlotFloat(vm, 1)); });
	App::BindMethod("mvec2", "MVec2", false, "e1=(_)",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 1); App::GetSlotObjectT<MVec2f32>(vm, 0)->SetE1(App::GetSlotFloat(vm, 1)); });
	App::BindMethod("mvec2", "MVec2", false, "e2=(_)",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 1); App::GetSlotObjectT<MVec2f32>(vm, 0)->SetE2(App::GetSlotFloat(vm, 1)); });
	App::BindMethod("mvec2", "MVec2", false, "e12=(_)",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 1); App::GetSlotObjectT<MVec2f32>(vm, 0)->SetE12(App::GetSlotFloat(vm, 1)); });

	App::BindMethod("mvec2", "MVec2", false, "-",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z2) MVec2f32(-(*z1));
		});

	App::BindMethod("mvec2", "MVec2", false, "~",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z2) MVec2f32(~(*z1));
		});

	App::BindMethod("mvec2", "MVec2", false, "dual",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z2) MVec2f32(z1->GetDual());
		});

	App::BindMethod("mvec2", "MVec2", false, "inv",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z2) MVec2f32(z1->GetInverse());
		});

	App::BindMethod("mvec2", "MVec2", false, "+(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2f32>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z3) MVec2f32((*z1) + (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "-(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2f32>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z3) MVec2f32((*z1) - (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "%(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2f32>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z3) MVec2f32((*z1) % (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "^(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2f32>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z3) MVec2f32((*z1) ^ (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "*(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2f32>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z3) MVec2f32((*z1) * (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "/(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2f32>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z3) MVec2f32((*z1) / (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "|(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2f32>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z3) MVec2f32((*z1) | (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "grade(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2f32>(vm, 0);
			auto i = App::GetSlotInt(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z2) MVec2f32((*z1).GetGrade(i));
		});

	App::BindMethod("mvec2", "MVec2", true, "exp(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z = App::GetSlotObjectT<MVec2f32>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z2) MVec2f32(MVec2f32::Exp(*z));
		});

	App::BindMethod("mvec2", "MVec2", true, "inf",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z = App::SetSlotNewObjectT<MVec2f32>(vm, 0, 0);
			new (z) MVec2f32(MVec2f32::Inf());
		});

	App::BindMethod("mvec2", "MVec2", false, "draw(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z = App::GetSlotObjectT<MVec2f32>(vm, 0);
			z->Draw(App::GetSlotUInt(vm, 1));
		});

	App::BindMethod("mvec2", "MVec2", false, "debug(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z = App::GetSlotObjectT<MVec2f32>(vm, 0);
			z->Debug(App::GetSlotString(vm, 1));
		});

	App::ParseFile("mvec2", PATH("/Pt1/mvec2.wren"));
	App::ParseFile("utils", PATH("/Common/utils.wren"));
}

///////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

// Helper function to trim leading spaces from a string
std::string ltrim(const std::string& str) {
	std::string result = str;
	result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
	return result;
}

// Helper function to trim trailing spaces from a string
std::string rtrim(const std::string& str) {
	std::string result = str;
	result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), result.end());
	return result;
}

// Helper function to trim both leading and trailing spaces from a string
std::string trim(const std::string& str) {
	return ltrim(rtrim(str));
}

// Function to split a string by a delimiter and remove trailing spaces
std::vector<std::string> splitAndTrim(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::istringstream stream(str);
	std::string token;

	while (std::getline(stream, token, delimiter)) {
		tokens.push_back(trim(token));
	}

	return tokens;
}

int main(int argc, char** args)
{
	std::string inputA = "a0 e0 + a1 e1 + a2 e2 + ap ep + an en + a12 e12 + a1p e1p + a1n e1n + a2p e2p + a2n e2n + apn epn + a12p e12p + a12n e12n + a1pn e1pn + a2pn e2pn + a12pn e12pn";
	std::string inputB = "b0 e0 + b1 e1 + b2 e2 + bp ep + bn en + b12 e12 + b1p e1p + b1n e1n + b2p e2p + b2n e2n + bpn epn + b12p e12p + b12n e12n + b1pn e1pn + b2pn e2pn + b12pn e12pn";

	char delimiter = '+';

	std::vector<std::string> tokensA = splitAndTrim(inputA, delimiter);
	std::vector<std::string> tokensB = splitAndTrim(inputB, delimiter);
	std::string result = "";

	// Print the tokens to verify the result
	for (const auto& tokenA : tokensA)
	{
		for (const auto& tokenB : tokensB)
		{
			std::cout << tokenA << ' ' << tokenB << " + ";
		}
	}

	return App::Run(&Configure, &BindApi);
}