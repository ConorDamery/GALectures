#include <App.hpp>

static AppConfig Configure()
{
	AppConfig config{};
	config.width = 800;
	config.height = 600;
	config.title = "GA Lectures Pt2";
	config.fullscreen = false;
	config.msaa = 8;
	config.gamefile = PATH("/Pt2/Game.wren");

	return config;
}

static void BindApi()
{
}

int main(int argc, char** args)
{
	return App::Run(&Configure, &BindApi);
}