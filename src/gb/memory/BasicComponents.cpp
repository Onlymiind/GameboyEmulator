#include "gb/memory/BasicComponents.h"

namespace gb
{
    uint8_t RAM::read(uint16_t address) const
    {
        return memory_[address];
    }

	void RAM::write(uint16_t address, uint8_t data)
    {
        memory_[address] = data;
    }

    void ROM::setData(std::vector<uint8_t> rom)
    { 
        memory_ = std::move(rom);
    }

	uint8_t ROM::read(uint16_t address) const
    {
        return memory_[address];
    }

	void ROM::write(uint16_t address, uint8_t data)
    {
        /*Do nothing*/
    }
}