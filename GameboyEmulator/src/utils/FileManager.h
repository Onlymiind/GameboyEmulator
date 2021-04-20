#pragma once
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>


namespace FileManager {

	inline std::vector<uint8_t> readFile(const std::string& fileName)
	{
		std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::ate);
		if (!file.is_open()) return {};

		auto size = file.tellg();
		file.seekg(0);

		std::vector<uint8_t> contents(static_cast<size_t>(size));
		file.read(reinterpret_cast<char*>(contents.data()), size);

		return contents;
	}

}
