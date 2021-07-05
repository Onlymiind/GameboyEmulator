#include "core/gb/AddressBus.h"
#include "utils/Utils.h"

#include <string>
#include <sstream>


namespace gb {


	uint8_t AddressBus::read(uint16_t address)
	{
		auto it = m_Memory.find(address);
		if (it != m_Memory.end())
		{
			return it->read(address);
		}
		return 0x00;

	}

	void AddressBus::write(uint16_t address, uint8_t data)
	{
		auto it = m_Memory.find(address);
		if (it != m_Memory.end())
		{
			it->write(address, data);
		}

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
