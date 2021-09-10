#include "IO.h"

#include <array>
#include <cstdint>
#include <iostream>

namespace gb
{

	uint8_t IORegisters::read(uint16_t address)
	{
		return registers_[address];
	}
	void IORegisters::write(uint16_t address, uint8_t data)
	{
		if (address == 0x02 && data == 0x81)
		{
			std::cout << read(0x01);
		}

		registers_[address] = data;
	}
}

