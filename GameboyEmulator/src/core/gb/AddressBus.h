#pragma once
#include "core/gb/MemoryController.h"

#include <cstdint>
#include <string>
#include <set>


namespace gb {


	class AddressBus {
	public:
		AddressBus() :
			m_Memory() 
		{}

		~AddressBus() = default;

		inline void connect(const MemoryController& controller) { m_Memory.insert(controller); }

		uint8_t read(uint16_t address);
		void write(uint16_t address, uint8_t data);
	private:
		std::string getErrorDescription(uint16_t address, int value = -1);

		std::set<MemoryController, ControllerComparator> m_Memory;
	};

}
