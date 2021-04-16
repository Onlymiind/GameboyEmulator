#include "core/gb/CPU.h"

#include <climits>

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
	uint8_t SharpSM83::LDH(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::LD_REG(const opcode code)
	{
		if (code.y == 6) write(REG.HL, *m_TableREG8[code.z]);      // LD [HL], reg8[code.z]
		else if (code.z == 6) *m_TableREG8[code.y] = read(REG.HL); // LD reg8[code.z], [HL]
		else *m_TableREG8[code.y] = *m_TableREG8[code.z];          // LD reg8[code.y], reg8[code.z]

		return 0;
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

				write(REG.HL, value);

				REG.Flags.Z = value == 0;
			}
			else { // INC reg8[code.y]
				REG.Flags.H = halfCarryOccured8Add(*m_TableREG8[code.y], 1);

				++(*m_TableREG8[code.y]);

				REG.Flags.Z = *m_TableREG8[code.y] == 0;
			}

			break;
		}
		}
		return 0;
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

				write(REG.HL, value);
				REG.Flags.Z = value == 0;
			}
			else //DEC reg8[code.y]
			{
				REG.Flags.H = halfCarryOccured8Sub(*m_TableREG8[code.y], 1);

				--(*m_TableREG8[code.y]);

				REG.Flags.Z = *m_TableREG8[code.y] == 0;
			}

			break;
		}
		}

		return 0;
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
			}
			else { //ADD A, reg8[code.z]
				REG.Flags.H = halfCarryOccured8Add(REG.A, *m_TableREG8[code.z]);
				REG.Flags.C = carryOccured8Add(REG.A, *m_TableREG8[code.z]);

				REG.A += *m_TableREG8[code.z];

				REG.Flags.Z = REG.A == 0;
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

				REG.Flags.Z = REG.A == 0;
			}
			break;
		}
		}
		return 0;
	}
	uint8_t SharpSM83::ADC(const opcode code)
	{
		REG.Flags.N = 0;
		uint8_t value{ 0 };

		switch (code.x) {
		case 2: {
			if (code.z == 6) { value = read(REG.HL); } // ADC [HL]
			else { value = *m_TableREG8[code.z]; }     // ADC reg8[code.z]
			break;
		}
		case 3: { value = fetch(); break; } // ADC d8
		}

		REG.Flags.H = halfCarryOccured8Add(REG.A, value) || halfCarryOccured8Add(REG.A, (value + REG.Flags.C));
		REG.Flags.C = carryOccured8Add(REG.A, value) || carryOccured8Add(REG.A, (value + REG.Flags.C));

		REG.A += value + REG.Flags.C;

		REG.Flags.Z = REG.A == 0;
		return 0;
	}
	uint8_t SharpSM83::SUB(const opcode code)
	{
		REG.Flags.N = 1;
		uint8_t value{ 0 };

		switch (code.x) {
		case 2: {
			if (code.z == 6) { value = read(REG.HL); } // SUB [HL]
			else { value = *m_TableREG8[code.z]; }     // SUB reg8[code.z]
			break;
		}
		case 3: { value = fetch(); break; } // SUB d8
		}

		REG.Flags.H = halfCarryOccured8Sub(REG.A, value);
		REG.Flags.C = carryOccured8Sub(REG.A, value);

		REG.A -= value;

		REG.Flags.Z = REG.A == 0;

		return 0;
	}
	uint8_t SharpSM83::SBC(const opcode code)
	{
		REG.Flags.N = 1;
		uint8_t value{ 0 };

		switch (code.x) {
		case 2: {
			if (code.z == 6) { value = read(REG.HL); } // SBC [HL]
			else { value = *m_TableREG8[code.z]; }     // SBC reg8[code.z]
			break;
		}
		case 3: { value = fetch(); break; } // SBC d8
		}

		REG.Flags.H = halfCarryOccured8Sub(REG.A, value) || halfCarryOccured8Sub(REG.A, (value + REG.Flags.C));
		REG.Flags.C = carryOccured8Sub(REG.A, value) || carryOccured8Sub(REG.A, (value + REG.Flags.C));

		REG.A -= value + REG.Flags.C;

		REG.Flags.Z = REG.A == 0;
		return 0;
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

		REG.Flags.Z = REG.A == 0;

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

		REG.Flags.Z = REG.A == 0;

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

		REG.Flags.Z = REG.A == 0;

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
		REG.Flags.Z = (REG.A - value) == 0;

		return 0;
	}
	uint8_t SharpSM83::JP(const opcode code)
	{
		switch (code.z) {
		case 1: {REG.PC = REG.HL; break; } // JP HL
		case 2: { //JP cond[code.y], a16
			uint16_t address = fetch();
			address |= static_cast<uint16_t>(fetch()) << 8;

			if (m_TableConditions[code.y](REG.Flags.Value)) 
			{
				REG.PC = address;
				return 1;
			}
			break;
		}
		case 3: { // JP a16
			uint16_t address = fetch();
			address |= static_cast<uint16_t>(fetch()) << 8;
			REG.PC = address;
			break;
		}
		}
		return 0;
	}
	uint8_t SharpSM83::JR(const opcode code)
	{
		uint8_t relAddress = fetch();

		if (code.y == 3) // JR r8
		{
			REG.PC += relAddress;
		}
		else // JR cond[code.y - 4], r8
		{
			if (m_TableConditions[code.y - 4](REG.Flags.Value))
			{
				REG.PC += relAddress;
				return 1;
			}
		}

		return 0;
	}
	uint8_t SharpSM83::PUSH(const opcode code)
	{
		pushStack(*m_TableREGP_AF[code.p]);
		return 0;
	}
	uint8_t SharpSM83::POP(const opcode code)
	{
		*m_TableREGP_AF[code.p] = popStack();
		return 0;
	}
	uint8_t SharpSM83::RST(const opcode code)
	{
		pushStack(REG.PC);
		REG.PC = static_cast<uint16_t>(code.y * 8);
		return 0;
	}
	uint8_t SharpSM83::CALL(const opcode code)
	{
		uint16_t address{ fetch() };
		address |= static_cast<uint16_t>(fetch()) << 8;

		switch (code.z) {
		case 4: { // CALL cond[code.y], a16
			if (m_TableConditions[code.y](REG.Flags.Value))
			{
				pushStack(REG.PC);
				REG.PC = address;
				return 3;
			}
			break;
		}
		case 5: { // CALL a16
			pushStack(REG.PC);
			REG.PC = address;
			break;
		}
		}
		return 0;
	}
	uint8_t SharpSM83::RET(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RETI(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::DI(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::EI(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::HALT(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::STOP(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::DAA(const opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::CPL(const opcode code)
	{
		REG.Flags.N = 1;
		REG.Flags.H = 1;
		REG.A = ~REG.A;
		return 0;
	}
	uint8_t SharpSM83::CCF(const opcode code)
	{
		REG.Flags.Z = 0;
		REG.Flags.N = 0;
		REG.Flags.C ^= 1;
		return 0;
	}
	uint8_t SharpSM83::SCF(const opcode code)
	{
		REG.Flags.Z = 0;
		REG.Flags.N = 0;
		REG.Flags.C = 1;
		return 0;
	}
	uint8_t SharpSM83::RLA(const opcode code)
	{
		uint8_t firstBit = REG.Flags.C;
		REG.Flags.Value = 0;
		REG.Flags.C = (REG.A & 0x80) != 0;
		REG.A = (REG.A << 1) | firstBit;

		return 0;
	}
	uint8_t SharpSM83::RRA(const opcode code)
	{
		uint8_t lastBit = static_cast<uint8_t>(REG.Flags.C) << 7;
		REG.Flags.Value = 0;
		REG.Flags.C = (REG.A & 0x01) != 0;
		REG.A = (REG.A >> 1) | lastBit;
		return 0;
	}
	uint8_t SharpSM83::RLCA(const opcode code)
	{
		REG.Flags.Value = 0;
		REG.Flags.C = (REG.A & 0x80) != 0;
		REG.A = (REG.A << 1) | (REG.A >> (sizeof(uint8_t) * CHAR_BIT - 1));
		REG.Flags.Z = REG.A == 0;

		return 0;
	}
	uint8_t SharpSM83::RRCA(const opcode code)
	{
		REG.Flags.Value = 0;
		REG.Flags.C = (REG.A & 0x80) != 0;
		REG.A = (REG.A >> 1) | (REG.A << (sizeof(uint8_t) * CHAR_BIT - 1));
		REG.Flags.Z = REG.A == 0;

		return 0;
	}
	uint8_t SharpSM83::RLC(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.z == 6) // RLC [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x80) != 0;
			value = (value << 1) | (value >> (sizeof(uint8_t) * CHAR_BIT - 1)); // (value << n) | (value >> (BIT_COUNT - n))
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else //RLC reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z] & 0x80) != 0;
			*m_TableREG8[code.z] = (*m_TableREG8[code.z] << 1) | (*m_TableREG8[code.z] >> (sizeof(uint8_t) * CHAR_BIT - 1)); // (value << n) | (value >> (BIT_COUNT - n))
			REG.Flags.Z = *m_TableREG8[code.z] == 0;
			return 2;
		}
	}
	uint8_t SharpSM83::RRC(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.z == 6) // RRC [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x01) != 0;
			value = (value >> 1) | (value << (sizeof(uint8_t) * CHAR_BIT - 1)); // (value >> n) | (value << (BIT_COUNT - n))
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else //RRC reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z] & 0x01) != 0;
			*m_TableREG8[code.z] = (*m_TableREG8[code.z] >> 1) | (*m_TableREG8[code.z] << (sizeof(uint8_t) * CHAR_BIT - 1)); // (value >> n) | (value << (BIT_COUNT - n))
			REG.Flags.Z = *m_TableREG8[code.z] == 0;
			return 2;
		}
	}
	uint8_t SharpSM83::RL(const opcode code)
	{
		uint8_t firstBit = REG.Flags.C;
		REG.Flags.Value = 0;

		if (code.z == 6) // RL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x80) != 0;
			value = (value << 1) | firstBit;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // RL reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z] & 0x80) != 0;
			*m_TableREG8[code.z] = (*m_TableREG8[code.z] << 1) | firstBit;
			REG.Flags.Z = *m_TableREG8[code.z] == 0;
			return 2;
		}
	}
	uint8_t SharpSM83::RR(const opcode code)
	{
		uint8_t lastBit = static_cast<uint8_t>(REG.Flags.C) << 7;
		REG.Flags.Value = 0;

		if (code.z == 6) // RL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x01) != 0;
			value = (value >> 1) | lastBit;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // RL reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z] & 0x01) != 0;
			*m_TableREG8[code.z] = (*m_TableREG8[code.z] >> 1) | lastBit;
			REG.Flags.Z = *m_TableREG8[code.z] == 0;
			return 2;
		}
	}
	uint8_t SharpSM83::SLA(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.z == 6) // SLA [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x80) != 0;
			value <<= 1;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // SLA reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z] & 0x80) != 0;
			*m_TableREG8[code.z] <<= 1;
			REG.Flags.Z = *m_TableREG8[code.z] == 0;
			return 2;
		}
	}
	uint8_t SharpSM83::SRA(const opcode code)
	{
		REG.Flags.Value = 0;
		uint8_t firstBit{ 0 };

		if (code.z == 6) // SRA [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x01) != 0;
			firstBit = value & 0x80;
			value >>= 1;
			value |= firstBit;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // SRA reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z] & 0x01) != 0;
			firstBit = (*m_TableREG8[code.z]) & 0x80;
			*m_TableREG8[code.z] >>= 1;
			*m_TableREG8[code.z] |= firstBit;
			REG.Flags.Z = *m_TableREG8[code.z] == 0;
			return 2;
		}
	}
	uint8_t SharpSM83::SWAP(const opcode code)
	{
		REG.Flags.Value = 0;
		uint8_t temp{ 0 };
		uint8_t result{ 0 };

		if (code.z == 6) // SWAP [HL]
		{
			uint8_t value = read(REG.HL);
			temp = value & 0xF0;
			temp >>= 4;
			value <<= 4;
			value |= temp;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // SWAP reg8[code.z]
		{
			temp = (*m_TableREG8[code.z]) & 0xF0;
			*m_TableREG8[code.z] <<= 4;
			temp >>= 4;
			*m_TableREG8[code.z] |= temp;
			REG.Flags.Z = *m_TableREG8[code.z] == 0;
			return 2;
		}
	}
	uint8_t SharpSM83::SRL(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.z == 6) // SRL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x01) != 0;
			value >>= 1;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // SRL reg8[code.z]
		{
			REG.Flags.C = (*m_TableREG8[code.z] & 0x01) != 0;
			*m_TableREG8[code.z] >>= 1;
			REG.Flags.Z = *m_TableREG8[code.z] == 0;
			return 2;
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
			result = read(REG.HL) & (1 << code.y);
			REG.Flags.Z = result == 0;
			return 3; 
		}
		else //BIT n, reg8[code.z]
		{
			result = *m_TableREG8[code.z] & (1 << code.y);
			REG.Flags.Z = result == 0;
			return 2;
		}
	}
	uint8_t SharpSM83::RES(const opcode code)
	{
		uint8_t mask = ~(1 << code.y);

		if (code.z == 6) // RES n, [HL]
		{ 
			write(REG.HL, (read(REG.HL) & mask));
			return 4;
		}
		else // RES n, reg8[code.z]
		{ 
			*m_TableREG8[code.z] &= mask;
			return 2;
		}
	}
	uint8_t SharpSM83::SET(const opcode code)
	{
		uint8_t mask = 1 << code.y;

		if (code.z == 6) // SET n, [HL]
		{
			write(REG.HL, (read(REG.HL) | mask));
			return 4;
		}
		else // SET n, reg8[code.z]
		{
			*m_TableREG8[code.z] |= mask;
			return 2;
		}
	}
}