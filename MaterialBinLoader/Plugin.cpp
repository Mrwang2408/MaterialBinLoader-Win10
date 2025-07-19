#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <Windows.h>

#include "Hook/Hook.h"
#include "Hook/MemoryUtils.h"
#include "Plugin.h"
#include <map>
using namespace std::filesystem;
std::unordered_map<std::string, std::string> BinList;

std::string GetLocalAppDataPath() {
	char* pValue;
	size_t len;
	errno_t err = _dupenv_s(&pValue, &len, "LOCALAPPDATA");
	if (err) {
		return "";
	}
	std::string path = pValue;
	free(pValue);

	return path;
}

std::string& replaceAll(std::string& str, const std::string& old_value, const std::string& new_value) {
	while (true) {
		std::string::size_type pos(0);
		if ((pos = str.find(old_value)) != std::string::npos)
			str.replace(pos, old_value.length(), new_value);
		else break;
	}
	return str;
}

std::string GetMCBEPath() {
	std::string path = GetLocalAppDataPath();
	if (path.find("\\Packages\\microsoft.minecraftuwp_8wekyb3d8bbwe\\AC") != std::string::npos) {
		replaceAll(path, "\\Packages\\microsoft.minecraftuwp_8wekyb3d8bbwe\\AC", "");
		path += "\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\LocalState\\games\\com.mojang\\";
	}
	else {
		replaceAll(path, "\\Packages\\microsoft.minecraftwindowsbeta_8wekyb3d8bbwe\\AC", "");
		path += "\\Packages\\Microsoft.MinecraftWindowsBeta_8wekyb3d8bbwe\\LocalState\\games\\com.mojang\\";
	}
	return path;
}

std::string UTF82String(std::u8string str) {
	return reinterpret_cast<std::string&>(str);
}

void ReadBin() {
	std::filesystem::directory_iterator ent(GetMCBEPath() + "renderer\\materials");
	for (auto& file : ent) {
		if (!file.is_regular_file())
			continue;
		auto& path = file.path();
		auto fileName = UTF82String(path.filename().u8string());

		std::string ext = UTF82String(path.extension().u8string());
		std::string parentPath = UTF82String(path.parent_path().u8string());
		std::string paths = parentPath + "\\" + fileName;
		BinList[fileName] = paths;

	}
}

void CreateConsole()
{
	if (!AllocConsole()) {
		return;
	}
	SetConsoleCP(CP_UTF8);
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();

	HANDLE hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hConIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
	SetStdHandle(STD_ERROR_HANDLE, hConOut);
	SetStdHandle(STD_INPUT_HANDLE, hConIn);
	std::wcout.clear();
	std::wclog.clear();
	std::wcerr.clear();
	std::wcin.clear();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {
		//CreateConsole();
		ReadBin();
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FIND_ADDR(Ver,Sig)                            \
    {void* ptr = ll::memory::resolveSignature(Sig);    \
     if (ptr) { return ptr;  }}              


void* findAddr(std::string name) {	
	switch (do_hash(name.c_str()))
	{
	case do_hash("ResourcePackManager::ResourcePackManager"): {
		//FIND_ADDR("1.21.60", "4C 8B DC 49 89 5B 18 49 89 53 10 49 89 4B 08 55 56 57 41 56 41 57 48 83 EC 70 41 0F B6 E9 4D 8B F0 48 8B F2 48 8B F9 49 8D 43 98 49 89 43 88 45");
		FIND_ADDR("1.21.50", "4C 8B DC 53 55 56 57 41 54 41 56 41 57 48 81 EC A0 00 00 00 41 0F B6 E9");
		FIND_ADDR("1.21.60", "4C 8B DC 49 89 5B ? 49 89 53 ? 49 89 4B ? 55 56 57 41 56");
		////////////////////////error/////////////////////
		CreateConsole();
		std::cout << "ResourcePackManager::ResourcePackManager address not found!!!" << std::endl;
	}
	case do_hash("AppPlatform::readAssetFile"): {
		//FIND_ADDR("1.21.60", "48 89 5C 24 08 48 89 7C 24 18 55 48 8D 6C 24 A0 48 81 EC 60 01 00 00 48 8B 05 62 11 30 08 48 33 C4 48 89 45 50 48 8B FA 48 89 55 B0 0F 57 C9 F3");
		FIND_ADDR("1.21.50", "48 89 5C 24 ? 55 56 57 48 8D 6C 24 ? 48 81 EC 50 01 00 00 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 49 8B F0 48 8B FA 48 89 55 ? 0F 57 C9 F3 0F 7F 4D ? 0F B6 5C 24 ? 80 E3 A1 80 CB 21 BA 28 00 00 00 65 48 8B 04 25 ? ? ? ? 48 8B 08 8B 04 0A 39 05 ? ? ? ? 0F 8F ? ? ? ? 48 8B 05 ? ? ? ? C7 44 24 ? ? ? ? ? 88 5C 24 ? 4C 8D");
		FIND_ADDR("1.21.60", "48 89 5C 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC 60 01 00 00 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 48 8B FA");
		////////////////////////error/////////////////////
		CreateConsole();
		std::cout << "AppPlatform::readAssetFile address not found!!!" << std::endl;
		break;
	}
	default:
		break;
	}
	return nullptr;
}

ResourcePackManager* GlobalResourcePackManager = nullptr;

LL_AUTO_STATIC_HOOK(
	HOOK0,
	HookPriority::Normal,
	findAddr("ResourcePackManager::ResourcePackManager"),
	ResourcePackManager* , ResourcePackManager* a1, __int64 a2, void* a3, char a4
){
	if(GlobalResourcePackManager == nullptr && a4){
		GlobalResourcePackManager = a1;
	}
	return origin(a1, a2, a3, a4);
}

LL_AUTO_STATIC_HOOK(
	Hook1,
	HookPriority::Normal,
	findAddr("AppPlatform::readAssetFile"),
	std::string*, void* _this, std::string* a2, Core::Path* a3
) {
	auto& data = a3->mPath.mUtf8StdString;
	if (data.size() < 32) {
		return origin(_this, a2, a3);
	}

	if (data.find("data/renderer/materials/") != std::string::npos && strncmp(data.c_str() + data.size() - 13, ".material.bin", 13) == 0) {
		std::string str = data.substr(data.find_last_of('/') + 1);
		std::string* resourceStream = new std::string();
		if (GlobalResourcePackManager) {
			auto result = GlobalResourcePackManager->load(*new ResourceLocation("renderer/materials/" + str), *resourceStream);
			if (!result) {
				//std::cout << "Failure location=" << str << std::endl;
				auto it = BinList.find(str);
				if (it != BinList.end()) {
					std::string path = it->second;
					a3->mPath.mUtf8StdString = path;
				}
			}
			else {
				//std::cout << "Success location=" << str <<" len:"<< resourceStream->length() << std::endl;
				origin(_this, a2, a3);
				a2->clear();
				*a2 = *resourceStream;
				return a2;
			}
		}
	}
	return origin(_this, a2, a3);
}