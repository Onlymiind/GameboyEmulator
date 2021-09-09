#pragma once
#include "gb/MemoryObject.h"


#include <array>
#include <cstdint>

namespace gb
{
	class IORegisters:public MemoryObject
	{
	public:
		IORegisters() = default;
		~IORegisters() = default;

		uint8_t read(uint16_t address) override;
		void write(uint16_t address, uint8_t data) override;

	private:

		std::array<uint8_t, 0x80> m_Registers;
	};
}



