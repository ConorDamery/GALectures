#include <App.hpp>

#include <iostream>

static AppConfig Configure()
{
	AppConfig config{};
	config.width = 800;
	config.height = 600;
	config.title = "GA Lectures Pt1";
	config.fullscreen = true;
	config.msaa = 8;
	config.gamefile = PATH("/Pt1/Examples/ExC2.wren");

	return config;
}

static void BindApi()
{
	App::ParseFile("utils", PATH("/Common/utils.wren"));
}

int main(int argc, char** args)
{
	return App::Run(&Configure, &BindApi);
}