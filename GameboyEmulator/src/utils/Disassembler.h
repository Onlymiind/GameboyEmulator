#pragma once
#include "core/gb/AddressBus.h"
#include <sstream>
#include <string>
#include <array>

namespace gb{

	class Disassembler {
	public:
		Disassembler(const AddressBus& bus) :
			m_Bus(bus)
		{}
		~Disassembler() = default;

		std::string disassemble(uint16_t addressFrom, uint32_t commandCount = 10);

	private:

		const AddressBus& m_Bus;

		struct Instruction {
			std::string_view Mnemonic;
			uint8_t ByteCount;
		};

		const std::array<Instruction, 256> m_Lookup =
		{ {
			{"NOP",       1}, {"LD BC, d16", 3}, {"LD [BC], A",  1}, {"INC BC", 1}, {"INC B",    1}, {"DEC B",    1}, {"LD B, d8",    2}, {"RLCA", 1}, {"LD [a16], SP", 3}, {"ADD HL, BC", 1}, {"LD A, [BC]",  1}, {"DEC BC", 1}, {"INC C", 1}, {"DEC C", 1}, {"LD C, d8", 2}, {"RRCA", 1},
			{"STOP",      1}, {"LD DE, d16", 3}, {"LD [DE], A",  1}, {"INC DE", 1}, {"INC D",    1}, {"DEC D",    1}, {"LD D, d8",    2}, {"RLA",  1}, {"JR r8",        2}, {"ADD HL, DE", 1}, {"LD A, [DE]",  1}, {"DEC DE", 1}, {"INC E", 1}, {"DEC E", 1}, {"LD E, d8", 2}, {"RRA",  1},
			{"JR NZ, r8", 2}, {"LD HL, d16", 3}, {"LD [HL+], A", 1}, {"INC HL", 1}, {"INC H",    1}, {"DEC H",    1}, {"LD H, d8",    2}, {"DAA",  1}, {"JR Z, r8",     2}, {"ADD HL, HL", 1}, {"LD A, [HL+]", 1}, {"DEC HL", 1}, {"INC L", 1}, {"DEC L", 1}, {"LD L, d8", 2}, {"CPL",  1},
			{"JR NC, r8", 2}, {"LD SP, d16", 3}, {"LD [HL-], A", 1}, {"INC SP", 1}, {"INC [HL]", 1}, {"DEC [HL]", 1}, {"LD [HL], d8", 2}, {"SCF",  1}, {"JR C, r8",     2}, {"ADD HL, SP", 1}, {"LD A, [HL-]", 1}, {"DEC SP", 1}, {"INC A", 1}, {"DEC A", 1}, {"LD A, d8", 2}, {"CCF",  1},

			{"LD B, B",    1}, {"LD B, C",    1}, {"LD B, D",    1}, {"LD B, E",    1}, {"LD B, H",    1}, {"LD B, L",    1}, {"LD B, [HL]", 1}, {"LD B, A",    1}, {"LD C, B", 1}, {"LD C, C", 1}, {"LD C, D", 1}, {"LD C, E", 1}, {"LD C, H", 1}, {"LD C, L", 1}, {"LD C, [HL]", 1}, {"LD C, A", 1},
			{"LD D, B",    1}, {"LD D, C",    1}, {"LD D, D",    1}, {"LD D, E",    1}, {"LD D, H",    1}, {"LD D, L",    1}, {"LD D, [HL]", 1}, {"LD D, A",    1}, {"LD E, B", 1}, {"LD E, C", 1}, {"LD E, D", 1}, {"LD E, E", 1}, {"LD E, H", 1}, {"LD E, L", 1}, {"LD E, [HL]", 1}, {"LD E, A", 1},
			{"LD H, B",    1}, {"LD H, C",    1}, {"LD H, D",    1}, {"LD H, E",    1}, {"LD H, H",    1}, {"LD H, L",    1}, {"LD H, [HL]", 1}, {"LD H. A",    1}, {"LD L, B", 1}, {"LD L, C", 1}, {"LD L, D", 1}, {"LD L, E", 1}, {"LD L, H", 1}, {"LD L, L", 1}, {"LD L, [HL]", 1}, {"LD L, A", 1},
			{"LD [HL], B", 1}, {"LD [HL], C", 1}, {"LD [HL], D", 1}, {"LD [HL], E", 1}, {"LD [HL], H", 1}, {"LD [HL], L", 1}, {"HALT",       1}, {"LD [HL], A", 1}, {"LD A, B", 1}, {"LD A, C", 1}, {"LD A, D", 1}, {"LD A. E", 1}, {"LD A, H", 1}, {"LD A, L", 1}, {"LD A, [HL]", 1}, {"LD A, A", 1},

			{"ADD A, B", 1}, {"ADD A, C", 1}, {"ADD A, D", 1}, {"ADD A, E", 1}, {"ADD A, H", 1}, {"ADD A, L", 1}, {"ADD A, [HL]", 1}, {"ADD A, A", 1}, {"ADC A, B", 1}, {"ADC A, C", 1}, {"ADC A, D", 1}, {"ADC A, E", 1}, {"ADC A, H", 1}, {"ADC A, L", 1}, {"ADC A, [HL]", 1}, {"ADC A, A", 1},
			{"SUB B",    1}, {"SUB C",    1}, {"SUB D",    1}, {"SUB E",    1}, {"SUB H",    1}, {"SUB L",    1}, {"SUB [HL]",    1}, {"SUB A",    1}, {"SBC A, B", 1}, {"SBC A, C", 1}, {"SBC A, D", 1}, {"SBC A, E", 1}, {"SBC A, H", 1}, {"SBC A, L", 1}, {"SBC A, [HL]", 1}, {"SBC A, A", 1},
			{"AND B",    1}, {"AND C",    1}, {"AND D",    1}, {"AND E",    1}, {"AND H",    1}, {"AND L",    1}, {"AND [HL]",    1}, {"AND A",    1}, {"XOR B",    1}, {"XOR C",    1}, {"XOR D",    1}, {"XOR E",    1}, {"XOR H",    1}, {"XOR L",    1}, {"XOR [HL]",    1}, {"XOR A",    1},
			{"OR B",     1}, {"OR C",     1}, {"OR D",     1}, {"OR E",     1}, {"OR H",     1}, {"OR L",     1}, {"OR [HL]",     1}, {"OR A",     1}, {"CP B",     1}, {"CP C",     1}, {"CP D",     1}, {"CP E",     1}, {"CP H",     1}, {"CP L",     1}, {"CP [HL]",     1}, {"CP A",     1},

			{"RET NZ",             1}, {"POP BC", 1}, {"JP NZ, a16",        3}, {"JP a16",  3}, {"CALL NZ, a16", 3}, {"PUSH BC", 1}, {"ADD A, d8", 2}, {"RST 00H", 1}, {"RET Z",          1}, {"RET",       1}, {"JP Z, a16",   3}, {"CB",   1}, {"CALL Z, a16", 3}, {"CALL a16", 3}, {"ADC A, d8", 2}, {"RST 08H", 1},
			{"RET NC",             1}, {"POP DE", 1}, {"JP NC, a16",        3}, {"NONE",    0}, {"CALL NC, a16", 3}, {"PUSH DE", 1}, {"SUB d8",    2}, {"RST 10H", 1}, {"RET C",          1}, {"RETI",      1}, {"JP C, a16",   3}, {"NONE", 0}, {"CALL C, a16", 3}, {"NONE",     0}, {"SBC A, d8", 2}, {"RST 18H", 1},
			{"LD [$FF00 + a8], A", 2}, {"POP HL", 1}, {"LD [$FF00 + C], A", 1}, {"NONE",    0}, {"NONE",         0}, {"PUSH HL", 1}, {"AND d8",    2}, {"RST 20H", 1}, {"ADD SP, r8",     2}, {"JP HL",     1}, {"LD [a16], A", 3}, {"NONE", 0}, {"NONE",        0}, {"NONE",     0}, {"XOR d8",    2}, {"RST 28H", 1},
			{"LD A, [$FF00 + a8]", 2}, {"POP AF", 1}, {"LD A, [$FF00 + C]", 1}, {"DI",      1}, {"NONE",         0}, {"PUSH AF", 1}, {"OR d8",     2}, {"RST 30H", 1}, {"LD HL, SP + r8", 2}, {"LD SP, HL", 1}, {"LD A, [a16]", 3}, {"EI",   1}, {"NONE",        0}, {"NONE",     0}, {"CP d8",     2}, {"RST 38H", 1}
		} };

	};

}