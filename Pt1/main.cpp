#include <App.hpp>

static AppConfig Configure()
{
	return AppConfig{};
}

static void PostInitialize()
{
}

int main(int argc, char** args)
{
	return App::Run(&Configure, &PostInitialize);
}