#pragma once
#include "gb/memory/Memory.h"

#include <cstdint>
#include <cstddef>
#include <vector>

namespace gb {

    class RAM : public MemoryObject {
    public:
        RAM(size_t size)
            : memory_(size)
        {}

        ~RAM() = default;

        uint8_t read(uint16_t address) const override { return memory_[address]; }
        void write(uint16_t address, uint8_t data) override { memory_[address] = data; }
    private:
        std::vector<uint8_t> memory_;
    };

    class ROM : public MemoryObject {
    public:
        ROM() = default;
        
        ROM(std::vector<uint8_t> rom) 
            : memory_(std::move(rom))
        {}

        ~ROM() = default;

        void setData(std::vector<uint8_t> rom) { memory_ = std::move(rom); }
        uint8_t read(uint16_t address) const override { return memory_[address]; }
        void write(uint16_t address, uint8_t data) override { /*do nothing*/ }
    private:
        std::vector<uint8_t> memory_;
    };
}
