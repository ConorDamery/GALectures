#include <App.hpp>

static void BindApi();

static AppConfig Configure()
{
	AppConfig config{};
	config.width = 800;
	config.height = 600;
	config.title = "PGA";
	config.windowMode = WindowMode::WINDOW;
	config.msaa = 8;

	static const char* paths[] =
	{
		"Assets/PGA/Scripts/main.wren"
	};
	config.paths = paths;
	config.pathCount = ARRAY_SIZE(paths);

	config.bindApiFn = BindApi;

	return config;
}

static void BindApi()
{
	App::WrenParseFile("pga", "Assets/PGA/Scripts/pga.wren");
}

int main(int argc, char** args)
{
	return App::Run(&Configure);
}