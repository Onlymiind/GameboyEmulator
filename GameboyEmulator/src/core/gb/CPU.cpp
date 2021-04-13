#include "core/gb/CPU.h"


namespace gbemu {


	SharpSM83::SharpSM83()
	{}


	void SharpSM83::tick()
	{
		//TODO: read next instruction;

		opcode code{};

		switch (code.x) {
		case 0: {
			if (code.z == 2 || code.z == 6 || (code.z == 1 && code.q == 0) || (code.z == 0 && code.y == 1)) LD(code);



		}
		case 1: {

		}
		case 2: {
			AL_LOOKUP[code.y](this, code);
			break;
		}
		case 3: {

		}
		}
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
	uint8_t SharpSM83::NOP(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::LD(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::INC(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RLA(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RLCA(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::ADD(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::JR(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::DEC(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RRA(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RRCA(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::SUB(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::OR(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::AND(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::XOR(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::PUSH(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::ADC(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::JP(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::POP(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RST(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::CALL(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::SBC(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::DI(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RET(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::CPL(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::RETI(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::CCF(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::EI(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::LDH(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::DAA(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::HALT(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::SCF(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::CP(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::STOP(opcode code)
	{
		return uint8_t();
	}
	uint8_t SharpSM83::NONE(opcode code)
	{
		return uint8_t();
	}
}