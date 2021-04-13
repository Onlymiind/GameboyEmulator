#pragma once
#include "utils/Utils.h"

#include <iostream>
#include <cstdint>
#include <string_view>
#include <sstream>
#include <string>
#include <array>
#include <functional>

namespace gbemu {

	/// <summary>
	/// Struct to support easy opcode decomposition;
	/// </summary>
	struct opcode {
		union {
			uint8_t code;

			struct {

				uint8_t z : 3;

				union {

					uint8_t y : 3;

					struct {

						uint8_t q : 1;

						uint8_t p : 2;
					};
				};

				uint8_t x : 2;
			};
		};
	};

	class SharpSM83 {
		friend struct Instruction;
	public:
		SharpSM83();
		~SharpSM83() {}

		void tick();

		std::string registersOut();
	public:

		//Unprefixed instrictions
		uint8_t NOP(opcode code); uint8_t LD(opcode code); uint8_t INC(opcode code); uint8_t RLA(opcode code); uint8_t RLCA(opcode code);
		uint8_t ADD(opcode code); uint8_t JR(opcode code); uint8_t DEC(opcode code); uint8_t RRA(opcode code); uint8_t RRCA(opcode code);
		uint8_t SUB(opcode code); uint8_t OR(opcode code); uint8_t AND(opcode code); uint8_t XOR(opcode code); uint8_t PUSH(opcode code);
		uint8_t ADC(opcode code); uint8_t JP(opcode code); uint8_t POP(opcode code); uint8_t RST(opcode code); uint8_t CALL(opcode code);
		uint8_t SBC(opcode code); uint8_t DI(opcode code); uint8_t RET(opcode code); uint8_t CPL(opcode code); uint8_t RETI(opcode code);
		uint8_t CCF(opcode code); uint8_t EI(opcode code); uint8_t LDH(opcode code); uint8_t DAA(opcode code); uint8_t HALT(opcode code);
		uint8_t SCF(opcode code); uint8_t CP(opcode code); uint8_t STOP(opcode code); uint8_t LD_REG(opcode code);

		uint8_t NONE(opcode code);

	private:
		struct {
			union {
				uint16_t AF{};
			
				struct {
					union {
						uint8_t Value;

						struct {
							uint8_t Unused : 4;
							uint8_t C : 1;
							uint8_t H : 1;
							uint8_t N : 1;
							uint8_t Z : 1;
						};
					}Flags;

					uint8_t A;
				};
			};

			union {
				uint16_t BC{};

				struct {
					uint8_t C;
					uint8_t B;
				};
			};

			union {
				uint16_t DE{};

				struct {
					uint8_t E;
					uint8_t D;
				};
			};

			union {
				uint16_t HL{};

				struct {
					uint8_t L;
					uint8_t H;
				};
			};

			uint16_t SP{}, PC{};
		} REG;


		//8-bit registers lookup
		const std::array<uint8_t*, 8> m_TableREG8 = 
		{
			reinterpret_cast<uint8_t*>(&REG.BC) + 1, reinterpret_cast<uint8_t*>(&REG.BC) + 0, // &B, &C
			reinterpret_cast<uint8_t*>(&REG.DE) + 1, reinterpret_cast<uint8_t*>(&REG.DE) + 0, // &D, &E
			reinterpret_cast<uint8_t*>(&REG.HL) + 1, reinterpret_cast<uint8_t*>(&REG.HL) + 0, // &H, &L
			reinterpret_cast<uint8_t*>(&REG.HL) + 0, reinterpret_cast<uint8_t*>(&REG.AF) + 1  // &HL ([HL]), &A
		};

		//Register pair lookup with SP
		const std::array<uint16_t*, 4> m_TableREGP_SP = { &REG.BC, &REG.DE, &REG.HL, &REG.SP };

		//Register pair lookup with AF
		const std::array<uint16_t*, 4> m_TableREGP_AF = { &REG.BC, &REG.DE, &REG.HL, &REG.AF };

		//Conditions lookup
		const std::array<std::function<bool(uint8_t)>, 4> m_TableConditions = 
		{ 
			[](uint8_t flags) {return (flags & 0x80) == 0; }, //Not Z-flag
			[](uint8_t flags) {return (flags & 0x80) != 0; }, //Z-flag
			[](uint8_t flags) {return (flags & 0x10) == 0; }, //Not C-flag
			[](uint8_t flags) {return (flags & 0x10) != 0; }  //C-flag
		};

		struct Instruction {
			std::string_view Mnemonic;
			std::function<uint8_t(SharpSM83*, opcode)> Implementation;
			uint8_t ClockCycles;
		};

		const std::array<Instruction, 256> m_TableLookup =
		{ {
			{"NOP",       &SharpSM83::NOP,  4}, {"LD BC, d16", &SharpSM83::LD, 12}, {"LD [BC], A",  &SharpSM83::LD, 8}, {"INC BC", &SharpSM83::INC, 8}, {"INC B",    &SharpSM83::INC, 4},  {"DEC B",    &SharpSM83::DEC, 4},  {"LD B, d8",    &SharpSM83::LD, 8},  {"RLCA", &SharpSM83::RLCA, 4}, {"LD [a16], SP", &SharpSM83::LD, 20}, {"ADD HL, BC", &SharpSM83::ADD, 8}, {"LD A, [BC]",  &SharpSM83::LD, 8}, {"DEC BC", &SharpSM83::DEC, 8}, {"INC C", &SharpSM83::INC, 4}, {"DEC C", &SharpSM83::DEC, 4}, {"LD C, d8", &SharpSM83::LD, 8}, {"RRCA", &SharpSM83::RRCA, 4},
			{"STOP",      &SharpSM83::STOP, 4}, {"LD DE, d16", &SharpSM83::LD, 12}, {"LD [DE], A",  &SharpSM83::LD, 8}, {"INC DE", &SharpSM83::INC, 8}, {"INC D",    &SharpSM83::INC, 4},  {"DEC D",    &SharpSM83::DEC, 4},  {"LD D, d8",    &SharpSM83::LD, 8},  {"RLA",  &SharpSM83::RLA,  4}, {"JR r8",        &SharpSM83::JR, 12}, {"ADD HL, DE", &SharpSM83::ADD, 8}, {"LD A, [DE]",  &SharpSM83::LD, 8}, {"DEC DE", &SharpSM83::DEC, 8}, {"INC E", &SharpSM83::INC, 4}, {"DEC E", &SharpSM83::DEC, 4}, {"LD E, d8", &SharpSM83::LD, 8}, {"RRA",  &SharpSM83::RRA, 4},
			{"JR NZ, r8", &SharpSM83::JR,   8}, {"LD HL, d16", &SharpSM83::LD, 12}, {"LD [HL+], A", &SharpSM83::LD, 8}, {"INC HL", &SharpSM83::INC, 8}, {"INC H",    &SharpSM83::INC, 4},  {"DEC H",    &SharpSM83::DEC, 4},  {"LD H, d8",    &SharpSM83::LD, 8},  {"DAA",  &SharpSM83::DAA,  4}, {"JR Z, r8",     &SharpSM83::JR, 8},  {"ADD HL, HL", &SharpSM83::ADD, 8}, {"LD A, [HL+]", &SharpSM83::LD, 8}, {"DEC HL", &SharpSM83::DEC, 8}, {"INC L", &SharpSM83::INC, 4}, {"DEC L", &SharpSM83::DEC, 4}, {"LD L, d8", &SharpSM83::LD, 8}, {"CPL",  &SharpSM83::CPL, 4},
			{"JR NC, r8", &SharpSM83::JR,   8}, {"LD SP, d16", &SharpSM83::LD, 12}, {"LD [HL-], A", &SharpSM83::LD, 8}, {"INC SP", &SharpSM83::INC, 8}, {"INC [HL]", &SharpSM83::INC, 12}, {"DEC [HL]", &SharpSM83::DEC, 12}, {"LD [HL], d8", &SharpSM83::LD, 12}, {"SCF",  &SharpSM83::SCF,  4}, {"JR C, r8",     &SharpSM83::JR, 8},  {"ADD HL, SP", &SharpSM83::ADD, 8}, {"LD A, [HL-]", &SharpSM83::LD, 8}, {"DEC SP", &SharpSM83::DEC, 8}, {"INC A", &SharpSM83::INC, 4}, {"DEC A", &SharpSM83::DEC, 4}, {"LD A, d8", &SharpSM83::LD, 8}, {"CCF",  &SharpSM83::CCF, 4},
			
			{"LD B, B",    &SharpSM83::LD_REG, 4}, {"LD B, C",    &SharpSM83::LD_REG, 4}, {"LD B, D",    &SharpSM83::LD_REG, 4}, {"LD B, E",    &SharpSM83::LD_REG, 4}, {"LD B, H",    &SharpSM83::LD_REG, 4}, {"LD B, L",    &SharpSM83::LD_REG, 4}, {"LD B, [HL]", &SharpSM83::LD_REG,   8}, {"LD B, A",    &SharpSM83::LD_REG, 4}, {"LD C, B", &SharpSM83::LD_REG, 4}, {"LD C, C", &SharpSM83::LD_REG, 4}, {"LD C, D", &SharpSM83::LD_REG, 4}, {"LD C, E", &SharpSM83::LD_REG, 4}, {"LD C, H", &SharpSM83::LD_REG, 4}, {"LD C, L", &SharpSM83::LD_REG, 4}, {"LD C, [HL]", &SharpSM83::LD_REG, 8}, {"LD C, A", &SharpSM83::LD_REG, 4},
			{"LD D, B",    &SharpSM83::LD_REG, 4}, {"LD D, C",    &SharpSM83::LD_REG, 4}, {"LD D, D",    &SharpSM83::LD_REG, 4}, {"LD D, E",    &SharpSM83::LD_REG, 4}, {"LD D, H",    &SharpSM83::LD_REG, 4}, {"LD D, L",    &SharpSM83::LD_REG, 4}, {"LD D, [HL]", &SharpSM83::LD_REG,   8}, {"LD D, A",    &SharpSM83::LD_REG, 4}, {"LD E, B", &SharpSM83::LD_REG, 4}, {"LD E, C", &SharpSM83::LD_REG, 4}, {"LD E, D", &SharpSM83::LD_REG, 4}, {"LD E, E", &SharpSM83::LD_REG, 4}, {"LD E, H", &SharpSM83::LD_REG, 4}, {"LD E, L", &SharpSM83::LD_REG, 4}, {"LD E, [HL]", &SharpSM83::LD_REG, 8}, {"LD E, A", &SharpSM83::LD_REG, 4},
			{"LD H, B",    &SharpSM83::LD_REG, 4}, {"LD H, C",    &SharpSM83::LD_REG, 4}, {"LD H, D",    &SharpSM83::LD_REG, 4}, {"LD H, E",    &SharpSM83::LD_REG, 4}, {"LD H, H",    &SharpSM83::LD_REG, 4}, {"LD H, L",    &SharpSM83::LD_REG, 4}, {"LD H, [HL]", &SharpSM83::LD_REG,   8}, {"LD H. A",    &SharpSM83::LD_REG, 4}, {"LD L, B", &SharpSM83::LD_REG, 4}, {"LD L, C", &SharpSM83::LD_REG, 4}, {"LD L, D", &SharpSM83::LD_REG, 4}, {"LD L, E", &SharpSM83::LD_REG, 4}, {"LD L, H", &SharpSM83::LD_REG, 4}, {"LD L, L", &SharpSM83::LD_REG, 4}, {"LD L, [HL]", &SharpSM83::LD_REG, 8}, {"LD L, A", &SharpSM83::LD_REG, 4},
			{"LD [HL], B", &SharpSM83::LD_REG, 8}, {"LD [HL], C", &SharpSM83::LD_REG, 8}, {"LD [HL], D", &SharpSM83::LD_REG, 8}, {"LD [HL], E", &SharpSM83::LD_REG, 8}, {"LD [HL], H", &SharpSM83::LD_REG, 8}, {"LD [HL], L", &SharpSM83::LD_REG, 8}, {"HALT",       &SharpSM83::HALT, 4}, {"LD [HL], A", &SharpSM83::LD_REG, 8}, {"LD A, B", &SharpSM83::LD_REG, 4}, {"LD A, C", &SharpSM83::LD_REG, 4}, {"LD A, D", &SharpSM83::LD_REG, 4}, {"LD A. E", &SharpSM83::LD_REG, 4}, {"LD A, H", &SharpSM83::LD_REG, 4}, {"LD A, L", &SharpSM83::LD_REG, 4}, {"LD A, [HL]", &SharpSM83::LD_REG, 8}, {"LD A, A", &SharpSM83::LD_REG, 4},
			
			{"ADD A, B", &SharpSM83::ADD, 4}, {"ADD A, C", &SharpSM83::ADD, 4}, {"ADD A, D", &SharpSM83::ADD, 4}, {"ADD A, E", &SharpSM83::ADD, 4}, {"ADD A, H", &SharpSM83::ADD, 4}, {"ADD A, L", &SharpSM83::ADD, 4}, {"ADD A, [HL]", &SharpSM83::ADD, 8}, {"ADD A, A", &SharpSM83::ADD, 4}, {"ADC A, B", &SharpSM83::ADC, 4}, {"ADC A, C", &SharpSM83::ADC, 4}, {"ADC A, D", &SharpSM83::ADC, 4}, {"ADC A, E", &SharpSM83::ADC, 4}, {"ADC A, H", &SharpSM83::ADC, 4}, {"ADC A, L", &SharpSM83::ADC, 4}, {"ADC A, [HL]", &SharpSM83::ADC, 8}, {"ADC A, A", &SharpSM83::ADC, 4},
			{"SUB B",    &SharpSM83::SUB, 4}, {"SUB C",    &SharpSM83::SUB, 4}, {"SUB D",    &SharpSM83::SUB, 4}, {"SUB E",    &SharpSM83::SUB, 4}, {"SUB H",    &SharpSM83::SUB, 4}, {"SUB L",    &SharpSM83::SUB, 4}, {"SUB [HL]",    &SharpSM83::SUB, 8}, {"SUB A",    &SharpSM83::SUB, 4}, {"SBC A, B", &SharpSM83::SBC, 4}, {"SBC A, C", &SharpSM83::SBC, 4}, {"SBC A, D", &SharpSM83::SBC, 4}, {"SBC A, E", &SharpSM83::SBC, 4}, {"SBC A, H", &SharpSM83::SBC, 4}, {"SBC A, L", &SharpSM83::SBC, 4}, {"SBC A, [HL]", &SharpSM83::SBC, 8}, {"SBC A, A", &SharpSM83::SBC, 4},
			
			{"AND B", &SharpSM83::AND, 4}, {"AND C", &SharpSM83::AND, 4}, {"AND D", &SharpSM83::AND, 4}, {"AND E", &SharpSM83::AND, 4}, {"AND H", &SharpSM83::AND, 4}, {"AND L", &SharpSM83::AND, 4}, {"AND [HL]", &SharpSM83::AND, 8}, {"AND A", &SharpSM83::AND, 4}, {"XOR B", &SharpSM83::XOR, 4}, {"XOR C", &SharpSM83::XOR, 4}, {"XOR D", &SharpSM83::XOR, 4}, {"XOR E", &SharpSM83::XOR, 4}, {"XOR H", &SharpSM83::XOR, 4}, {"XOR L", &SharpSM83::XOR, 4}, {"XOR [HL]", &SharpSM83::XOR, 8}, {"XOR A", &SharpSM83::XOR, 4},
			{"OR B",  &SharpSM83::OR,  4}, {"OR C",  &SharpSM83::OR,  4}, {"OR D",  &SharpSM83::OR,  4}, {"OR E",  &SharpSM83::OR,  4}, {"OR H",  &SharpSM83::OR,  4}, {"OR L",  &SharpSM83::OR,  4}, {"OR [HL]",  &SharpSM83::OR,  8}, {"OR A",  &SharpSM83::OR,  4}, {"CP B",  &SharpSM83::CP,  4}, {"CP C",  &SharpSM83::CP,  4}, {"CP D",  &SharpSM83::CP,  4}, {"CP E",  &SharpSM83::CP,  4}, {"CP H",  &SharpSM83::CP,  4}, {"CP L",  &SharpSM83::CP,  4}, {"CP [HL]",  &SharpSM83::CP,  8}, {"CP A",  &SharpSM83::CP,  4},
			
			{"RET NZ", &SharpSM83::RET, 8}, {"POP BC", &SharpSM83::POP, 12}, {"JP NZ, a16", &SharpSM83::JP, 12}, {"JP a16", &SharpSM83::JP, 16}, {"CALL NZ, a16", &SharpSM83::CALL, 12}, {"PUSH BC", &SharpSM83::PUSH, 16}, {"ADD A, d8", &SharpSM83::ADD, 8}, {"RST 00H", &SharpSM83::RST, 16}, {"RET Z", &SharpSM83::RET, 8}, {"RET",  &SharpSM83::RET,  16}, {"JP Z, a16", &SharpSM83::JP, 12}, {"CB",   nullptr, 4}, {"CALL Z, a16", &SharpSM83::CALL, 12}, {"CALL a16", &SharpSM83::CALL, 24}, {"ADC A, d8", &SharpSM83::ADC, 8}, {"RST 08H", &SharpSM83::RST, 16},
			{"RET NC", &SharpSM83::RET, 8}, {"POP DE", &SharpSM83::POP, 12}, {"JP NC, a16", &SharpSM83::JP, 12}, {"NONE", nullptr,  0},          {"CALL NC, a16", &SharpSM83::CALL, 12}, {"PUSH DE", &SharpSM83::PUSH, 16}, {"SUB d8",    &SharpSM83::SUB, 8}, {"RST 10H", &SharpSM83::RST, 16}, {"RET C", &SharpSM83::RET, 8}, {"RETI", &SharpSM83::RETI, 16}, {"JP C, a16", &SharpSM83::JP, 12}, {"NONE", nullptr, 0}, {"CALL C, a16", &SharpSM83::CALL, 12}, {"NONE",     nullptr,  0},          {"SBC A, d8", &SharpSM83::ADC, 8}, {"RST 18H", &SharpSM83::RST, 16},
			
			{"LDH [a8], A", &SharpSM83::LDH, 12}, {"POP HL", &SharpSM83::POP, 12}, {"LD [C], A", &SharpSM83::LD, 8}, {"NONE", nullptr, 0},      {"NONE", nullptr, 0}, {"PUSH HL", &SharpSM83::PUSH, 16}, {"AND d8", &SharpSM83::AND, 8}, {"RST 20H", &SharpSM83::RST, 16}, {"ADD SP, r8",     &SharpSM83::ADD, 16}, {"JP HL",     &SharpSM83::JP, 4}, {"LD [a16], A", &SharpSM83::LD, 16}, {"NONE", nullptr, 0},      {"NONE", nullptr, 0}, {"NONE", nullptr, 0}, {"XOR d8", &SharpSM83::XOR, 8}, {"RST 28H", &SharpSM83::RST, 16},
			{"LDH A, [a8]", &SharpSM83::LDH, 12}, {"POP AF", &SharpSM83::POP, 12}, {"LD A, [C]", &SharpSM83::LD, 8}, {"DI", &SharpSM83::DI, 4}, {"NONE", nullptr, 0}, {"PUSH AF", &SharpSM83::PUSH, 16}, {"OR d8",  &SharpSM83::OR,  8}, {"RST 30H", &SharpSM83::RST, 16}, {"LD HL, SP + r8", &SharpSM83::LD,  12}, {"LD SP, HL", &SharpSM83::LD, 8}, {"LD A, [a16]", &SharpSM83::LD, 16}, {"EI", &SharpSM83::EI, 4}, {"NONE", nullptr, 0}, {"NONE", nullptr, 0}, {"CP d8",  &SharpSM83::CP,  8}, {"RST 38H", &SharpSM83::RST, 16}
		} };
	};
}