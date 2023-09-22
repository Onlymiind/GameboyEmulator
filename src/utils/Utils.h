#pragma once

#include <type_traits>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <string_view>
#include <sstream>
#include <fstream>

inline std::vector<uint8_t> readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		return {};
	}
	auto size = file.tellg();
	file.seekg(0);
	
	std::vector<uint8_t> contents(static_cast<size_t>(size));
	file.read(reinterpret_cast<char*>(contents.data()), size);
	return contents;
}

template<typename T, typename std::enable_if<std::is_integral<T>{}, bool>::type = true >
inline void toHexOutput(std::stringstream& stream, T value) {
    stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +value;
}

template<typename... Args>
std::string printToString(std::string_view separator, Args... args) {
    std::stringstream stream;
    ((stream << separator << args), ...);
    return stream.str();
}
