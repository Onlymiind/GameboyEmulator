#ifndef GB_EMULATOR_SRC_UTIL_UTIL_HDR_
#define GB_EMULATOR_SRC_UTIL_UTIL_HDR_

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

inline std::vector<uint8_t> readFile(const std::filesystem::path &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    auto size = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> contents(static_cast<size_t>(size));
    file.read(reinterpret_cast<char *>(contents.data()), size);
    return contents;
}

template <typename T, typename std::enable_if<std::is_integral<T>{}, bool>::type = true>
inline void toHexOutput(std::stringstream &stream, T value) {
    stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +value;
}

template <typename... Args>
std::string printToString(std::string_view separator, Args... args) {
    std::stringstream stream;
    ((stream << separator << args), ...);
    return stream.str();
}

template <typename... Args>
class Variant : public std::variant<Args...> {
  public:
    using Base = std::variant<Args...>;

    using Base::variant;
    using Base::operator=;

    template <typename T>
    bool is() const {
        return std::holds_alternative<T>(*this);
    }

    template <typename T>
    T &get() {
        return std::get<T>(*this);
    }

    template <typename T>
    T *get_if() {
        return std::get_if<T>(this);
    }

    template <typename T>
    const T &get() const {
        return std::get<T>(*this);
    }

    template <typename T>
    const T *get_if() const {
        return std::get_if<T>(this);
    }

    bool empty() const { return std::holds_alternative<std::monostate>(*this); }
};

template <size_t CAPACITY>
class StaticStringBuffer {
  public:
    StaticStringBuffer() { data_[0] = '\0'; }

    ~StaticStringBuffer() = default;

    StaticStringBuffer(const StaticStringBuffer<CAPACITY> &) = delete;
    StaticStringBuffer<CAPACITY> &operator=(const StaticStringBuffer<CAPACITY> &) = delete;

    size_t capacity() { return CAPACITY; }
    size_t capacityWithNullChar() { return CAPACITY + 1; }
    char *data() { return data_; }

    char *begin() { return data_; }
    char *end() { return data_ + CAPACITY + 1; }

    void clear() { data_[0] = '\0'; }

    bool empty() { return data_[0] == '\0'; }

    char &operator[](size_t idx) { return data_[idx]; }

  private:
    char data_[CAPACITY + 1];
};

template <typename T, size_t CAPACITY>
class RingBuffer {
  public:
    RingBuffer() = default;
    static_assert(CAPACITY >= 0, "capacity must be non-zero");
    static_assert(std::is_trivially_destructible<T>::value, "T must be trivially destructible");

    template <typename... Args>
    void push_back(Args &&...args) {
        data_[current_] = T{std::forward<Args...>(args)...};
        current_ = (current_ + 1) % CAPACITY;
        if (!full_ && current_ == 0) {
            full_ = true;
        }
    }

    T *begin() { return data_; }
    T *end() { return full_ ? data_ + CAPACITY : data_ + current_; }

    void clear() {
        full_ = false;
        current_ = 0;
    }
    size_t size() const { return full_ ? CAPACITY : current_; }

    T &operator[](size_t idx) {
        if (full_) {
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

template <typename T, size_t CAPACITY>
class Queue {
  public:
    static_assert(CAPACITY >= 0, "capacity must be non-zero");
    static_assert(std::is_trivially_destructible<T>::value, "T must be trivially destructible");

    Queue() = default;

    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }

    T front() const {
        if (empty()) {
            throw std::runtime_error("attempting to pop_front from empty queue");
        }
        return data_[begin_];
    }

    T pop_front() {
        if (empty()) {
            throw std::runtime_error("attempting to pop_front from empty queue");
        }
        T elem = data_[begin_];
        begin_ = (begin_ + 1) % CAPACITY;
        --size_;
        return elem;
    }

    void push_back(T elem) {
        if (size_ == CAPACITY) {
            throw std::runtime_error("attempting to push_back to full queue");
        }
        data_[end_] = elem;
        end_ = (end_ + 1) % CAPACITY;
        ++size_;
    }

  private:
    T data_[CAPACITY];
    size_t begin_ = 0;
    size_t end_ = 0;
    size_t size_ = 0;
};

constexpr inline uint8_t setBit(uint8_t bit) { return uint8_t(1) << bit; }

class StringBuffer {
  public:
    StringBuffer() = default;
    StringBuffer(size_t capacity) : data_(new char[capacity + 1]), capacity_(capacity + 1) {}
    ~StringBuffer() { delete[] data_; }

    StringBuffer(const StringBuffer &) = delete;
    StringBuffer(StringBuffer &&other) noexcept {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
    }

    StringBuffer &operator=(const StringBuffer &) = delete;
    StringBuffer &operator=(StringBuffer &&other) noexcept {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    const char *data() const { return data_; }

    void reserveAndClear(size_t size) {
        if (capacity_ < (size + 1)) {
            char *data = new char[size + 1];
            delete[] data_;
            data_ = data;
            capacity_ = size + 1;
        }
        data_[0] = '\0';
        size_ = 0;
    }

    StringBuffer &put(char c) {
        ensure(1);
        uncheckedPut(c);
        return *this;
    }

    StringBuffer &putString(std::string_view str) {
        ensure(str.size());
        // TODO: memcpy might be faster
        for (char c : str) {
            uncheckedPut(c);
        }
        return *this;
    }

    StringBuffer &putU8(uint8_t val) {
        ensure(2);
        putHexDigit(val / 0x10);
        putHexDigit(val % 0x10);
        return *this;
    }

    StringBuffer &putU16(uint16_t val) {
        ensure(4);
        putHexDigit(uint8_t(val >> 12));
        putHexDigit(uint8_t((val & 0x0F00) >> 8));
        putHexDigit(uint8_t((val & 0x00F0) >> 4));
        putHexDigit(uint8_t(val % 0x10));
        return *this;
    }

    StringBuffer &putSigned(int8_t val) {
        ensure(4);
        if (val < 0) {
            uncheckedPut('-');
            val = int8_t(-val);
        }
        uncheckedPut('0' + val / 100);
        uncheckedPut('0' + val / 10);
        uncheckedPut('0' + val % 10);
        return *this;
    }

    StringBuffer &putBool(bool b) {
        ensure(1);
        uncheckedPut('0' + uint8_t(b));
        return *this;
    }

    void finish() { data_[size_] = '\0'; }

  private:
    void ensure(size_t length) {
        if (capacity_ - size_ < length + 1) {
            throw std::runtime_error("not enough space in the buffer");
        }
    }

    void uncheckedPut(char c) {
        data_[size_] = c;
        ++size_;
    }

    void putHexDigit(uint8_t digit) {
        if (digit < 10) {
            uncheckedPut('0' + digit);
        } else {
            uncheckedPut('a' + (digit - 10));
        }
    }

    char *data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename T, size_t CAPACITY>
class StaticVector {
  public:
    static_assert(std::is_aggregate_v<T>);
    StaticVector() = default;

    T &operator[](size_t idx) {
        if (idx >= size_) [[unlikely]] {
            throw std::out_of_range("out of bounds access");
        }

        return elems_[idx];
    }

    T *begin() { return &elems_[0]; }
    T *end() { return &elems_[0] + size_; }

    const T *begin() const { return &elems_[0]; }
    const T *end() const { return &elems_[0] + size_; }

    size_t size() const { return size_; }
    size_t capacity() const { return CAPACITY; }

    bool empty() const { return size_ == 0; }

    void push_back(T elem) {
        if (size_ >= CAPACITY) {
            throw std::out_of_range("pushing into full StaticVector");
        }

        elems_[size_] = elem;
        ++size_;
    }

  private:
    T elems_[CAPACITY];
    size_t size_ = 0;
};

#endif
