#pragma once

#include <type_traits>
#include <cstdint>

namespace gb {

    class MemoryObject {
    public:
        virtual void onRead(uint16_t address, uint8_t data) {};
        virtual void onWrite(uint16_t address, uint8_t data) {};
    };

    class MemoryController {
    public:

        MemoryController(uint16_t minAddress, uint16_t maxAddress, MemoryObject& object)
            : min_address_(minAddress), max_address_(maxAddress), memory_object_(&object) 
        {}
        MemoryController(const MemoryController& other)
            : min_address_(other.min_address_), max_address_(other.max_address_), memory_object_(other.memory_object_)
        {}

        ~MemoryController() = default;

        void onRead(uint16_t address, uint8_t data) const { memory_object_->onRead(address, data); }
        void onWrite(uint16_t address, uint8_t data) const { memory_object_->onWrite(address, data); }

        bool isInRange(uint16_t address) const { return address >= min_address_ && address <= max_address_; }

        uint16_t getMinAddress() const { return min_address_; }
        uint16_t getMaxAddress() const { return max_address_; }

        bool operator < (const MemoryController& other) const { return max_address_ < other.min_address_; }

        bool operator < (uint16_t address) const { return address > max_address_; }

        bool operator > (uint16_t address) const { return address < min_address_; }
    private:

        uint16_t min_address_, max_address_;
        MemoryObject* memory_object_;
    };
}
