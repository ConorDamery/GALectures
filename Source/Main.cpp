#include <App.hpp>

int main(int argc, char** args)
{
	AppConfig config{};
	config.width = 800;
	config.height = 600;
	config.title = "GA Sandbox";
	config.windowMode = WindowMode::WINDOWED;
	config.msaa = 8;
	config.headless = false;

	return App::Run(config);
}