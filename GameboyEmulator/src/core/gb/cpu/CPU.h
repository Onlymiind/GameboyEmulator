#pragma once
#include "utils/Utils.h"
#include "core/gb/AddressBus.h"

#include <cstdint>
#include <string_view>
#include <string>
#include <array>
#include <functional>

namespace gbemu {

	/// <summary>
	/// Struct to support easy opcode decomposition;
	/// </summary>
	struct opcode {
		opcode() :
			code(0) {}
		opcode(uint8_t val) :
			code(val) {}

		inline uint8_t getX() const { return (code & 0b11000000) >> 6; }
		inline uint8_t getY() const { return (code & 0b00111000) >> 3; }
		inline uint8_t getZ() const { return (code & 0b00000111) >> 0; }
		inline uint8_t getP() const { return (code & 0b00110000) >> 4; }
		inline uint8_t getQ() const { return (code & 0b00001000) >> 3; }

		uint8_t code;
	};

	class SharpSM83 {
	public:
		SharpSM83(AddressBus& bus);
		~SharpSM83() {}

		void tick();

		std::string registersOut();

		inline uint16_t getProgramCounter() const { return REG.PC; }

		inline bool isFinished() const { return m_CyclesToFinish == 0; }

	private:

		uint8_t read(uint16_t address);

		void write(uint16_t address, uint8_t data);

		uint8_t fetch();

		uint16_t fetchWord();

		void pushStack(uint16_t value);

		uint16_t popStack();

		bool halfCarryOccured8Add(uint8_t lhs, uint8_t rhs);

		bool halfCarryOccured8Sub(uint8_t lhs, uint8_t rhs);

		bool carryOccured8Add(uint8_t lhs, uint8_t rhs);

		bool carryOccured8Sub(uint8_t lhs, uint8_t rhs);
		
		bool halfCarryOccured16Add(uint16_t lhs, uint16_t rhs);

		bool carryOccured16Add(uint16_t lhs, uint16_t rhs);

		int8_t fetchSigned();


		//Unprefixed instrictions. Can return additional amount of machine cycles needed for the instruction
		uint8_t NOP(const opcode code); uint8_t LD(const opcode code); uint8_t INC(const opcode code); uint8_t RLA(const opcode code); uint8_t RLCA(const opcode code);
		uint8_t ADD(const opcode code); uint8_t JR(const opcode code); uint8_t DEC(const opcode code); uint8_t RRA(const opcode code); uint8_t RRCA(const opcode code);
		uint8_t SUB(const opcode code); uint8_t OR(const opcode code); uint8_t AND(const opcode code); uint8_t XOR(const opcode code); uint8_t PUSH(const opcode code);
		uint8_t ADC(const opcode code); uint8_t JP(const opcode code); uint8_t POP(const opcode code); uint8_t RST(const opcode code); uint8_t CALL(const opcode code);
		uint8_t SBC(const opcode code); uint8_t DI(const opcode code); uint8_t RET(const opcode code); uint8_t CPL(const opcode code); uint8_t RETI(const opcode code);
		uint8_t CCF(const opcode code); uint8_t EI(const opcode code); uint8_t DAA(const opcode code); uint8_t HALT(const opcode code); uint8_t LD_IO(const opcode code);
		uint8_t SCF(const opcode code); uint8_t CP(const opcode code); uint8_t STOP(const opcode code); uint8_t LD_REG8(const opcode code); uint8_t LD_IMM(const opcode code);

		uint8_t NONE(const opcode code);

		//Prefixed instructions. Always return the amount of machine cycles needed for the instruction
		uint8_t RLC (const opcode code); uint8_t RRC(const opcode code); 
		uint8_t RL  (const opcode code); uint8_t RR (const opcode code);
		uint8_t SLA (const opcode code); uint8_t SRA(const opcode code);
		uint8_t SWAP(const opcode code); uint8_t SRL(const opcode code);
		uint8_t BIT (const opcode code); uint8_t RES(const opcode code);  
		uint8_t SET (const opcode code);


	private: //REGISTERS
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

			uint16_t SP{0xFFFE}, PC{0x0100}; // Stack pointer, program counter
		} REG;

		bool IME{ false }; // Interrupt master enable
		bool m_EnableIME{ false };


	private: //LOOKUP

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
			[](uint8_t flags) {return (flags & 0b10000000) == 0; }, //Not Z-flag
			[](uint8_t flags) {return (flags & 0b10000000) != 0; }, //Z-flag
			[](uint8_t flags) {return (flags & 0b00010000) == 0; }, //Not C-flag
			[](uint8_t flags) {return (flags & 0b00010000) != 0; }  //C-flag
		};

		struct Instruction {
			std::string_view Mnemonic;
			std::function<uint8_t(SharpSM83*, const opcode)> Implementation;
			uint8_t MachineCycles;
		};

		const std::array<Instruction, 256> m_TableLookup =
		{ {
			{"NOP",       &SharpSM83::NOP,  1}, {"LD BC, d16", &SharpSM83::LD_IMM, 3}, {"LD [BC], A",  &SharpSM83::LD, 2}, {"INC BC", &SharpSM83::INC, 2}, {"INC B",    &SharpSM83::INC, 1}, {"DEC B",    &SharpSM83::DEC, 1}, {"LD B, d8",    &SharpSM83::LD_IMM, 2}, {"RLCA", &SharpSM83::RLCA, 1}, {"LD [a16], SP", &SharpSM83::LD_IMM, 5}, {"ADD HL, BC", &SharpSM83::ADD, 2}, {"LD A, [BC]",  &SharpSM83::LD, 2}, {"DEC BC", &SharpSM83::DEC, 2}, {"INC C", &SharpSM83::INC, 1}, {"DEC C", &SharpSM83::DEC, 1}, {"LD C, d8", &SharpSM83::LD_IMM, 2}, {"RRCA", &SharpSM83::RRCA, 1},
			{"STOP",      &SharpSM83::STOP, 1}, {"LD DE, d16", &SharpSM83::LD_IMM, 3}, {"LD [DE], A",  &SharpSM83::LD, 2}, {"INC DE", &SharpSM83::INC, 2}, {"INC D",    &SharpSM83::INC, 1}, {"DEC D",    &SharpSM83::DEC, 1}, {"LD D, d8",    &SharpSM83::LD_IMM, 2}, {"RLA",  &SharpSM83::RLA,  1}, {"JR r8",        &SharpSM83::JR,     3}, {"ADD HL, DE", &SharpSM83::ADD, 2}, {"LD A, [DE]",  &SharpSM83::LD, 2}, {"DEC DE", &SharpSM83::DEC, 2}, {"INC E", &SharpSM83::INC, 1}, {"DEC E", &SharpSM83::DEC, 1}, {"LD E, d8", &SharpSM83::LD_IMM, 2}, {"RRA",  &SharpSM83::RRA,  1},
			{"JR NZ, r8", &SharpSM83::JR,   2}, {"LD HL, d16", &SharpSM83::LD_IMM, 3}, {"LD [HL+], A", &SharpSM83::LD, 2}, {"INC HL", &SharpSM83::INC, 2}, {"INC H",    &SharpSM83::INC, 1}, {"DEC H",    &SharpSM83::DEC, 1}, {"LD H, d8",    &SharpSM83::LD_IMM, 2}, {"DAA",  &SharpSM83::DAA,  1}, {"JR Z, r8",     &SharpSM83::JR,     2}, {"ADD HL, HL", &SharpSM83::ADD, 2}, {"LD A, [HL+]", &SharpSM83::LD, 2}, {"DEC HL", &SharpSM83::DEC, 2}, {"INC L", &SharpSM83::INC, 1}, {"DEC L", &SharpSM83::DEC, 1}, {"LD L, d8", &SharpSM83::LD_IMM, 2}, {"CPL",  &SharpSM83::CPL,  1},
			{"JR NC, r8", &SharpSM83::JR,   2}, {"LD SP, d16", &SharpSM83::LD_IMM, 3}, {"LD [HL-], A", &SharpSM83::LD, 2}, {"INC SP", &SharpSM83::INC, 2}, {"INC [HL]", &SharpSM83::INC, 3}, {"DEC [HL]", &SharpSM83::DEC, 3}, {"LD [HL], d8", &SharpSM83::LD_IMM, 3}, {"SCF",  &SharpSM83::SCF,  1}, {"JR C, r8",     &SharpSM83::JR,     2}, {"ADD HL, SP", &SharpSM83::ADD, 2}, {"LD A, [HL-]", &SharpSM83::LD, 2}, {"DEC SP", &SharpSM83::DEC, 2}, {"INC A", &SharpSM83::INC, 1}, {"DEC A", &SharpSM83::DEC, 1}, {"LD A, d8", &SharpSM83::LD_IMM, 2}, {"CCF",  &SharpSM83::CCF,  1},
			
			{"LD B, B",    &SharpSM83::LD_REG8, 1}, {"LD B, C",    &SharpSM83::LD_REG8, 1}, {"LD B, D",    &SharpSM83::LD_REG8, 1}, {"LD B, E",    &SharpSM83::LD_REG8, 1}, {"LD B, H",    &SharpSM83::LD_REG8, 1}, {"LD B, L",    &SharpSM83::LD_REG8, 1}, {"LD B, [HL]", &SharpSM83::LD_REG8, 2}, {"LD B, A",    &SharpSM83::LD_REG8, 1}, {"LD C, B", &SharpSM83::LD_REG8, 1}, {"LD C, C", &SharpSM83::LD_REG8, 1}, {"LD C, D", &SharpSM83::LD_REG8, 1}, {"LD C, E", &SharpSM83::LD_REG8, 1}, {"LD C, H", &SharpSM83::LD_REG8, 1}, {"LD C, L", &SharpSM83::LD_REG8, 1}, {"LD C, [HL]", &SharpSM83::LD_REG8, 2}, {"LD C, A", &SharpSM83::LD_REG8, 1},
			{"LD D, B",    &SharpSM83::LD_REG8, 1}, {"LD D, C",    &SharpSM83::LD_REG8, 1}, {"LD D, D",    &SharpSM83::LD_REG8, 1}, {"LD D, E",    &SharpSM83::LD_REG8, 1}, {"LD D, H",    &SharpSM83::LD_REG8, 1}, {"LD D, L",    &SharpSM83::LD_REG8, 1}, {"LD D, [HL]", &SharpSM83::LD_REG8, 2}, {"LD D, A",    &SharpSM83::LD_REG8, 1}, {"LD E, B", &SharpSM83::LD_REG8, 1}, {"LD E, C", &SharpSM83::LD_REG8, 1}, {"LD E, D", &SharpSM83::LD_REG8, 1}, {"LD E, E", &SharpSM83::LD_REG8, 1}, {"LD E, H", &SharpSM83::LD_REG8, 1}, {"LD E, L", &SharpSM83::LD_REG8, 1}, {"LD E, [HL]", &SharpSM83::LD_REG8, 2}, {"LD E, A", &SharpSM83::LD_REG8, 1},
			{"LD H, B",    &SharpSM83::LD_REG8, 1}, {"LD H, C",    &SharpSM83::LD_REG8, 1}, {"LD H, D",    &SharpSM83::LD_REG8, 1}, {"LD H, E",    &SharpSM83::LD_REG8, 1}, {"LD H, H",    &SharpSM83::LD_REG8, 1}, {"LD H, L",    &SharpSM83::LD_REG8, 1}, {"LD H, [HL]", &SharpSM83::LD_REG8, 2}, {"LD H. A",    &SharpSM83::LD_REG8, 1}, {"LD L, B", &SharpSM83::LD_REG8, 1}, {"LD L, C", &SharpSM83::LD_REG8, 1}, {"LD L, D", &SharpSM83::LD_REG8, 1}, {"LD L, E", &SharpSM83::LD_REG8, 1}, {"LD L, H", &SharpSM83::LD_REG8, 1}, {"LD L, L", &SharpSM83::LD_REG8, 1}, {"LD L, [HL]", &SharpSM83::LD_REG8, 2}, {"LD L, A", &SharpSM83::LD_REG8, 1},
			{"LD [HL], B", &SharpSM83::LD_REG8, 2}, {"LD [HL], C", &SharpSM83::LD_REG8, 2}, {"LD [HL], D", &SharpSM83::LD_REG8, 2}, {"LD [HL], E", &SharpSM83::LD_REG8, 2}, {"LD [HL], H", &SharpSM83::LD_REG8, 2}, {"LD [HL], L", &SharpSM83::LD_REG8, 2}, {"HALT",       &SharpSM83::HALT,    1}, {"LD [HL], A", &SharpSM83::LD_REG8, 2}, {"LD A, B", &SharpSM83::LD_REG8, 1}, {"LD A, C", &SharpSM83::LD_REG8, 1}, {"LD A, D", &SharpSM83::LD_REG8, 1}, {"LD A. E", &SharpSM83::LD_REG8, 1}, {"LD A, H", &SharpSM83::LD_REG8, 1}, {"LD A, L", &SharpSM83::LD_REG8, 1}, {"LD A, [HL]", &SharpSM83::LD_REG8, 2}, {"LD A, A", &SharpSM83::LD_REG8, 1},
			
			{"ADD A, B", &SharpSM83::ADD, 1}, {"ADD A, C", &SharpSM83::ADD, 1}, {"ADD A, D", &SharpSM83::ADD, 1}, {"ADD A, E", &SharpSM83::ADD, 1}, {"ADD A, H", &SharpSM83::ADD, 1}, {"ADD A, L", &SharpSM83::ADD, 1}, {"ADD A, [HL]", &SharpSM83::ADD, 2}, {"ADD A, A", &SharpSM83::ADD, 1}, {"ADC A, B", &SharpSM83::ADC, 1}, {"ADC A, C", &SharpSM83::ADC, 1}, {"ADC A, D", &SharpSM83::ADC, 1}, {"ADC A, E", &SharpSM83::ADC, 1}, {"ADC A, H", &SharpSM83::ADC, 1}, {"ADC A, L", &SharpSM83::ADC, 1}, {"ADC A, [HL]", &SharpSM83::ADC, 2}, {"ADC A, A", &SharpSM83::ADC, 1},
			{"SUB B",    &SharpSM83::SUB, 1}, {"SUB C",    &SharpSM83::SUB, 1}, {"SUB D",    &SharpSM83::SUB, 1}, {"SUB E",    &SharpSM83::SUB, 1}, {"SUB H",    &SharpSM83::SUB, 1}, {"SUB L",    &SharpSM83::SUB, 1}, {"SUB [HL]",    &SharpSM83::SUB, 2}, {"SUB A",    &SharpSM83::SUB, 1}, {"SBC A, B", &SharpSM83::SBC, 1}, {"SBC A, C", &SharpSM83::SBC, 1}, {"SBC A, D", &SharpSM83::SBC, 1}, {"SBC A, E", &SharpSM83::SBC, 1}, {"SBC A, H", &SharpSM83::SBC, 1}, {"SBC A, L", &SharpSM83::SBC, 1}, {"SBC A, [HL]", &SharpSM83::SBC, 2}, {"SBC A, A", &SharpSM83::SBC, 1},
			{"AND B",    &SharpSM83::AND, 1}, {"AND C",    &SharpSM83::AND, 1}, {"AND D",    &SharpSM83::AND, 1}, {"AND E",    &SharpSM83::AND, 1}, {"AND H",    &SharpSM83::AND, 1}, {"AND L",    &SharpSM83::AND, 1}, {"AND [HL]",    &SharpSM83::AND, 2}, {"AND A",    &SharpSM83::AND, 1}, {"XOR B",    &SharpSM83::XOR, 1}, {"XOR C",    &SharpSM83::XOR, 1}, {"XOR D",    &SharpSM83::XOR, 1}, {"XOR E",    &SharpSM83::XOR, 1}, {"XOR H",    &SharpSM83::XOR, 1}, {"XOR L",    &SharpSM83::XOR, 1}, {"XOR [HL]",    &SharpSM83::XOR, 2}, {"XOR A",    &SharpSM83::XOR, 1},
			{"OR B",     &SharpSM83::OR,  1}, {"OR C",     &SharpSM83::OR,  1}, {"OR D",     &SharpSM83::OR,  1}, {"OR E",     &SharpSM83::OR,  1}, {"OR H",     &SharpSM83::OR,  1}, {"OR L",     &SharpSM83::OR,  1}, {"OR [HL]",     &SharpSM83::OR,  2}, {"OR A",     &SharpSM83::OR,  1}, {"CP B",     &SharpSM83::CP,  1}, {"CP C",     &SharpSM83::CP,  1}, {"CP D",     &SharpSM83::CP,  1}, {"CP E",     &SharpSM83::CP,  1}, {"CP H",     &SharpSM83::CP,  1}, {"CP L",     &SharpSM83::CP,  1}, {"CP [HL]",     &SharpSM83::CP,  2}, {"CP A",     &SharpSM83::CP,  1},
			
			{"RET NZ",             &SharpSM83::RET,    2}, {"POP BC", &SharpSM83::POP, 3}, {"JP NZ, a16",         &SharpSM83::JP,    3}, {"JP a16", &SharpSM83::JP, 4}, {"CALL NZ, a16", &SharpSM83::CALL, 3}, {"PUSH BC", &SharpSM83::PUSH, 4}, {"ADD A, d8", &SharpSM83::ADD, 2}, {"RST 00H", &SharpSM83::RST, 4}, {"RET Z",          &SharpSM83::RET,     2}, {"RET",       &SharpSM83::RET,  4}, {"JP Z, a16",   &SharpSM83::JP, 3}, {"CB",   nullptr,        1}, {"CALL Z, a16", &SharpSM83::CALL, 3}, {"CALL a16", &SharpSM83::CALL, 6}, {"ADC A, d8", &SharpSM83::ADC, 2}, {"RST 08H", &SharpSM83::RST, 4},
			{"RET NC",             &SharpSM83::RET,    2}, {"POP DE", &SharpSM83::POP, 3}, {"JP NC, a16",         &SharpSM83::JP,    3}, {"NONE",   nullptr,        0}, {"CALL NC, a16", &SharpSM83::CALL, 3}, {"PUSH DE", &SharpSM83::PUSH, 4}, {"SUB d8",    &SharpSM83::SUB, 2}, {"RST 10H", &SharpSM83::RST, 4}, {"RET C",          &SharpSM83::RET,     2}, {"RETI",      &SharpSM83::RETI, 4}, {"JP C, a16",   &SharpSM83::JP, 3}, {"NONE", nullptr,        0}, {"CALL C, a16", &SharpSM83::CALL, 3}, {"NONE",     nullptr,          0}, {"SBC A, d8", &SharpSM83::SBC, 2}, {"RST 18H", &SharpSM83::RST, 4},
			{"LD [$FF00 + a8], A", &SharpSM83::LD_IO,  3}, {"POP HL", &SharpSM83::POP, 3}, {"LD [$FF00 + C], A",  &SharpSM83::LD_IO, 2}, {"NONE",   nullptr,        0}, {"NONE",         nullptr,          0}, {"PUSH HL", &SharpSM83::PUSH, 4}, {"AND d8",    &SharpSM83::AND, 2}, {"RST 20H", &SharpSM83::RST, 4}, {"ADD SP, r8",     &SharpSM83::ADD,     4}, {"JP HL",     &SharpSM83::JP,   1}, {"LD [a16], A", &SharpSM83::LD, 4}, {"NONE", nullptr,        0}, {"NONE",        nullptr,          0}, {"NONE",     nullptr,          0}, {"XOR d8",    &SharpSM83::XOR, 2}, {"RST 28H", &SharpSM83::RST, 4},
			{"LD A, [$FF00 + a8]", &SharpSM83::LD_IO,  3}, {"POP AF", &SharpSM83::POP, 3}, {"LD A, [$FF00 + C]",  &SharpSM83::LD_IO, 2}, {"DI",     &SharpSM83::DI, 1}, {"NONE",         nullptr,          0}, {"PUSH AF", &SharpSM83::PUSH, 4}, {"OR d8",     &SharpSM83::OR,  2}, {"RST 30H", &SharpSM83::RST, 4}, {"LD HL, SP + r8", &SharpSM83::LD_IMM,  3}, {"LD SP, HL", &SharpSM83::LD,   2}, {"LD A, [a16]", &SharpSM83::LD, 4}, {"EI",   &SharpSM83::EI, 1}, {"NONE",        nullptr,          0}, {"NONE",     nullptr,          0}, {"CP d8",     &SharpSM83::CP,  2}, {"RST 38H", &SharpSM83::RST, 4}
		} };

		const std::array<std::function<uint8_t(SharpSM83*, opcode)>, 8> m_TableBitOperations =
		{
			&SharpSM83::RLC,  &SharpSM83::RRC,
			&SharpSM83::RL,   &SharpSM83::RR,
			&SharpSM83::SLA,  &SharpSM83::SRA,
			&SharpSM83::SWAP, &SharpSM83::SRL
		};

	private: //STUFF

		AddressBus& m_Bus;

		uint8_t m_CyclesToFinish;
	};
}