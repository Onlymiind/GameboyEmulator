#pragma once
#include "core/gb/MemoryController.h"

#include <cstdint>
#include <vector>
#include <functional>


namespace gb {


	class AddressBus {
	public:
		AddressBus() :
			m_Memory() 
		{}

		~AddressBus() = default;

		inline void connect(const MemoryController& controller) { m_Memory.push_back(controller); }

		uint8_t read(uint16_t address);
		void write(uint16_t address, uint8_t data);
	private:
		std::vector<MemoryController> m_Memory;
	};

}