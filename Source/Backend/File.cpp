#include <App.hpp>

#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;

#include <fstream>
#include <sstream>

using namespace GASandbox;

struct FileGlobal
{
	list<sFileInfo> index{};
	list<sFileInfo> manifest{};
};
static FileGlobal g;

bool App::FileInitialize(const sAppConfig& config)
{
	string indexSrc = FileLoad("Assets/index.txt");
	std::istringstream stream(indexSrc);
	string line;

	while (std::getline(stream, line))
	{
		if (!line.empty()) // Avoid empty lines
		{
			g.index.push_back(FileGetInfo(line.c_str()));
		}
	}

	fs::path assetPath{ FilePath("Assets") };
	for (const auto& entry : fs::recursive_directory_iterator(assetPath))
	{
		if (fs::is_regular_file(entry))
		{
			fs::path relativePath = fs::relative(entry.path(), assetPath);
			auto path = "Assets/" + relativePath.generic_string();
			g.manifest.emplace_back(FileGetInfo(path.c_str()));
		}
	}

	return true;
}

void App::FileShutdown()
{
}

const list<sFileInfo>& App::FileGetIndex()
{
	return g.index;
}

const list<sFileInfo>& App::FileGetManifest()
{
	return g.manifest;
}

sFileInfo App::FileGetInfo(cstring filepath)
{
	sFileInfo info;
	info.path = filepath;

	// Find last '/' or '\' (for both Windows and Unix-style paths)
	size_type lastSlash = info.path.find_last_of("/\\");
	size_type lastDot = info.path.find_last_of('.');

	// Extract filename and extension
	if (lastSlash != string::npos)
		info.name = info.path.substr(lastSlash + 1, (lastDot != string::npos ? lastDot - lastSlash - 1 : string::npos));
	else
		info.name = info.path.substr(0, lastDot); // No folder, just the name

	if (lastDot != string::npos && lastDot > lastSlash)
		info.ext = info.path.substr(lastDot + 1);

	// Hash values
	static std::hash<string> g_hash;
	info.pathHash = g_hash(info.path);
	info.nameHash = g_hash(info.name);
	info.extHash = g_hash(info.ext);

	return info;
}

cstring App::FilePath(cstring filepath)
{
#if _DEBUG
	thread_local string pathStr{};
	pathStr = PROJECT_PATH + string(filepath);
	return pathStr.c_str();
#else
	return filepath;
#endif
}

string App::FileLoad(cstring filepath)
{
	cstring path = FilePath(filepath);
	std::ifstream file(path);
	if (!file.is_open())
	{
		LOGW("Failed to open file: %s", filepath);
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return buffer.str();
}

void App::FileSave(cstring filepath, const string& src)
{
	cstring path = FilePath(filepath);
	std::ofstream file(path);
	if (!file.is_open())
	{
		LOGW("Failed to save file: %s", filepath);
		return;
	}

	file << src;
	file.close();
}