#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#define KBYTE 1024


namespace gb {



	class RAM {
	public:
		RAM(size_t size) :
			m_Memory(size)
		{}
		~RAM() = default;

		inline uint8_t read(uint16_t address) { return m_Memory[address]; }
		inline void write(uint16_t address, uint8_t data) { m_Memory[address] = data; }
	private:
		std::vector<uint8_t> m_Memory;
	};
}