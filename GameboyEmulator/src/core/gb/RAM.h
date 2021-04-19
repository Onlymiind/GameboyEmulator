#pragma once
#include <cstdint>
#include <array>
#define KBYTE 1024


namespace gbemu {



	class RAM {
	public:
		RAM() :
			m_Memory() 
		{}
		~RAM() = default;

		inline uint8_t read(uint16_t address) { return m_Memory[address]; }
		inline void write(uint16_t address, uint8_t data) { m_Memory[address] = data; }
	private:
		std::array<uint8_t, 64 * KBYTE> m_Memory;
	};
}