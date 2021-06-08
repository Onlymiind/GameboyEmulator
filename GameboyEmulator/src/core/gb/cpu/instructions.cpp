#include "core/gb/cpu/CPU.h"
#include "utils/Utils.h"

#include <climits>
#include <iostream>
#include <cstdint>


namespace gb {


	uint8_t SharpSM83::NONE(SharpSM83& cpu, const opcode code)
	{
		return 0;
	}

	uint8_t SharpSM83::NOP(SharpSM83& cpu, const opcode code)
	{
		return 1;
	}

	uint8_t SharpSM83::LD(SharpSM83& cpu, const opcode code)
	{
		switch (code.getX()) {
		case 0: {
			switch (code.getQ()) {
			case 0: {
				switch (code.getP()) {
				case 0: { // LD [BC], A
					cpu.write(cpu.REG.BC, cpu.REG.A);
					return 2;
				}
				case 1: { // LD [DE], A
					cpu.write(cpu.REG.DE, cpu.REG.A);
					return 2;
				}
				case 2: { // LD [HLI], A
					cpu.write(cpu.REG.HL, cpu.REG.A);
					++cpu.REG.HL;
					return 2;
				}
				case 3: { // LD [HLD], A
					cpu.write(cpu.REG.HL, cpu.REG.A);
					--cpu.REG.HL;
					return 2;
				}
				}
				break;
			}
			case 1: {
				switch (code.getP()) {
				case 0: { // LD A, [BC]
					cpu.REG.A = cpu.read(cpu.REG.BC);
					return 2;
				}
				case 1: { // LD A, [DE]
					cpu.REG.A = cpu.read(cpu.REG.DE);
					return 2;
				}
				case 2: { // LD A, [HLI]
					cpu.REG.A = cpu.read(cpu.REG.HL);
					++cpu.REG.HL;
					return 2;
				}
				case 3: { // LD A, [HLD]
					cpu.REG.A = cpu.read(cpu.REG.HL);
					--cpu.REG.HL;
					return 2;
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

				cpu.REG.SP = cpu.REG.HL;

				return 2;
			}
			case 2: {
				uint16_t address = cpu.fetchWord();

				switch (code.getY()) {
				case 5: { // LD [a16], A
					cpu.write(address, cpu.REG.A);
					return 4;
				}
				case 7: { // LD A, [a16]
					cpu.REG.A = cpu.read(address);
					return 4;
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

	uint8_t SharpSM83::LD_IMM(SharpSM83& cpu, const opcode code)
	{
		switch (code.getX()) {
		case 0: {
			switch (code.getZ()) {
			case 0: { //LD [a16], SP
				uint16_t address = cpu.fetchWord();

				cpu.write(address, cpu.REG.SP & 0x00FF);
				++address;
				cpu.write(address, (cpu.REG.SP & 0xFF00) >> 8);
				return 5;
			}
			case 1: {                     // LD reg16[code.p], d16
				uint16_t value = cpu.fetchWord();

				*cpu.m_TableREGP_SP[code.getP()] = value;
				return 3;
			}
			case 6: {
				uint8_t value = cpu.fetch();

				if (code.getY() == 6)
				{
					cpu.write(cpu.REG.HL, value); // LD [HL], d8
					return 3;
				}
				else
				{
					*cpu.m_TableREG8[code.getY()] = value;     // LD reg8[code.y], d8
					return 2;
				}

				break;
			}
			}

			break;
		}
		case 3: { // LD HL, SP + r8
			int8_t offset = cpu.fetchSigned();

			cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.SP & 0x00FF, offset);
			cpu.REG.Flags.C = cpu.carryOccured8Add(cpu.REG.SP & 0x00FF, offset);
			cpu.REG.Flags.Z = 0;
			cpu.REG.Flags.N = 0;

			cpu.REG.HL = cpu.REG.SP + offset;
			return 3;
		}
		}

		return 0;
	}

	uint8_t SharpSM83::LD_IO(SharpSM83& cpu, const opcode code)
	{
		switch (code.getZ()) {
		case 0: {
			uint8_t address = cpu.fetch();
			if (code.getY() == 4) cpu.write(0xFF00 + address, cpu.REG.A); // LD [$FF00 + a8], A
			else cpu.REG.A = cpu.read(0xFF00 + address);             // LD A, [$FF00 + a8]
			return 3;
		}
		case 2: {
			if (code.getY() == 4) cpu.write(0xFF00 + cpu.REG.C, cpu.REG.A);   // LD [$FF00 + C], A
			else cpu.REG.A = cpu.read(0xFF00 + cpu.REG.C);               // LD A, [$FF00 + C]
			return 2;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::LD_REG8(SharpSM83& cpu, const opcode code)
	{
		if (code.getY() == 6)
		{
			cpu.write(cpu.REG.HL, *cpu.m_TableREG8[code.getZ()]);      // LD [HL], reg8[code.getZ()]
			return 2;
		}
		else if (code.getZ() == 6)
		{
			*cpu.m_TableREG8[code.getY()] = cpu.read(cpu.REG.HL); // LD reg8[code.getY()], [HL]
			return 2;
		}
		else
		{
			*cpu.m_TableREG8[code.getY()] = *cpu.m_TableREG8[code.getZ()]; // LD reg8[code.getY()], reg8[code.getZ()]
			return 1;
		}

		return 0;
	}

	uint8_t SharpSM83::INC(SharpSM83& cpu, const opcode code)
	{
		switch (code.getZ()) {
		case 3: { // INC reg16[code.getP()]
			++(*cpu.m_TableREGP_SP[code.getP()]);
			return 2;
		}
		case 4: {
			cpu.REG.Flags.N = 0;
			if (code.getY() == 6) // INC [HL]
			{
				uint8_t value = cpu.read(cpu.REG.HL);

				cpu.REG.Flags.H = cpu.halfCarryOccured8Add(value, 1);

				++value;

				cpu.write(cpu.REG.HL, value);

				cpu.REG.Flags.Z = value == 0;

				return 3;
			}
			else { // INC reg8[code.getY()]
				cpu.REG.Flags.H = cpu.halfCarryOccured8Add(*cpu.m_TableREG8[code.getY()], 1);

				++(*cpu.m_TableREG8[code.getY()]);

				cpu.REG.Flags.Z = *cpu.m_TableREG8[code.getY()] == 0;

				return 1;
			}

			break;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::DEC(SharpSM83& cpu, const opcode code)
	{
		switch (code.getZ()) {
		case 3: { // DEC reg16[code.getP()]
			--(*cpu.m_TableREGP_SP[code.getP()]);
			return 2;
		}
		case 5: {
			cpu.REG.Flags.N = 1;
			if (code.getY() == 6) // DEC [HL]
			{
				uint8_t value = cpu.read(cpu.REG.HL);
				cpu.REG.Flags.H = cpu.halfCarryOccured8Sub(value, 1);

				--value;

				cpu.write(cpu.REG.HL, value);
				cpu.REG.Flags.Z = value == 0;

				return 3;
			}
			else //DEC reg8[code.getY()]
			{
				cpu.REG.Flags.H = cpu.halfCarryOccured8Sub(*cpu.m_TableREG8[code.getY()], 1);

				--(*cpu.m_TableREG8[code.getY()]);

				cpu.REG.Flags.Z = *cpu.m_TableREG8[code.getY()] == 0;

				return 1;
			}

			break;
		}
		}

		return 0;
	}

	uint8_t SharpSM83::ADD(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.N = 0;

		switch (code.getX()) {
		case 0: { // ADD HL, reg16[code.getP()]
			cpu.REG.Flags.H = cpu.halfCarryOccured16Add(cpu.REG.HL, *cpu.m_TableREGP_SP[code.getP()]);
			cpu.REG.Flags.C = cpu.carryOccured16Add(cpu.REG.HL, *cpu.m_TableREGP_SP[code.getP()]);

			cpu.REG.HL += *cpu.m_TableREGP_SP[code.getP()];

			return 2;;
		}
		case 2: {
			if (code.getZ() == 6) //ADD A, [HL]
			{
				uint8_t value = cpu.read(cpu.REG.HL);
				cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.A, value);
				cpu.REG.Flags.C = cpu.carryOccured8Add(cpu.REG.A, value);

				cpu.REG.A += value;

				cpu.REG.Flags.Z = (cpu.REG.A == 0);

				return 2;
			}
			else { //ADD A, reg8[code.getZ()]
				cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.A, *cpu.m_TableREG8[code.getZ()]);
				cpu.REG.Flags.C = cpu.carryOccured8Add(cpu.REG.A, *cpu.m_TableREG8[code.getZ()]);

				cpu.REG.A += *cpu.m_TableREG8[code.getZ()];

				cpu.REG.Flags.Z = cpu.REG.A == 0;

				return 1;
			}
			break;
		}
		case 3: {
			if (code.getZ() == 0) //ADD SP, r8
			{
				cpu.REG.Flags.Z = 0;
				int8_t value = cpu.fetchSigned();

				cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.SP & 0x00FF, value); //According to specification H flag should be set if overflow from bit 3
				cpu.REG.Flags.C = cpu.carryOccured8Add(cpu.REG.SP & 0x00FF, value); //Carry flag should be set if overflow from bit 7

				cpu.REG.SP += value;

				return 4;
			}
			else //ADD A, d8
			{
				uint8_t value = cpu.fetch();
				cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.A, value);
				cpu.REG.Flags.C = cpu.carryOccured8Add(cpu.REG.A, value);

				cpu.REG.A += value;

				cpu.REG.Flags.Z = cpu.REG.A == 0;

				return 2;
			}
			break;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::ADC(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.N = 0;
		uint8_t value{ 0 };
		uint8_t cycles{ 0 };
		uint8_t regA = cpu.REG.A;

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) { value = cpu.read(cpu.REG.HL); cycles = 2; } // ADC [HL]
			else { value = *cpu.m_TableREG8[code.getZ()]; cycles = 1; }     // ADC cpu.REG8[code.getZ()]
			break;
		}
		case 3: { // ADC d8
			value = cpu.fetch();
			cycles = 2;
			break; 
		} 
		}

		cpu.REG.A += value + cpu.REG.Flags.C;
		cpu.REG.Flags.Z = cpu.REG.A == 0;
		cpu.REG.Flags.H = ((regA & 0x0F) + (value &0x0F) + cpu.REG.Flags.C) > 0x0F;
		cpu.REG.Flags.C = static_cast<uint16_t>(regA) + value + cpu.REG.Flags.C > 0xFF;


		return cycles;
	}

	uint8_t SharpSM83::SUB(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.N = 1;
		uint8_t value{ 0 };
		uint8_t cycles{ 0 };

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) { value = cpu.read(cpu.REG.HL); cycles = 2; } // SUB [HL]
			else { value = *cpu.m_TableREG8[code.getZ()]; cycles = 1; }     // SUB cpu.REG8[code.getZ()]
			break;
		}
		case 3: { value = cpu.fetch(); cycles = 2; break; } // SUB d8
		}

		cpu.REG.Flags.H = cpu.halfCarryOccured8Sub(cpu.REG.A, value);
		cpu.REG.Flags.C = cpu.carryOccured8Sub(cpu.REG.A, value);

		cpu.REG.A -= value;

		cpu.REG.Flags.Z = cpu.REG.A == 0;

		return cycles;
	}

	uint8_t SharpSM83::SBC(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.N = 1;
		uint8_t value{ 0 };
		uint8_t cycles{ 0 };
		uint8_t regA = cpu.REG.A;

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) { value = cpu.read(cpu.REG.HL); cycles = 2; } // SBC [HL]
			else { value = *cpu.m_TableREG8[code.getZ()]; cycles = 1; }     // SBC cpu.REG8[code.getZ()]
			break;
		}
		case 3: { value = cpu.fetch(); cycles = 2; break; } // SBC d8
		}

		cpu.REG.A -= value + cpu.REG.Flags.C;
		cpu.REG.Flags.Z = cpu.REG.A == 0;
		cpu.REG.Flags.H = (regA & 0x0F) < ((value & 0x0F) + cpu.REG.Flags.C);
		cpu.REG.Flags.C = regA < (static_cast<uint16_t>(value) + cpu.REG.Flags.C);


		return cycles;
	}

	uint8_t SharpSM83::OR(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of OR
		uint8_t cycles{ 0 };
		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) //OR [HL]
			{
				cpu.REG.A |= cpu.read(cpu.REG.HL);
				cycles = 2;
			}
			else //OR cpu.REG8[code.getZ()]
			{
				cpu.REG.A |= *cpu.m_TableREG8[code.getZ()];
				cycles = 1;
			}
			break;
		}
		case 3: { cpu.REG.A |= cpu.fetch(); cycles = 2; break; } //OR d8
		}

		cpu.REG.Flags.Z = cpu.REG.A == 0;

		return cycles;
	}

	uint8_t SharpSM83::AND(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.Value = 0;
		cpu.REG.Flags.H = 1;
		uint8_t cycles{ 0 };

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) //AND [HL]
			{
				cpu.REG.A &= cpu.read(cpu.REG.HL);
				cycles = 2;
			}
			else //AND cpu.REG8[code.getZ()]
			{
				cpu.REG.A &= *cpu.m_TableREG8[code.getZ()];
				cycles = 1;
			}
			break;
		}
		case 3: { cpu.REG.A &= cpu.fetch(); cycles = 2; break; } //AND d8
		}

		cpu.REG.Flags.Z = cpu.REG.A == 0;

		return cycles;
	}

	uint8_t SharpSM83::XOR(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of XOR
		uint8_t cycles{ 0 };

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) //XOR [HL]
			{
				cpu.REG.A ^= cpu.read(cpu.REG.HL);
				cycles = 2;
			}
			else //XOR cpu.REG8[code.getZ()]
			{
				cpu.REG.A ^= *cpu.m_TableREG8[code.getZ()];
				cycles = 1;
			}
			break;
		}
		case 3: { cpu.REG.A ^= cpu.fetch(); cycles = 2; break; } //XOR d8
		}

		cpu.REG.Flags.Z = cpu.REG.A == 0;

		return cycles;
	}

	uint8_t SharpSM83::CP(SharpSM83& cpu, const opcode code)
	{
		uint8_t value{ 0 };
		uint8_t cycles{ 0 };

		switch (code.getX()) {
		case 2: {
			if (code.getZ() == 6) //CP [HL]
			{
				value = cpu.read(cpu.REG.HL);
				cycles = 2;
			}
			else //CP cpu.REG8[code.getZ()]
			{
				value = *cpu.m_TableREG8[code.getZ()];
				cycles = 1;
			}
			break;
		}
		case 3: { value = cpu.fetch(); cycles = 2; break; } //CP d8
		}

		cpu.REG.Flags.N = 1;
		cpu.REG.Flags.H = cpu.halfCarryOccured8Sub(cpu.REG.A, value);
		cpu.REG.Flags.C = cpu.carryOccured8Sub(cpu.REG.A, value);
		cpu.REG.Flags.Z = (cpu.REG.A - value) == 0;

		return cycles;
	}

	uint8_t SharpSM83::JP(SharpSM83& cpu, const opcode code)
	{
		switch (code.getZ()) {
		case 1: {cpu.REG.PC = cpu.REG.HL; return 1; } // JP HL
		case 2: { //JP cond[code.getY()], a16
			uint16_t address = cpu.fetchWord();

			if (cpu.m_TableConditions[code.getY()](cpu.REG.Flags.Value))
			{
				cpu.REG.PC = address;
				return 4;
			}

			return 3;
		}
		case 3: { // JP a16
			uint16_t address = cpu.fetchWord();

			cpu.REG.PC = address;
			return 4;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::JR(SharpSM83& cpu, const opcode code)
	{
		int8_t relAddress = cpu.fetchSigned();

		if (code.getY() == 3) // JR r8
		{
			cpu.REG.PC += relAddress;
			return 3;
		}
		else // JR cond[code.getY() - 4], r8
		{
			if (cpu.m_TableConditions[code.getY() - 4](cpu.REG.Flags.Value))
			{
				cpu.REG.PC += relAddress;
				return 3;
			}

			return 2;
		}
	}

	uint8_t SharpSM83::PUSH(SharpSM83& cpu, const opcode code)
	{
		if (code.getP() == 3) { // PUSH AF
			cpu.pushStack(cpu.REG.AF & 0xFFF0);
		}
		else {
			cpu.pushStack(*cpu.m_TableREGP_AF[code.getP()]);
		}

		return 4;
	}

	uint8_t SharpSM83::POP(SharpSM83& cpu, const opcode code)
	{
		if (code.getP() == 3) //POP AF
		{
			cpu.REG.AF = cpu.popStack() & 0xFFF0;
		}
		else { //POP REGAF[code.p]
			*cpu.m_TableREGP_AF[code.getP()] = cpu.popStack();
		}

		return 3;
	}

	uint8_t SharpSM83::RST(SharpSM83& cpu, const opcode code)
	{
		cpu.pushStack(cpu.REG.PC);
		cpu.REG.PC = static_cast<uint16_t>(code.getY() * 8);
		return 4;
	}

	uint8_t SharpSM83::CALL(SharpSM83& cpu, const opcode code)
	{
		uint16_t address = cpu.fetchWord();

		switch (code.getZ()) {
		case 4: { // CALL cond[code.getY()], a16
			if (cpu.m_TableConditions[code.getY()](cpu.REG.Flags.Value))
			{
				cpu.pushStack(cpu.REG.PC);
				cpu.REG.PC = address;
				return 6;
			}

			return 3;
		}
		case 5: { // CALL a16
			cpu.pushStack(cpu.REG.PC);
			cpu.REG.PC = address;
			return 6;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::RET(SharpSM83& cpu, const opcode code)
	{
		switch (code.getZ()) {
		case 0: { // RET cond[code.getY()]
			if (cpu.m_TableConditions[code.getY()](cpu.REG.Flags.Value))
			{
				cpu.REG.PC = cpu.popStack();
				return 5;
			}

			return 2;
		}
		case 1: { // RET
			cpu.REG.PC = cpu.popStack();
			return 4;
		}
		}
		return 0;
	}

	uint8_t SharpSM83::RETI(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.PC = cpu.popStack();
		cpu.IME = true;
		return 4;
	}

	uint8_t SharpSM83::DI(SharpSM83& cpu, const opcode code)
	{
		cpu.IME = false;
		cpu.m_EnableIME = false;
		return 1;
	}

	uint8_t SharpSM83::EI(SharpSM83& cpu, const opcode code)
	{
		cpu.m_EnableIME = true;
		return 1;
	}

	uint8_t SharpSM83::HALT(SharpSM83& cpu, const opcode code)
	{
		return 1;
	}

	uint8_t SharpSM83::STOP(SharpSM83& cpu, const opcode code)
	{
		return 1;
	}

	uint8_t SharpSM83::DAA(SharpSM83& cpu, const opcode code)
	{
		if (cpu.REG.Flags.N)
		{
			if (cpu.REG.Flags.C) cpu.REG.A -= 0x60;
			if (cpu.REG.Flags.H) cpu.REG.A -= 0x06;
		}
		else
		{
			if (cpu.REG.Flags.C || cpu.REG.A > 0x99)
			{
				cpu.REG.A += 0x60;
				cpu.REG.Flags.C = 1;
			}
			if (cpu.REG.Flags.H || (cpu.REG.A & 0x0F) > 0x09) cpu.REG.A += 0x06;
		}

		cpu.REG.Flags.H = 0;
		cpu.REG.Flags.Z = cpu.REG.A == 0;

		return 1;
	}

	uint8_t SharpSM83::CPL(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.N = 1;
		cpu.REG.Flags.H = 1;
		cpu.REG.A = ~cpu.REG.A;
		return 1;
	}

	uint8_t SharpSM83::CCF(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.N = 0;
		cpu.REG.Flags.H = 0;
		cpu.REG.Flags.C = !cpu.REG.Flags.C;
		return 1;
	}

	uint8_t SharpSM83::SCF(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.N = 0;
		cpu.REG.Flags.H = 0;
		cpu.REG.Flags.C = 1;
		return 1;
	}

	uint8_t SharpSM83::RLA(SharpSM83& cpu, const opcode code)
	{
		uint8_t firstBit = cpu.REG.Flags.C;
		cpu.REG.Flags.Value = 0;
		cpu.REG.Flags.C = (cpu.REG.A & 0x80) != 0;
		cpu.REG.A = (cpu.REG.A << 1) | firstBit;

		return 1;
	}

	uint8_t SharpSM83::RRA(SharpSM83& cpu, const opcode code)
	{
		uint8_t lastBit = static_cast<uint8_t>(cpu.REG.Flags.C) << 7;
		cpu.REG.Flags.Value = 0;
		cpu.REG.Flags.C = (cpu.REG.A & 0x01) != 0;
		cpu.REG.A = (cpu.REG.A >> 1) | lastBit;
		return 1;
	}

	uint8_t SharpSM83::RLCA(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.Value = 0;
		cpu.REG.Flags.C = (cpu.REG.A & 0b10000000) != 0;
		cpu.REG.A = (cpu.REG.A << 1) | (cpu.REG.A >> (sizeof(uint8_t) * CHAR_BIT - 1));

		return 1;
	}

	uint8_t SharpSM83::RRCA(SharpSM83& cpu, const opcode code)
	{
		cpu.REG.Flags.Value = 0;
		cpu.REG.Flags.C = (cpu.REG.A & 0b00000001) != 0;
		cpu.REG.A = (cpu.REG.A >> 1) | (cpu.REG.A << (sizeof(uint8_t) * CHAR_BIT - 1));

		return 1;
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