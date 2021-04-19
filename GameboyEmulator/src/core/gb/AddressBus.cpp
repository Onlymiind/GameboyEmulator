#include "core/gb/AddressBus.h"

namespace gbemu {


	uint8_t AddressBus::read(uint16_t address)
	{
		auto it = std::find_if(m_Memory.begin(), m_Memory.end(), [address](const MemoryController& controller) { return controller.isInRange(address); });

		if (it == m_Memory.end()) return 0x00;
		return it->read(address);
	}

	void AddressBus::write(uint16_t address, uint8_t data)
	{
		auto it = std::find_if(m_Memory.begin(), m_Memory.end(), [address](const MemoryController& controller) { return controller.isInRange(address); });

		if (it != m_Memory.end()) it->write(address, data);
	}
}