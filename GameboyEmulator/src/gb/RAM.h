#pragma once
#include "gb/MemoryObject.h"



#include <cstdint>
#include <cstddef>
#include <vector>

#define KBYTE 1024


namespace gb {



	class RAM:public MemoryObject {
	public:
		RAM(size_t size):
			m_Memory(size)
		{}
		~RAM() = default;

		inline uint8_t read(uint16_t address) override { return m_Memory[address]; }
		inline void write(uint16_t address, uint8_t data) override { m_Memory[address] = data; }
	private:
		std::vector<uint8_t> m_Memory;
	};
}
