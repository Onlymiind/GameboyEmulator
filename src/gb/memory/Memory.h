#pragma once

#include <type_traits>
#include <cstdint>

namespace gb {

    class MemoryObject {
    public:
        virtual void onRead(uint16_t address, uint8_t data) {};
        virtual void onWrite(uint16_t address, uint8_t data) {};
        virtual uint16_t minAddress() const = 0;
        virtual uint16_t maxAddress() const = 0;

        bool isInRange(uint16_t address) const { return address >= minAddress() && address <= maxAddress(); }
    };
}
