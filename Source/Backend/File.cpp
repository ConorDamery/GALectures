#include <App.hpp>

#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;

#include <fstream>
#include <sstream>
#include <vector>

struct FileGlobal
{
	std::vector<FileInfo> index{};
	std::vector<FileInfo> manifest{};
};
static FileGlobal g;

bool App::FileInitialize(const AppConfig& config)
{
	std::string indexSrc = FileLoad("Assets/index.txt");
	std::istringstream stream(indexSrc);
	std::string line;

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

const std::vector<FileInfo>& App::FileGetIndex()
{
	return g.index;
}

const std::vector<FileInfo>& App::FileGetManifest()
{
	return g.manifest;
}

FileInfo App::FileGetInfo(const char* filepath)
{
	FileInfo info;
	info.path = filepath;

	// Find last '/' or '\' (for both Windows and Unix-style paths)
	SizeType lastSlash = info.path.find_last_of("/\\");
	SizeType lastDot = info.path.find_last_of('.');

	// Extract filename and extension
	if (lastSlash != std::string::npos)
		info.name = info.path.substr(lastSlash + 1, (lastDot != std::string::npos ? lastDot - lastSlash - 1 : std::string::npos));
	else
		info.name = info.path.substr(0, lastDot); // No folder, just the name

	if (lastDot != std::string::npos && lastDot > lastSlash)
		info.ext = info.path.substr(lastDot + 1);

	// Hash values
	static std::hash<std::string> g_hash;
	info.pathHash = g_hash(info.path);
	info.nameHash = g_hash(info.name);
	info.extHash = g_hash(info.ext);

	return info;
}

const char* App::FilePath(const char* filepath)
{
#if _DEBUG
	thread_local std::string pathStr{};
	pathStr = PROJECT_PATH + std::string(filepath);
	return pathStr.c_str();
#else
	return filepath;
#endif
}

std::string App::FileLoad(const char* filepath)
{
	const char* path = FilePath(filepath);
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

void App::FileSave(const char* filepath, const std::string& src)
{
	const char* path = FilePath(filepath);
	std::ofstream file(path);
	if (!file.is_open())
	{
		LOGW("Failed to save file: %s", filepath);
		return;
	}

	file << src;
	file.close();
}