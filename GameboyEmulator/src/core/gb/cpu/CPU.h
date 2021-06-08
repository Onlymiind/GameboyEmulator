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

		uint8_t dispatch(opcode code);

		uint8_t read(uint16_t address);

		void write(uint16_t address, uint8_t data);

		uint8_t fetch();

		uint16_t fetchWord();

		int8_t fetchSigned();

		void pushStack(uint16_t value);

		uint16_t popStack();

		bool halfCarryOccured8Add(uint8_t lhs, uint8_t rhs);

		bool halfCarryOccured8Sub(uint8_t lhs, uint8_t rhs);

		bool carryOccured8Add(uint8_t lhs, uint8_t rhs);

		bool carryOccured8Sub(uint8_t lhs, uint8_t rhs);
		
		bool halfCarryOccured16Add(uint16_t lhs, uint16_t rhs);

		bool carryOccured16Add(uint16_t lhs, uint16_t rhs);



		//Unprefixed instrictions. Return the amount of machine cycles needed for the instruction
		static uint8_t NOP(SharpSM83& cpu, const opcode code); static uint8_t LD(SharpSM83& cpu, const opcode code); static uint8_t INC(SharpSM83& cpu, const opcode code); static uint8_t RLA(SharpSM83& cpu, const opcode code); static uint8_t RLCA(SharpSM83& cpu, const opcode code);
		static uint8_t ADD(SharpSM83& cpu, const opcode code); static uint8_t JR(SharpSM83& cpu, const opcode code); static uint8_t DEC(SharpSM83& cpu, const opcode code); static uint8_t RRA(SharpSM83& cpu, const opcode code); static uint8_t RRCA(SharpSM83& cpu, const opcode code);
		static uint8_t SUB(SharpSM83& cpu, const opcode code); static uint8_t OR(SharpSM83& cpu, const opcode code); static uint8_t AND(SharpSM83& cpu, const opcode code); static uint8_t XOR(SharpSM83& cpu, const opcode code); static uint8_t PUSH(SharpSM83& cpu, const opcode code);
		static uint8_t ADC(SharpSM83& cpu, const opcode code); static uint8_t JP(SharpSM83& cpu, const opcode code); static uint8_t POP(SharpSM83& cpu, const opcode code); static uint8_t RST(SharpSM83& cpu, const opcode code); static uint8_t CALL(SharpSM83& cpu, const opcode code);
		static uint8_t SBC(SharpSM83& cpu, const opcode code); static uint8_t DI(SharpSM83& cpu, const opcode code); static uint8_t RET(SharpSM83& cpu, const opcode code); static uint8_t CPL(SharpSM83& cpu, const opcode code); static uint8_t RETI(SharpSM83& cpu, const opcode code);
		static uint8_t CCF(SharpSM83& cpu, const opcode code); static uint8_t EI(SharpSM83& cpu, const opcode code); static uint8_t DAA(SharpSM83& cpu, const opcode code); static uint8_t HALT(SharpSM83& cpu, const opcode code); static uint8_t LD_IO(SharpSM83& cpu, const opcode code);
		static uint8_t SCF(SharpSM83& cpu, const opcode code); static uint8_t CP(SharpSM83& cpu, const opcode code); static uint8_t STOP(SharpSM83& cpu, const opcode code); static uint8_t LD_REG8(SharpSM83& cpu, const opcode code); static uint8_t LD_IMM(SharpSM83& cpu, const opcode code);

		static uint8_t NONE(SharpSM83& cpu, const opcode code);

		//Prefixed instructions. Return the amount of machine cycles needed for the instruction
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

			uint16_t SP{ 0xFFFE }, PC{ 0x0100 }; // Stack pointer, program counter
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
			[](uint8_t flags) { return (flags & 0b10000000) == 0; }, //Not Z-flag
			[](uint8_t flags) { return (flags & 0b10000000) != 0; }, //Z-flag
			[](uint8_t flags) { return (flags & 0b00010000) == 0; }, //Not C-flag
			[](uint8_t flags) { return (flags & 0b00010000) != 0; }  //C-flag
		};

		const std::array<uint8_t(*)(SharpSM83&, const opcode), 8> m_TableALU =
		{
			SharpSM83::ADD, SharpSM83::ADC, SharpSM83::SUB, SharpSM83::SBC,
			SharpSM83::AND, SharpSM83::XOR, SharpSM83::OR,  SharpSM83::CP
		};

		const std::array<std::function<uint8_t(SharpSM83*, opcode)>, 8> m_TableBitOperations =
		{
			&SharpSM83::RLC,  &SharpSM83::RRC,
			&SharpSM83::RL,   &SharpSM83::RR,
			&SharpSM83::SLA,  &SharpSM83::SRA,
			&SharpSM83::SWAP, &SharpSM83::SRL
		};

		//static constexpr uint32_t i = sizeof(std::array<std::function<uint8_t(SharpSM83*, const opcode)>, 8>);

		const std::array<uint8_t(*)(SharpSM83&, const opcode), 128> m_TableLookup =
		{ {
			/*NOP      */ SharpSM83::NOP,  /*LD BC, d16*/ SharpSM83::LD_IMM, /*LD [BC] A */ SharpSM83::LD, /*INC BC*/ SharpSM83::INC, /*INC B   */ SharpSM83::INC, /*DEC B   */ SharpSM83::DEC, /*LD B, d8   */ SharpSM83::LD_IMM, /*RLCA*/ SharpSM83::RLCA, /*LD [a16], SP*/ SharpSM83::LD_IMM, /*ADD HL, BC*/ SharpSM83::ADD, /*LD A, [BC] */ SharpSM83::LD, /*DEC BC*/ SharpSM83::DEC, /*INC C*/ SharpSM83::INC, /*DEC C*/ SharpSM83::DEC, /*LD C d8*/ SharpSM83::LD_IMM, /*RRCA*/ SharpSM83::RRCA,
			/*STOP     */ SharpSM83::STOP, /*LD DE, d16*/ SharpSM83::LD_IMM, /*LD [DE] A */ SharpSM83::LD, /*INC DE*/ SharpSM83::INC, /*INC D   */ SharpSM83::INC, /*DEC D   */ SharpSM83::DEC, /*LD D, d8   */ SharpSM83::LD_IMM, /*RLA */ SharpSM83::RLA,  /*JR r8       */ SharpSM83::JR,     /*ADD HL, DE*/ SharpSM83::ADD, /*LD A, [DE] */ SharpSM83::LD, /*DEC DE*/ SharpSM83::DEC, /*INC E*/ SharpSM83::INC, /*DEC E*/ SharpSM83::DEC, /*LD E d8*/ SharpSM83::LD_IMM, /*RRA */ SharpSM83::RRA,
			/*JR NZ, r8*/ SharpSM83::JR,   /*LD HL, d16*/ SharpSM83::LD_IMM, /*LD [HL+] A*/ SharpSM83::LD, /*INC HL*/ SharpSM83::INC, /*INC H   */ SharpSM83::INC, /*DEC H   */ SharpSM83::DEC, /*LD H, d8   */ SharpSM83::LD_IMM, /*DAA */ SharpSM83::DAA,  /*JR Z, r8    */ SharpSM83::JR,     /*ADD HL, HL*/ SharpSM83::ADD, /*LD A, [HL+]*/ SharpSM83::LD, /*DEC HL*/ SharpSM83::DEC, /*INC L*/ SharpSM83::INC, /*DEC L*/ SharpSM83::DEC, /*LD L d8*/ SharpSM83::LD_IMM, /*CPL */ SharpSM83::CPL,
			/*JR NC, r8*/ SharpSM83::JR,   /*LD SP, d16*/ SharpSM83::LD_IMM, /*LD [HL-] A*/ SharpSM83::LD, /*INC SP*/ SharpSM83::INC, /*INC [HL]*/ SharpSM83::INC, /*DEC [HL]*/ SharpSM83::DEC, /*LD [HL], d8*/ SharpSM83::LD_IMM, /*SCF */ SharpSM83::SCF,  /*JR C, r8    */ SharpSM83::JR,     /*ADD HL, SP*/ SharpSM83::ADD, /*LD A, [HL-]*/ SharpSM83::LD, /*DEC SP*/ SharpSM83::DEC, /*INC A*/ SharpSM83::INC, /*DEC A*/ SharpSM83::DEC, /*LD A d8*/ SharpSM83::LD_IMM, /*CCF */ SharpSM83::CCF,
			
			/*8-bit load and HALT*/

			/*ALU instructions*/
			
			/*RET NZ            */ SharpSM83::RET,   /*POP BC*/ SharpSM83::POP, /*JP NZ, a16       */  SharpSM83::JP,    /*JP a16*/ SharpSM83::JP, /*CALL NZ, a16*/ SharpSM83::CALL, /*PUSH BC*/ SharpSM83::PUSH, /*ADD A, d8*/ SharpSM83::ADD, /*RST 00H*/ SharpSM83::RST, /*RET Z         */ SharpSM83::RET,    /*RET      */ SharpSM83::RET,  /*JP Z, a16  */ SharpSM83::JP, /*CB  */ nullptr,       /*CALL Z, a16*/ SharpSM83::CALL, /*CALL a16*/ SharpSM83::CALL, /*ADC A, d8*/ SharpSM83::ADC, /*RST 08H*/ SharpSM83::RST,
			/*RET NC            */ SharpSM83::RET,   /*POP DE*/ SharpSM83::POP, /*JP NC, a16       */  SharpSM83::JP,    /*NONE  */ nullptr,       /*CALL NC, a16*/ SharpSM83::CALL, /*PUSH DE*/ SharpSM83::PUSH, /*SUB d8   */ SharpSM83::SUB, /*RST 10H*/ SharpSM83::RST, /*RET C         */ SharpSM83::RET,    /*RETI     */ SharpSM83::RETI, /*JP C, a16  */ SharpSM83::JP, /*NONE*/ nullptr,       /*CALL C, a16*/ SharpSM83::CALL, /*NONE    */ nullptr,         /*SBC A, d8*/ SharpSM83::SBC, /*RST 18H*/ SharpSM83::RST,
			/*LD [$FF00 + a8], A*/ SharpSM83::LD_IO, /*POP HL*/ SharpSM83::POP, /*LD [$FF00 + C], A*/  SharpSM83::LD_IO, /*NONE  */ nullptr,       /*NONE        */ nullptr,         /*PUSH HL*/ SharpSM83::PUSH, /*AND d8   */ SharpSM83::AND, /*RST 20H*/ SharpSM83::RST, /*ADD SP, r8    */ SharpSM83::ADD,    /*JP HL    */ SharpSM83::JP,   /*LD [a16], A*/ SharpSM83::LD, /*NONE*/ nullptr,       /*NONE       */ nullptr,         /*NONE    */ nullptr,         /*XOR d8   */ SharpSM83::XOR, /*RST 28H*/ SharpSM83::RST,
			/*LD A, [$FF00 + a8]*/ SharpSM83::LD_IO, /*POP AF*/ SharpSM83::POP, /*LD A, [$FF00 + C]*/  SharpSM83::LD_IO, /*DI    */ SharpSM83::DI, /*NONE        */ nullptr,         /*PUSH AF*/ SharpSM83::PUSH, /*OR d8    */ SharpSM83::OR,  /*RST 30H*/ SharpSM83::RST, /*LD HL, SP + r8*/ SharpSM83::LD_IMM, /*LD SP, HL*/ SharpSM83::LD,   /*LD A, [a16]*/ SharpSM83::LD, /*EI  */ SharpSM83::EI, /*NONE       */ nullptr,         /*NONE    */ nullptr,         /*CP d8    */ SharpSM83::CP,  /*RST 38H*/ SharpSM83::RST 
		} };


	private: //STUFF

		AddressBus& m_Bus;

		uint8_t m_CyclesToFinish;

		//Used to get correct offset for lookup tables during decoding
		static constexpr uint32_t k_BlockSize = 256 / 4;
	};
}