#include "core/gb/cpu/CPU.h"
#include "core/gb/AddressBus.h"
#include "utils/Utils.h"

#include <climits>
#include <iostream>
#include <cstdint>
#include <string_view>
#include <sstream>
#include <string>
#include <array>
#include <functional>


namespace gbemu {


	SharpSM83::SharpSM83(AddressBus& bus) :
		m_Bus(bus), m_CyclesToFinish(0)
	{}


	void SharpSM83::tick()
	{
		if (m_CyclesToFinish == 0) {

			opcode code = fetch();
			//TODO: HALT state (with bug)
			//TODO: Check if opcode is invalid
			if (code.code == 0xCB) 
			{
				code.code = fetch();
				switch (code.getX()) {
				case 0: {
					m_CyclesToFinish = m_TableBitOperations[code.getY()](this, code);
					break;
				}
				case 1: {
					m_CyclesToFinish = BIT(code);
					break;
				}
				case 2: {
					m_CyclesToFinish = RES(code);
					break;
				}
				case 3: {
					m_CyclesToFinish = SET(code);
					break;
				}
				}
			}
			else 
			{
				//std::cout << m_TableLookup[code.code].Mnemonic << "\n";
				m_CyclesToFinish = m_TableLookup[code.code].MachineCycles + m_TableLookup[code.code].Implementation(this, code);
			}

		}

		if (m_EnableIME) { // Enable jumping to interrupt vectors if enabling it is scheduled by EI
			IME = true;
			m_EnableIME = false;
		}

		--m_CyclesToFinish;
	}

	std::string SharpSM83::registersOut()
	{
		std::stringstream stream{};
		
		stream << "CPU registers:\n";

		stream << "A: "; toHexOutput(stream, REG.A); stream << " F: "; toHexOutput(stream, static_cast<uint8_t>(REG.AF & 0x00FF)); stream << "\n";

		stream << "B: "; toHexOutput(stream, REG.B); stream << " C: "; toHexOutput(stream, REG.C); stream << "\n";
		stream << "D: "; toHexOutput(stream, REG.D); stream << " E: "; toHexOutput(stream, REG.E); stream << "\n";
		stream << "H: "; toHexOutput(stream, REG.H); stream << " L: "; toHexOutput(stream, REG.L); stream << "\n";

		stream << "SP: "; toHexOutput(stream, REG.SP); stream << "\n";
		stream << "PC: "; toHexOutput(stream, REG.PC); stream << "\n";

		return stream.str();
	}

	bool SharpSM83::halfCarryOccured8Add(uint8_t lhs, uint8_t rhs)
	{
		return (((lhs & 0x0F) + (rhs & 0x0F)) & 0x10) != 0;
	}

	bool SharpSM83::halfCarryOccured8Sub(uint8_t lhs, uint8_t rhs)
	{
		return (lhs & 0x0F) < (rhs & 0x0F);
	}

	bool SharpSM83::carryOccured8Add(uint8_t lhs, uint8_t rhs)
	{
		uint16_t lhs16{ lhs }, rhs16{ rhs };

		return (lhs16 + rhs16) > 0x00FF;
	}

	bool SharpSM83::carryOccured8Sub(uint8_t lhs, uint8_t rhs)
	{
		return lhs < rhs;
	}

	bool SharpSM83::halfCarryOccured16Add(uint16_t lhs, uint16_t rhs)
	{
		return (((lhs & 0x0FFF) + (rhs & 0x0FFF)) & 0x1000) != 0;
	}

	bool SharpSM83::carryOccured16Add(uint16_t lhs, uint16_t rhs)
	{
		uint32_t lhs32{ lhs }, rhs32{ rhs };

		return (lhs32 + rhs32) > 0xFFFF;
	}

	void SharpSM83::write(uint16_t address, uint8_t data)
	{
		m_Bus.write(address, data);
	}

	uint8_t SharpSM83::read(uint16_t address)
	{
		return m_Bus.read(address);
	}

	void SharpSM83::pushStack(uint16_t value)
	{
		uint8_t msb{ 0 }, lsb{ 0 };

		lsb = static_cast<uint8_t>(value & 0x00FF);
		msb = static_cast<uint8_t>((value & 0xFF00) >> 8);

		--REG.SP;
		write(REG.SP, msb);
		--REG.SP;
		write(REG.SP, lsb);
	}

	uint8_t SharpSM83::fetch()
	{
		uint8_t value = read(REG.PC);
		++REG.PC;
		return value;
	}

	uint16_t SharpSM83::fetchWord()
	{
		uint8_t lsb{ 0 }, msb{ 0 };
		uint16_t value;
		lsb = fetch();
		msb = fetch();
		value = (static_cast<uint16_t>(msb) << 8) | (static_cast<uint16_t>(lsb));
		return value;
	}

	uint16_t SharpSM83::popStack()
	{
		uint8_t msb{ 0 }, lsb{ 0 };
		lsb = read(REG.SP);
		++REG.SP;
		msb = read(REG.SP);
		++REG.SP;
		uint16_t result = (static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb);
		return result;
	}

	int8_t SharpSM83::fetchSigned()
	{
		uint8_t u_value = fetch();
		int8_t result = reinterpret_cast<int8_t&>(u_value);
		return result;
	}

}