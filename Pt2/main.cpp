#include <App.hpp>

static void BindApi();

static AppConfig Configure()
{
	AppConfig config{};
	config.width = 800;
	config.height = 600;
	config.title = "GA Lectures Pt2";
	config.windowMode = WindowMode::WINDOW;
	config.msaa = 8;

	static const char* paths[] =
	{
		"Scripts/s1.wren"
	};
	config.paths = paths;
	config.pathCount = ARRAY_SIZE(paths);

	config.bindApiFn = BindApi;

	return config;
}

static void BindApi()
{
	App::WrenParseFile("pga", "Scripts/pga.wren");
}

int main(int argc, char** args)
{
	return App::Run(&Configure);
}