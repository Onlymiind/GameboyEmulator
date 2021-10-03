#include "Utils.h"

#include <cstdint>
#include <fstream>

std::vector<uint8_t> readFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		return {};
	}
	auto size = file.tellg();
	file.seekg(0);
	
	std::vector<uint8_t> contents(static_cast<size_t>(size));
	file.read(reinterpret_cast<char*>(contents.data()), size);
	return contents;
}