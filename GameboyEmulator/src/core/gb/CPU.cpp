#include "core/gb/CPU.h"


namespace gbemu {


	SharpSM83::SharpSM83()
	{}


	void SharpSM83::tick()
	{
		const opcode code = fetch();
		//TODO: Check CB prefix
		//TODO: Check if previous instruction has finished
		//TODO: HALT state (with bug)
		//TODO: Check if opcode is invalid

		m_TableLookup[code.code].Implementation(this, code);

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
	uint8_t SharpSM83::NONE(const opcode code)
	{
		return 0;
	}
	uint8_t SharpSM83::NOP(const opcode code)
	{
		return 0;
	}
	uint8_t SharpSM83::LD(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::INC(const opcode code)
	{
		switch (code.z) {
		case 3: { // INC reg16[code.p]
			++(*m_TableREGP_SP[code.p]);
			break;
		}
		case 5: {
			REG.Flags.N = 0;
			if (code.y == 6) // INC [HL]
			{
				uint8_t value = read(REG.HL);
				REG.Flags.H = halfCarryOccured8Add(value, 1);

				++value;

				REG.Flags.Z = (value == 0);

				write(REG.HL, value);
			}
			else { // INC reg8[code.y]
				REG.Flags.H = halfCarryOccured8Add(*m_TableREG8[code.y], 1);

				++(*m_TableREG8[code.y]);

				REG.Flags.Z = (*m_TableREG8[code.y] == 0);
			}

			break;
		}
		}
		return 0;
	}
	uint8_t SharpSM83::RLA(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RLCA(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::ADD(const opcode code)
	{
		REG.Flags.N = 0;

		switch (code.x) {
		case 0: { // ADD HL, reg16[code.p]
			REG.Flags.H = halfCarryOccured16Add(REG.HL, *m_TableREGP_SP[code.p]);
			REG.Flags.C = carryOccured16Add(REG.HL, *m_TableREGP_SP[code.p]);

			REG.HL += *m_TableREGP_SP[code.p];

			break;
		}
		case 2: {
			if (code.z == 6) //ADD A, [HL]
			{
				uint8_t value = read(REG.HL);
				REG.Flags.H = halfCarryOccured8Add(REG.A, value);
				REG.Flags.C = carryOccured8Add(REG.A, value);

				REG.A += value;

				REG.Flags.Z = (REG.A == 0);
				//write(REG.HL, value); - don't actually need this, might uncomment for consistency
			}
			else { //ADD A, reg8[code.z]
				REG.Flags.H = halfCarryOccured8Add(REG.A, *m_TableREG8[code.z]);
				REG.Flags.C = carryOccured8Add(REG.A, *m_TableREG8[code.z]);

				REG.A += *m_TableREG8[code.z];

				REG.Flags.Z = (REG.A == 0);
			}
			break;
		}
		case 3: {
			if (code.z == 0) //ADD SP, r8
			{
				REG.Flags.Z = 0;
				uint8_t value = fetch();

				REG.Flags.H = halfCarryOccured8Add(REG.SP & 0x00FF, value); //According to specification H flag should be set if overflow from bit 3
				REG.Flags.C = carryOccured8Add(REG.SP & 0x00FF, value); //Carry flag should be set if overflow from bit 7

				REG.SP += value;
			}
			else //ADD A, d8
			{
				uint8_t value = fetch();
				REG.Flags.H = halfCarryOccured8Add(REG.A, value);
				REG.Flags.C = carryOccured8Add(REG.A, value);

				REG.A += value;

				REG.Flags.Z = (REG.A == 0);
			}
			break;
		}
		}
		return 0;
	}
	uint8_t SharpSM83::JR(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::DEC(const opcode code)
	{
		switch (code.z) {
		case 3: { // DEC reg16[code.p]
			--(*m_TableREGP_SP[code.p]);
			break;
		}
		case 5: {
			REG.Flags.N = 1;
			if (code.y == 6) // DEC [HL]
			{
				uint8_t value = read(REG.HL);
				REG.Flags.H = halfCarryOccured8Sub(value, 1);

				--value;

				REG.Flags.Z = (value == 0);
				write(REG.HL, value);
			}
			else //DEC reg8[code.y]
			{
				REG.Flags.H = halfCarryOccured8Sub(*m_TableREG8[code.y], 1);

				--(*m_TableREG8[code.y]);

				REG.Flags.Z = (*m_TableREG8[code.y] == 0);
			}

			break;
		}
		}

		return 0;
	}
	uint8_t SharpSM83::RRA(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RRCA(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::SUB(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::OR(const opcode code)
	{
		REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of OR
		switch (code.x) {
		case 2: {
			if (code.z == 6) //OR [HL]
			{
				REG.A |= read(REG.HL);
			}
			else //OR reg8[code.z]
			{
				REG.A |= *m_TableREG8[code.z];
			}
			break;
		}
		case 3: { REG.A |= fetch(); break; } //OR d8
		}

		REG.Flags.Z = (REG.A == 0);

		return 0;
	}
	uint8_t SharpSM83::AND(const opcode code)
	{
		REG.Flags.Value = 0;
		REG.Flags.H = 1;
		switch (code.x) {
		case 2: {
			if (code.z == 6) //AND [HL]
			{
				REG.A &= read(REG.HL);
			}
			else //AND reg8[code.z]
			{
				REG.A &= *m_TableREG8[code.z];
			}
			break;
		}
		case 3: { REG.A &= fetch(); break; } //AND d8
		}

		REG.Flags.Z = (REG.A == 0);

		return 0;
	}
	uint8_t SharpSM83::XOR(const opcode code)
	{
		REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of XOR
		switch (code.x) {
		case 2: {
			if (code.z == 6) //XOR [HL]
			{
				REG.A ^= read(REG.HL);
			}
			else //XOR reg8[code.z]
			{
				REG.A ^= *m_TableREG8[code.z];
			}
			break;
		}
		case 3: { REG.A ^= fetch(); break; } //XOR d8
		}

		REG.Flags.Z = (REG.A == 0);

		return 0;
	}
	uint8_t SharpSM83::CP(const opcode code)
	{
		uint8_t value{ 0 };

		switch (code.x) {
		case 2: {
			if (code.z == 6) //CP [HL]
			{
				value = read(REG.HL);
			}
			else //CP reg8[code.z]
			{
				value = *m_TableREG8[code.z];
			}
			break;
		}
		case 3: { value = fetch(); break; } //CP d8
		}

		REG.Flags.N = 1;
		REG.Flags.H = halfCarryOccured8Sub(REG.A, value);
		REG.Flags.C = carryOccured8Sub(REG.A, value);
		REG.Flags.Z = ((REG.A - value) == 0);

		return 0;
	}
	uint8_t SharpSM83::PUSH(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::ADC(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::JP(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::POP(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RST(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::CALL(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::SBC(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::DI(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RET(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::CPL(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RETI(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::CCF(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::EI(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::LDH(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::DAA(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::HALT(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::SCF(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::STOP(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::LD_REG(const opcode code)
	{
		if (code.y == 6) write(REG.HL, *m_TableREG8[code.z]);      // LD [HL], reg8[code.z]
		else if (code.z == 6) *m_TableREG8[code.y] = read(REG.HL); // LD reg8[code.z], [HL]
		else *m_TableREG8[code.y] = (*m_TableREG8[code.z]);        // LD reg8[code.y], reg8[code.z]

		return 0;
	}
	uint8_t SharpSM83::RLC(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RRC(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RL(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.z == 6) // RL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = value & 0x80;
			value <<= 1;
			value |= REG.Flags.C;
			write(REG.HL, value);
			REG.Flags.Z = (value == 0);
			return 16;
		}
		else // RL reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z]) & 0x80;
			*m_TableREG8[code.z] <<= 1;
			*m_TableREG8[code.z] |= REG.Flags.C;
			REG.Flags.Z = (*m_TableREG8[code.z] == 0);
			return 8;
		}
	}
	uint8_t SharpSM83::RR(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.z == 6) // RL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = value & 0x01;
			value >>= 1;
			value |= static_cast<uint8_t>(REG.Flags.C) << 7;
			write(REG.HL, value);
			REG.Flags.Z = (value == 0);
			return 16;
		}
		else // RL reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z]) & 0x01;
			*m_TableREG8[code.z] >>= 1;
			*m_TableREG8[code.z] |= static_cast<uint8_t>(REG.Flags.C) << 7;
			REG.Flags.Z = (*m_TableREG8[code.z] == 0);
			return 8;
		}
	}
	uint8_t SharpSM83::SLA(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::SRA(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::SWAP(const opcode code)
	{
		REG.Flags.Value = 0;
		uint8_t temp{ 0 };
		uint8_t result{ 0 };

		if (code.z == 6) // SWAP [HL]
		{
			uint8_t value = read(REG.HL);
			temp = value & 0xFF00;
			temp >>= 4;
			value <<= 4;
			value |= temp;
			write(REG.HL, value);
			REG.Flags.Z = (value == 0);
			return 16;
		}
		else // SWAP reg8[code.z]
		{
			temp = (*m_TableREG8[code.z]) & 0xFF00;
			*m_TableREG8[code.z] <<= 4;
			temp >>= 4;
			*m_TableREG8[code.z] |= temp;
			REG.Flags.Z = (*m_TableREG8[code.z] == 0);
			return 8;
		}
	}
	uint8_t SharpSM83::SRL(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.z == 6) // SRL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = ((value & 0x01) != 0);
			value >>= 1;
			write(REG.HL, value);
			REG.Flags.Z = (value == 0);
			return 16;
		}
		else // SRL reg8[code.z]
		{
			REG.Flags.C = (((*m_TableREG8[code.z]) & 0x01) != 0);
			*m_TableREG8[code.z] >>= 1;
			REG.Flags.Z = (*m_TableREG8[code.z] == 0);
			return 8;
		}

		return uint8_t();
	}
	uint8_t SharpSM83::BIT(const opcode code)
	{
		REG.Flags.N = 0;
		REG.Flags.H = 1;

		uint8_t result{ 0 };

		if (code.z == 6) // BIT n, [HL]
		{
			result = read(REG.HL) & (UINT8_C(1) << code.y);
			REG.Flags.Z = (result == 0);
			return 12; 
		}
		else //BIT n, reg8[code.z]
		{
			result = (*m_TableREG8[code.z] & (UINT8_C(1) << code.y));
			REG.Flags.Z = (result == 0);
			return 8;
		}
	}
	uint8_t SharpSM83::RES(const opcode code)
	{
		uint8_t mask = ~(UINT8_C(1) << code.y);

		if (code.z == 6) // RES n, [HL]
		{ 
			write(REG.HL, (read(REG.HL) & mask));
			return 16;
		}
		else // RES n, reg8[code.z]
		{ 
			*m_TableREG8[code.z] &= mask;
			return 8;
		}
	}
	uint8_t SharpSM83::SET(const opcode code)
	{
		uint8_t mask = UINT8_C(1) << code.y;

		if (code.z == 6) // SET n, [HL]
		{
			write(REG.HL, (read(REG.HL) | mask));
			return 16;
		}
		else // SET n, reg8[code.z]
		{
			*m_TableREG8[code.z] |= mask;
			return 8;
		}
	}
}