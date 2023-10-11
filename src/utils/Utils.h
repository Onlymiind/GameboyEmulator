#pragma once

#include <type_traits>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <string_view>
#include <sstream>
#include <fstream>
#include <variant>
#include <optional>

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

template<typename... Args>
class Variant : public std::variant<Args...> {
public:
    using Base = std::variant<Args...>;

    using Base::variant;
    using Base::operator=;
    
    template<typename T>
    bool is() const { return std::holds_alternative<T>(*this); }

    template<typename T>
    T& get() { return std::get<T>(*this); }

    template<typename T>
    T* get_if() { return std::get_if<T>(this); }

    template<typename T>
    const T& get() const { return std::get<T>(*this); }

    template<typename T>
    const T* get_if() const { return std::get_if<T>(this); }

    bool empty() const { return std::holds_alternative<std::monostate>(*this); }
};

template<size_t size>
class StringBuffer{
public:
private:
};
