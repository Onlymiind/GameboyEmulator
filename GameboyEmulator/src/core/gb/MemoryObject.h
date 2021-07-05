#pragma once
#include <cstdint>

namespace gb
{
	class MemoryObject
	{
	public:
		virtual uint8_t read(uint16_t address) = 0;
		virtual void write(uint16_t address, uint8_t data) = 0;
	}
}












