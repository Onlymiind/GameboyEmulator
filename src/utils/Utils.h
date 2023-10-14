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

template<size_t CAPACITY>
class StringBuffer {
public:
    StringBuffer() {
        data_[0] = '\0';
    }

    ~StringBuffer() = default;

    StringBuffer(const StringBuffer<CAPACITY>&) = delete;
    StringBuffer<CAPACITY>& operator=(const StringBuffer<CAPACITY>&) = delete;

    size_t capacity() { return CAPACITY; }
    size_t capacityWithNullChar() { return CAPACITY + 1; }
    char* data() { return data_; }

    char* begin() { return data_; }
    char* end() { return data_ + CAPACITY + 1; }

    void clear() { data_[0] = '\0'; }

    bool empty() { return data_[0] == '\0'; }

    char& operator[](size_t idx) { return data_[idx]; }

private:
    char data_[CAPACITY + 1];
};

template<typename T, size_t CAPACITY>
class RingBuffer{
public:
    RingBuffer() = default;
    static_assert(CAPACITY >= 0, "capacity must be non-zero");
    static_assert(std::is_trivially_destructible<T>::value, "T must be trivially destructible");

    template<typename... Args>
    void push_back(Args&&... args) {
        data_[current_] = T{std::forward<Args...>(args)...};
        current_ = (current_ + 1) % CAPACITY;
        if(!full_ && current_ == 0) {
            full_ = true;
        }
    }

    T* begin() { return data_; }
    T* end() { return full_ ? data_ + CAPACITY : data_ + current_; }

    void clear() { full_ = false; current_ = 0; }
    size_t size() const { return full_ ? CAPACITY : current_; }

    T& operator[](size_t idx) {
        if(full_) {
            return data_[(current_ + idx) % CAPACITY];
        } else {
            return data_[idx];
        }
    }
private:
    T data_[CAPACITY];
    size_t current_ = 0;
    bool full_ = false;
};
