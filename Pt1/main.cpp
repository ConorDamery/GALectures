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
	config.gamefile = PATH("/Pt1/game.wren");

	return config;
}

static void BindApi()
{
	App::ParseFile("utils", PATH("/Pt1/utils.wren"));
}

int main(int argc, char** args)
{
	return App::Run(&Configure, &BindApi);
}