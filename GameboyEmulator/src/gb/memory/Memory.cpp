#include "gb/memory/Memory.h"

namespace gb
{
    uint8_t MemoryController::read(uint16_t address) const
    { 
        return memory_object_.read(address - min_address_);
    }

	void MemoryController::write(uint16_t address, uint8_t data) const
    { 
        memory_object_.write(address - min_address_, data);
    }

	bool MemoryController::isInRange(uint16_t address) const
    { 
        return address >= min_address_ && address <= max_address_; 
    }

	uint16_t MemoryController::GetMinAddress() const
    {
        return min_address_;
    }

	uint16_t MemoryController::GetMaxAddress() const 
    {
        return max_address_;
    }

	bool MemoryController::operator<(const MemoryController& other) const
	{
		return max_address_ < other.min_address_;
	}

	bool MemoryController::operator<(uint16_t address) const
	{
		return address > max_address_;
	}

	bool MemoryController::operator>(uint16_t address) const
	{
		return address < min_address_;
	}

    bool ControllerComparator::operator()(const MemoryController& lhs, const MemoryController& rhs) const
	{
		return lhs < rhs;
	}

	bool ControllerComparator::operator()(const MemoryController& lhs, uint16_t rhs) const
	{
		return lhs < rhs;
	}

	bool ControllerComparator::operator()(uint16_t lhs, const MemoryController& rhs) const
	{
		return rhs > lhs;
	}
}
