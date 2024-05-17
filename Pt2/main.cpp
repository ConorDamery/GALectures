#include <App.hpp>

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
	App::BindMethod("mvec", "MVec", true, "test(_)", [](ScriptVM* vm)
		{
			std::cout << "Hello";
		});

	App::ParseFile("mvec", PATH("/Pt2/mvec.wren"));
}

int main(int argc, char** args)
{
	return App::Run(&Configure, &BindApi);
}