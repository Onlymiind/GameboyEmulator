#pragma once
#include <array>
#include <cstdint>

namespace gb
{
	class IORegisters
	{
	public:
		IORegisters() = default;
		~IORegisters() = default;

		uint8_t read(uint16_t address);
		void write(uint16_t address, uint8_t data);

	private:

		std::array<uint8_t, 0x7F> m_Registers;
	};
}