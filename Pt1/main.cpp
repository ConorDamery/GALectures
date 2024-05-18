#include <App.hpp>
#include "MVec2.hpp"

#include <iostream>

static AppConfig Configure()
{
	AppConfig config{};
	config.width = 800;
	config.height = 600;
	config.title = "GA Lectures Pt1";
	config.fullscreen = false;
	config.msaa = 8;
	config.gamefile = PATH("/Pt1/game.wren");

	return config;
}

static void BindApi()
{
	App::ParseFile("utils", PATH("/Pt1/utils.wren"));

	App::BindClass("mvec2", "MVec2",
		ScriptClass
		{
			[](ScriptVM* vm)
			{
				MVec2* mvec2 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
				new (mvec2) MVec2(App::GetSlotFloat(vm, 1), App::GetSlotFloat(vm, 2), App::GetSlotFloat(vm, 3), App::GetSlotFloat(vm, 4));
			},
			[](void* data)
			{
				auto obj = (MVec2*)data;
				obj->~MVec2();
			}
		});

	App::BindMethod("mvec2", "MVec2", false, "e0",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 0); App::SetSlotFloat(vm, 0, App::GetSlotObjectT<MVec2>(vm, 0)->GetE0()); });
	App::BindMethod("mvec2", "MVec2", false, "e1",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 0); App::SetSlotFloat(vm, 0, App::GetSlotObjectT<MVec2>(vm, 0)->GetE1()); });
	App::BindMethod("mvec2", "MVec2", false, "e2",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 0); App::SetSlotFloat(vm, 0, App::GetSlotObjectT<MVec2>(vm, 0)->GetE2()); });
	App::BindMethod("mvec2", "MVec2", false, "e12",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 0); App::SetSlotFloat(vm, 0, App::GetSlotObjectT<MVec2>(vm, 0)->GetE12()); });
	App::BindMethod("mvec2", "MVec2", false, "e0=(_)",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 1); App::GetSlotObjectT<MVec2>(vm, 0)->SetE0(App::GetSlotFloat(vm, 1)); });
	App::BindMethod("mvec2", "MVec2", false, "e1=(_)",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 1); App::GetSlotObjectT<MVec2>(vm, 0)->SetE1(App::GetSlotFloat(vm, 1)); });
	App::BindMethod("mvec2", "MVec2", false, "e2=(_)",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 1); App::GetSlotObjectT<MVec2>(vm, 0)->SetE2(App::GetSlotFloat(vm, 1)); });
	App::BindMethod("mvec2", "MVec2", false, "e12=(_)",
		[](ScriptVM* vm) { App::EnsureSlots(vm, 1); App::GetSlotObjectT<MVec2>(vm, 0)->SetE12(App::GetSlotFloat(vm, 1)); });

	App::BindMethod("mvec2", "MVec2", false, "-",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z2) MVec2(-(*z1));
		});

	App::BindMethod("mvec2", "MVec2", false, "~",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z2) MVec2(~(*z1));
		});

	App::BindMethod("mvec2", "MVec2", false, "dual",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z2) MVec2(z1->GetDual());
		});

	App::BindMethod("mvec2", "MVec2", false, "inv",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z2) MVec2(z1->GetInverse());
		});

	App::BindMethod("mvec2", "MVec2", false, "+(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z3) MVec2((*z1) + (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "-(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z3) MVec2((*z1) - (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "%(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z3) MVec2((*z1) % (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "^(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z3) MVec2((*z1) ^ (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "*(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z3) MVec2((*z1) * (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "/(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z3) MVec2((*z1) / (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "|(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);
			auto z2 = App::GetSlotObjectT<MVec2>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z3 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z3) MVec2((*z1) | (*z2));
		});

	App::BindMethod("mvec2", "MVec2", false, "grade(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z1 = App::GetSlotObjectT<MVec2>(vm, 0);
			auto i = App::GetSlotInt(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z2) MVec2((*z1).GetGrade(i));
		});

	App::BindMethod("mvec2", "MVec2", true, "exp(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z = App::GetSlotObjectT<MVec2>(vm, 1);

			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z2 = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z2) MVec2(MVec2::Exp(*z));
		});

	App::BindMethod("mvec2", "MVec2", true, "pinf",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 0);
			App::GetVariable(vm, "mvec2", "MVec2", 0);
			auto z = App::SetSlotNewObjectT<MVec2>(vm, 0, 0);
			new (z) MVec2(MVec2::PInf());
		});

	App::BindMethod("mvec2", "MVec2", false, "draw(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z = App::GetSlotObjectT<MVec2>(vm, 0);
			z->Draw(App::GetSlotUInt(vm, 1));
		});

	App::BindMethod("mvec2", "MVec2", false, "debug(_)",
		[](ScriptVM* vm)
		{
			App::EnsureSlots(vm, 1);
			auto z = App::GetSlotObjectT<MVec2>(vm, 0);
			z->Debug(App::GetSlotString(vm, 1));
		});

	App::ParseFile("mvec2", PATH("/Pt1/mvec2.wren"));
}

int main(int argc, char** args)
{
	return App::Run(&Configure, &BindApi);
}