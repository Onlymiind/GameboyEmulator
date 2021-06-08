#pragma once
#include <cstdint>
#include <vector>
#define KBYTE 1024


namespace gbemu {



	class RAM {
	public:
		RAM(uint16_t firstAddress, uint16_t lastAddress) :
			m_Memory(static_cast<size_t>(lastAddress) - firstAddress + 1), m_FirstAddress(firstAddress), m_LastAddress(lastAddress)
		{}
		~RAM() = default;

		inline uint8_t read(uint16_t address) { return m_Memory[address - m_FirstAddress]; }
		inline void write(uint16_t address, uint8_t data) { m_Memory[address - m_FirstAddress] = data; }
	private:
		std::vector<uint8_t> m_Memory;
		uint16_t m_FirstAddress;
		uint16_t m_LastAddress;
	};
}