#pragma once
#include "core/gb/MemoryController.h"

#include <cstdint>
#include <vector>
#include <functional>
#include <string>


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
		std::string getErrorDescription(uint16_t address, int value = -1);

		std::vector<MemoryController> m_Memory;
	};

}