#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <vector>

namespace gb {

    class RAM {
    public:
        RAM(size_t size)
            : memory_(size)
        {}

        ~RAM() = default;

        uint8_t read(uint16_t address) const { return memory_[address]; }
        void write(uint16_t address, uint8_t data) { memory_[address] = data; }
    private:
        std::vector<uint8_t> memory_;
    };

    //TODO: cartridge RAM, mapper chips
    class Cartridge {
    public:
        Cartridge() = default;
        Cartridge(std::vector<uint8_t> rom)
            : rom_(std::move(rom))
        {}

        void setROM(std::vector<uint8_t> rom) { rom_ = std::move(rom); }
        uint8_t read(uint16_t address) const { return rom_[address]; }
        void write(uint16_t address, uint8_t data) { /*do nothing*/ }

        bool hasRAM() const { return !ram_.empty(); }
        bool hasROM() const { return !rom_.empty(); }

    private:
        std::vector<uint8_t> rom_;
        std::vector<uint8_t> ram_;
    };

    template<size_t SIZE>
    using StaticRAM = std::array<uint8_t, SIZE>;
}
