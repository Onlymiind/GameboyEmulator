#include "core/gb/cpu/CPU.h"
#include "utils/Utils.h"

#include <climits>
#include <iostream>
#include <cstdint>


namespace gbemu {


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
		switch (code.getX()) {
		case 0: {
			switch (code.getQ()) {
			case 0: {
				switch (code.getP()) {
				case 0: { // LD [BC], A
					write(REG.BC, REG.A);
					break;
				}
				case 1: { // LD [DE], A
					write(REG.DE, REG.A);
					break;
				}
				case 2: { // LD [HLI], A
					write(REG.HL, REG.A);
					++REG.HL;
					break;
				}
				case 3: { // LD [HLD], A
					write(REG.HL, REG.A);
					--REG.HL;
					break;
				}
				}
				break;
			}
			case 1: {
				switch (code.getP()) {
				case 0: { // LD A, [BC]
					REG.A = read(REG.BC);
					break;
				}
				case 1: { // LD A, [DE]
					REG.A = read(REG.DE);
					break;
				}
				case 2: { // LD A, [HLI]
					REG.A = read(REG.HL);
					++REG.HL;
					break;
				}
				case 3: { // LD A, [HLD]
					REG.A = read(REG.HL);
					--REG.HL;
					break;
				}
				}
				break;
			}
			}
			break;
		}
		case 3: {
			switch (code.getZ()) {
			case 1: { // LD SP, HL

				REG.SP = REG.HL;

				break;
			}
			case 2: {
				uint16_t address = fetchWord();

				switch (code.getY()) {
				case 5: { // LD [a16], A
					write(address, REG.A);
					break;
				}
				case 7: { // LD A, [a16]
					REG.A = read(address);
					break;
				}
				}
				break;
			}
			}

			break;
		}
		}


		return 0;
	}

	uint8_t SharpSM83::LD_IMM(const opcode code)
	{
		switch (code.getX()) {
		case 0: {
			switch (code.getZ()) {
			case 0: { //LD [a16], SP
				uint16_t address = fetchWord();

				write(address, REG.SP & 0x00FF);
				++address;
				write(address, (REG.SP & 0xFF00) >> 8);
				break;
			}
			case 1: {                     // LD reg16[code.p], d16
				uint16_t value = fetchWord();

				*m_TableREGP_SP[code.getP()] = value;
				break;
			}
			case 6: {
				uint8_t value = fetch();

				if (code.getY() == 6) write(REG.HL, value); // LD [HL], d8
				else *m_TableREG8[code.getY()] = value;     // LD reg8[code.y], d8

				break;
			}
			}

			break;
		}
		case 3: { // LD HL, SP + r8
			int8_t offset = fetchSigned();

			REG.Flags.H = halfCarryOccured8Add(REG.SP & 0x00FF, offset);
			REG.Flags.C = carryOccured8Add(REG.SP & 0x00FF, offset);
			REG.Flags.Z = 0;
			REG.Flags.N = 0;

			REG.HL = REG.SP + offset;
			break;
		}
		}

		return 0;
	}

	uint8_t SharpSM83::LD_IO(const opcode code)
	{
		switch (code.getZ()) {
		case 0: {
			uint8_t address = fetch();
			if (code.getY() == 4) write(0xFF00 + address, REG.A); // LD [$FF00 + a8], A
			else REG.A = read(0xFF00 + address);             // LD A, [$FF00 + a8]
			break;
		}
		case 2: {
			if (code.getY() == 4) write(0xFF00 + REG.C, REG.A);   // LD [$FF00 + C], A
			else REG.A = read(0xFF00 + REG.C);               // LD A, [$FF00 + C]
			break;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::LD_REG8(const opcode code)
	{
		if (code.getY() == 6) write(REG.HL, *m_TableREG8[code.getZ()]);      // LD [HL], reg8[code.getZ()]
		else if (code.getZ() == 6) *m_TableREG8[code.getY()] = read(REG.HL); // LD reg8[code.getY()], [HL]
		else *m_TableREG8[code.getY()] = *m_TableREG8[code.getZ()];          // LD reg8[code.getY()], reg8[code.getZ()]

		return 0;
	}

	uint8_t SharpSM83::INC(const opcode code)
	{
		switch (code.getZ()) {
		case 3: { // INC reg16[code.getP()]
			++(*m_TableREGP_SP[code.getP()]);
			break;
		}
		case 4: {
			REG.Flags.N = 0;
			if (code.getY() == 6) // INC [HL]
			{
				uint8_t value = read(REG.HL);

				REG.Flags.H = halfCarryOccured8Add(value, 1);

				++value;

				write(REG.HL, value);

				REG.Flags.Z = value == 0;
			}
			else { // INC reg8[code.getY()]
				REG.Flags.H = halfCarryOccured8Add(*m_TableREG8[code.getY()], 1);

				++(*m_TableREG8[code.getY()]);

				REG.Flags.Z = *m_TableREG8[code.getY()] == 0;
			}

			break;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::DEC(const opcode code)
	{
		switch (code.getZ()) {
		case 3: { // DEC reg16[code.getP()]
			--(*m_TableREGP_SP[code.getP()]);
			break;
		}
		case 5: {
			REG.Flags.N = 1;
			if (code.getY() == 6) // DEC [HL]
			{
				uint8_t value = read(REG.HL);
				REG.Flags.H = halfCarryOccured8Sub(value, 1);

				--value;

				write(REG.HL, value);
				REG.Flags.Z = value == 0;
			}
			else //DEC reg8[code.getY()]
			{
				REG.Flags.H = halfCarryOccured8Sub(*m_TableREG8[code.getY()], 1);

				--(*m_TableREG8[code.getY()]);

				REG.Flags.Z = *m_TableREG8[code.getY()] == 0;
			}

			break;
		}
		}

		return 0;
	}

	uint8_t SharpSM83::ADD(const opcode code)
	{
		REG.Flags.N = 0;

		switch (code.getX()) {
		case 0: { // ADD HL, reg16[code.getP()]
			REG.Flags.H = halfCarryOccured16Add(REG.HL, *m_TableREGP_SP[code.getP()]);
			REG.Flags.C = carryOccured16Add(REG.HL, *m_TableREGP_SP[code.getP()]);

			REG.HL += *m_TableREGP_SP[code.getP()];

			break;
		}
		case 2: {
			if (code.getZ() == 6) //ADD A, [HL]
			{
				uint8_t value = read(REG.HL);
				REG.Flags.H = halfCarryOccured8Add(REG.A, value);
				REG.Flags.C = carryOccured8Add(REG.A, value);

				REG.A += value;

				REG.Flags.Z = (REG.A == 0);
			}
			else { //ADD A, reg8[code.getZ()]
				REG.Flags.H = halfCarryOccured8Add(REG.A, *m_TableREG8[code.getZ()]);
				REG.Flags.C = carryOccured8Add(REG.A, *m_TableREG8[code.getZ()]);

				REG.A += *m_TableREG8[code.getZ()];

				REG.Flags.Z = REG.A == 0;
			}
			break;
		}
		case 3: {
			if (code.getZ() == 0) //ADD SP, r8
			{
				REG.Flags.Z = 0;
				int8_t value = fetchSigned();

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
		uint8_t regA = REG.A;

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) { value = read(REG.HL); } // ADC [HL]
			else { value = *m_TableREG8[code.getZ()]; }     // ADC reg8[code.getZ()]
			break;
		}
		case 3: { 
			value = fetch(); 
			break; } // ADC d8
		}

		REG.A += value + REG.Flags.C;
		REG.Flags.Z = REG.A == 0;
		REG.Flags.H = ((regA & 0x0F) + (value &0x0F) + REG.Flags.C) > 0x0F;
		REG.Flags.C = static_cast<uint16_t>(regA) + value + REG.Flags.C > 0xFF;


		return 0;
	}

	uint8_t SharpSM83::SUB(const opcode code)
	{
		REG.Flags.N = 1;
		uint8_t value{ 0 };

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) { value = read(REG.HL); } // SUB [HL]
			else { value = *m_TableREG8[code.getZ()]; }     // SUB reg8[code.getZ()]
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

	uint8_t SharpSM83::SBC(const opcode code) //SBC d8 fails
	{
		REG.Flags.N = 1;
		uint8_t value{ 0 };
		uint8_t regA = REG.A;

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) { value = read(REG.HL); } // SBC [HL]
			else { value = *m_TableREG8[code.getZ()]; }     // SBC reg8[code.getZ()]
			break;
		}
		case 3: { value = fetch(); break; } // SBC d8
		}

		REG.A -= value + REG.Flags.C;
		REG.Flags.Z = REG.A == 0;
		REG.Flags.H = (regA & 0x0F) < ((value & 0x0F) + REG.Flags.C);
		REG.Flags.C = regA < (static_cast<uint16_t>(value) + REG.Flags.C);


		return 0;
	}

	uint8_t SharpSM83::OR(const opcode code)
	{
		REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of OR
		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) //OR [HL]
			{
				REG.A |= read(REG.HL);
			}
			else //OR reg8[code.getZ()]
			{
				REG.A |= *m_TableREG8[code.getZ()];
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
		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) //AND [HL]
			{
				REG.A &= read(REG.HL);
			}
			else //AND reg8[code.getZ()]
			{
				REG.A &= *m_TableREG8[code.getZ()];
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
		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) //XOR [HL]
			{
				REG.A ^= read(REG.HL);
			}
			else //XOR reg8[code.getZ()]
			{
				REG.A ^= *m_TableREG8[code.getZ()];
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

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) //CP [HL]
			{
				value = read(REG.HL);
			}
			else //CP reg8[code.getZ()]
			{
				value = *m_TableREG8[code.getZ()];
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
		switch (code.getZ()) {
		case 1: {REG.PC = REG.HL; break; } // JP HL
		case 2: { //JP cond[code.getY()], a16
			uint16_t address = fetchWord();

			if (m_TableConditions[code.getY()](REG.Flags.Value))
			{
				REG.PC = address;
				return 1;
			}
			break;
		}
		case 3: { // JP a16
			uint16_t address = fetchWord();

			REG.PC = address;
			break;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::JR(const opcode code)
	{
		int8_t relAddress = fetchSigned();

		if (code.getY() == 3) // JR r8
		{
			REG.PC += relAddress;
		}
		else // JR cond[code.getY() - 4], r8
		{
			if (m_TableConditions[code.getY() - 4](REG.Flags.Value))
			{
				REG.PC += relAddress;
				return 1;
			}
		}

		return 0;
	}

	uint8_t SharpSM83::PUSH(const opcode code)
	{
		if (code.getP() == 3) { // PUSH AF
			pushStack(REG.AF & 0xFFF0);
		}
		else {
			pushStack(*m_TableREGP_AF[code.getP()]);
		}
		return 0;
	}

	uint8_t SharpSM83::POP(const opcode code)
	{
		if (code.getP() == 3) //POP AF
		{
			REG.AF = popStack() & 0xFFF0;
		}
		else { //POP regAF[code.p]
			*m_TableREGP_AF[code.getP()] = popStack();
		}

		return 0;
	}

	uint8_t SharpSM83::RST(const opcode code)
	{
		pushStack(REG.PC);
		REG.PC = static_cast<uint16_t>(code.getY() * 8);
		return 0;
	}

	uint8_t SharpSM83::CALL(const opcode code)
	{
		uint16_t address = fetchWord();

		switch (code.getZ()) {
		case 4: { // CALL cond[code.getY()], a16
			if (m_TableConditions[code.getY()](REG.Flags.Value))
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
		switch (code.getZ()) {
		case 0: { // RET cond[code.getY()]
			if (m_TableConditions[code.getY()](REG.Flags.Value))
			{
				REG.PC = popStack();
				return 3;
			}
			break;
		}
		case 1: { // RET
			REG.PC = popStack();
			break;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::RETI(const opcode code)
	{
		REG.PC = popStack();
		IME = true;
		return 0;
	}

	uint8_t SharpSM83::DI(const opcode code)
	{
		IME = false;
		m_EnableIME = false;
		return 0;
	}

	uint8_t SharpSM83::EI(const opcode code)
	{
		m_EnableIME = true;
		return 0;
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
		if (REG.Flags.N)
		{
			if (REG.Flags.C) REG.A -= 0x60;
			if (REG.Flags.H) REG.A -= 0x06;
		}
		else
		{
			if (REG.Flags.C || REG.A > 0x99)
			{
				REG.A += 0x60;
				REG.Flags.C = 1;
			}
			if (REG.Flags.H || (REG.A & 0x0F) > 0x09) REG.A += 0x06;
		}

		REG.Flags.H = 0;
		REG.Flags.Z = REG.A == 0;

		return 0;
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
		REG.Flags.N = 0;
		REG.Flags.H = 0;
		REG.Flags.C = !REG.Flags.C;
		return 0;
	}

	uint8_t SharpSM83::SCF(const opcode code)
	{
		REG.Flags.N = 0;
		REG.Flags.H = 0;
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
		REG.Flags.C = (REG.A & 0b10000000) != 0;
		REG.A = (REG.A << 1) | (REG.A >> (sizeof(uint8_t) * CHAR_BIT - 1));

		return 0;
	}

	uint8_t SharpSM83::RRCA(const opcode code)
	{
		REG.Flags.Value = 0;
		REG.Flags.C = (REG.A & 0b00000001) != 0;
		REG.A = (REG.A >> 1) | (REG.A << (sizeof(uint8_t) * CHAR_BIT - 1));

		return 0;
	}

	uint8_t SharpSM83::RLC(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.getZ() == 6) // RLC [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x80) != 0;
			value = (value << 1) | (value >> (sizeof(uint8_t) * CHAR_BIT - 1)); // (value << n) | (value >> (BIT_COUNT - n))
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else //RLC reg8[code.getZ()]
		{
			REG.Flags.C = (*m_TableREG8[code.getZ()] & 0x80) != 0;
			*m_TableREG8[code.getZ()] = (*m_TableREG8[code.getZ()] << 1) | (*m_TableREG8[code.getZ()] >> (sizeof(uint8_t) * CHAR_BIT - 1)); // (value << n) | (value >> (BIT_COUNT - n))
			REG.Flags.Z = *m_TableREG8[code.getZ()] == 0;
			return 2;
		}
	}

	uint8_t SharpSM83::RRC(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.getZ() == 6) // RRC [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x01) != 0;
			value = (value >> 1) | (value << (sizeof(uint8_t) * CHAR_BIT - 1)); // (value >> n) | (value << (BIT_COUNT - n))
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else //RRC reg8[code.getZ()]
		{
			REG.Flags.C = (*m_TableREG8[code.getZ()] & 0x01) != 0;
			*m_TableREG8[code.getZ()] = (*m_TableREG8[code.getZ()] >> 1) | (*m_TableREG8[code.getZ()] << (sizeof(uint8_t) * CHAR_BIT - 1)); // (value >> n) | (value << (BIT_COUNT - n))
			REG.Flags.Z = *m_TableREG8[code.getZ()] == 0;
			return 2;
		}
	}

	uint8_t SharpSM83::RL(const opcode code)
	{
		uint8_t firstBit = REG.Flags.C;
		REG.Flags.Value = 0;

		if (code.getZ() == 6) // RL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x80) != 0;
			value = (value << 1) | firstBit;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // RL reg8[code.getZ()]
		{
			REG.Flags.C = (*m_TableREG8[code.getZ()] & 0x80) != 0;
			*m_TableREG8[code.getZ()] = (*m_TableREG8[code.getZ()] << 1) | firstBit;
			REG.Flags.Z = *m_TableREG8[code.getZ()] == 0;
			return 2;
		}
	}

	uint8_t SharpSM83::RR(const opcode code)
	{
		uint8_t lastBit = static_cast<uint8_t>(REG.Flags.C) << 7;
		REG.Flags.Value = 0;

		if (code.getZ() == 6) // RL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x01) != 0;
			value = (value >> 1) | lastBit;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // RL reg8[code.getZ()]
		{
			REG.Flags.C = (*m_TableREG8[code.getZ()] & 0x01) != 0;
			*m_TableREG8[code.getZ()] = (*m_TableREG8[code.getZ()] >> 1) | lastBit;
			REG.Flags.Z = *m_TableREG8[code.getZ()] == 0;
			return 2;
		}
	}

	uint8_t SharpSM83::SLA(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.getZ() == 6) // SLA [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x80) != 0;
			value <<= 1;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // SLA reg8[code.getZ()]
		{
			REG.Flags.C = (*m_TableREG8[code.getZ()] & 0x80) != 0;
			*m_TableREG8[code.getZ()] <<= 1;
			REG.Flags.Z = *m_TableREG8[code.getZ()] == 0;
			return 2;
		}
	}

	uint8_t SharpSM83::SRA(const opcode code)
	{
		REG.Flags.Value = 0;
		uint8_t firstBit{ 0 };

		if (code.getZ() == 6) // SRA [HL]
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
		else // SRA reg8[code.getZ()]
		{
			REG.Flags.C = (*m_TableREG8[code.getZ()] & 0x01) != 0;
			firstBit = (*m_TableREG8[code.getZ()]) & 0x80;
			*m_TableREG8[code.getZ()] >>= 1;
			*m_TableREG8[code.getZ()] |= firstBit;
			REG.Flags.Z = *m_TableREG8[code.getZ()] == 0;
			return 2;
		}
	}

	uint8_t SharpSM83::SWAP(const opcode code)
	{
		REG.Flags.Value = 0;
		uint8_t temp{ 0 };
		uint8_t result{ 0 };

		if (code.getZ() == 6) // SWAP [HL]
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
		else // SWAP reg8[code.getZ()]
		{
			temp = (*m_TableREG8[code.getZ()]) & 0xF0;
			*m_TableREG8[code.getZ()] <<= 4;
			temp >>= 4;
			*m_TableREG8[code.getZ()] |= temp;
			REG.Flags.Z = *m_TableREG8[code.getZ()] == 0;
			return 2;
		}
	}

	uint8_t SharpSM83::SRL(const opcode code)
	{
		REG.Flags.Value = 0;

		if (code.getZ() == 6) // SRL [HL]
		{
			uint8_t value = read(REG.HL);
			REG.Flags.C = (value & 0x01) != 0;
			value >>= 1;
			write(REG.HL, value);
			REG.Flags.Z = value == 0;
			return 4;
		}
		else // SRL reg8[code.getZ()]
		{
			REG.Flags.C = (*m_TableREG8[code.getZ()] & 0x01) != 0;
			*m_TableREG8[code.getZ()] >>= 1;
			REG.Flags.Z = *m_TableREG8[code.getZ()] == 0;
			return 2;
		}

		return uint8_t();
	}

	uint8_t SharpSM83::BIT(const opcode code)
	{
		REG.Flags.N = 0;
		REG.Flags.H = 1;

		uint8_t result{ 0 };

		if (code.getZ() == 6) // BIT n, [HL]
		{
			result = read(REG.HL) & (1 << code.getY());
			REG.Flags.Z = result == 0;
			return 3;
		}
		else //BIT n, reg8[code.getZ()]
		{
			result = *m_TableREG8[code.getZ()] & (1 << code.getY());
			REG.Flags.Z = result == 0;
			return 2;
		}
	}

	uint8_t SharpSM83::RES(const opcode code)
	{
		uint8_t mask = ~(1 << code.getY());

		if (code.getZ() == 6) // RES n, [HL]
		{
			write(REG.HL, (read(REG.HL) & mask));
			return 4;
		}
		else // RES n, reg8[code.getZ()]
		{
			*m_TableREG8[code.getZ()] &= mask;
			return 2;
		}
	}

	uint8_t SharpSM83::SET(const opcode code)
	{
		uint8_t mask = 1 << code.getY();

		if (code.getZ() == 6) // SET n, [HL]
		{
			write(REG.HL, (read(REG.HL) | mask));
			return 4;
		}
		else // SET n, reg8[code.getZ()]
		{
			*m_TableREG8[code.getZ()] |= mask;
			return 2;
		}
	}
}