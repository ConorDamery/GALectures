#include <App.hpp>

#include <iostream>

static AppConfig Configure()
{
	AppConfig config{};
	config.width = 800;
	config.height = 600;
	config.title = "GA Lectures Pt1";
	config.fullscreen = false;
	config.msaa = 8;
	config.gamefile = PATH("/Pt1/Game.wren");

	return config;
}

static void BindApi()
{
	App::BindMethod("mvec", "MVec", true, "test(_)", [](ScriptVM* vm)
		{
			std::cout << "Hello";
		});

	App::ParseFile("mvec", PATH("/Pt1/MVec.wren"));
}

int main(int argc, char** args)
{
	return App::Run(&Configure, &BindApi);
}