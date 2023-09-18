#pragma once
#include "gb/memory/Memory.h"

#include <cstdint>
#include <string>
#include <vector>


namespace gb 
{
    class AddressBus 
    {
    public:
        AddressBus()
        {}

        ~AddressBus() = default;

        void connect(const MemoryController& controller);
        void addObserver(const MemoryController& observer);

        size_t getObjectCount() const { return memory_.size(); }

        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data) const;

    private:
        std::string getErrorDescription(uint16_t address, int value = -1) const;

        std::vector<MemoryController> memory_;
        std::vector<MemoryController> observers_;
    };

}
