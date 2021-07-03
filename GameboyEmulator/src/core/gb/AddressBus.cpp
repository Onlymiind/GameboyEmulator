#include "core/gb/AddressBus.h"
#include "utils/Utils.h"

#include <string>
#include <sstream>
#include <exception>

namespace gb {


	uint8_t AddressBus::read(uint16_t address)
	{
		auto it = std::find_if(m_Memory.begin(), m_Memory.end(), [address](const MemoryController& controller) { return controller.isInRange(address); });

		if (it == m_Memory.end())
		{
			throw std::out_of_range(getErrorDescription(address));
		}

		return it->read(address);
	}

	void AddressBus::write(uint16_t address, uint8_t data)
	{
		auto it = std::find_if(m_Memory.begin(), m_Memory.end(), [address](const MemoryController& controller) { return controller.isInRange(address); });

		if (it == m_Memory.end())
		{
			throw std::out_of_range(getErrorDescription(address, data));
		}

		it->write(address, data);
	}
	std::string AddressBus::getErrorDescription(uint16_t address, int value)
	{
		std::stringstream err;
		
		if (value == -1)
		{
			err << "Attempting to read from invalid memory address: ";
			toHexOutput(err, address);
		}
		else
		{
			err << "Attempting to write to invalid memory address: ";
			toHexOutput(err, address);
			err << ". Data: ";
			toHexOutput(err, value);
		}

		return err.str();
	}
}