#pragma once
#include "utils/Utils.h"
#include "core/gb/AddressBus.h"
#include "core/gb/InterruptRegister.h"
#include "core/gb/cpu/Operation.h"

#include <cstdint>
#include <string_view>
#include <string>
#include <array>
#include <map>
#include <functional>

namespace gb 
{
	class SharpSM83 
	{
		using pfn_instruction = uint8_t(*)(SharpSM83&, const opcode);
	public:
		SharpSM83(AddressBus& bus, InterruptRegister& interruptEnable, InterruptRegister& interruptFlags);
		~SharpSM83() {}

		void tick();

		std::string registersOut();

		inline uint16_t getProgramCounter() const { return REG.PC; }

		inline bool isFinished() const { return m_CyclesToFinish == 0; }

		void reset();

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

		InterruptRegister& m_InterruptEnable;
		InterruptRegister& m_InterruptFlags;

	private: //OPCODE DECODING

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

		const std::array<pfn_instruction, 8> m_TableALU =
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

		const std::map<uint8_t, pfn_instruction> m_ColumnToImplUpper = 
		{
			{ 0x01, SharpSM83::LD_IMM }, { 0x02, SharpSM83::LD }, { 0x03, SharpSM83::INC }, { 0x04, SharpSM83::INC }, { 0x05, SharpSM83::DEC }, { 0x06, SharpSM83::LD_IMM },
			{ 0x09, SharpSM83::ADD }, { 0x0A, SharpSM83::LD }, { 0x0B, SharpSM83::DEC }, { 0x0C, SharpSM83::INC }, { 0x0D, SharpSM83::DEC }, { 0x0E, SharpSM83::LD_IMM }
		};

		const std::map<uint8_t, pfn_instruction> m_ColumnToImplLower =
		{
			{ 0x01, SharpSM83::POP }, { 0x05, SharpSM83::PUSH }, { 0x07, SharpSM83::RST }, { 0x0F, SharpSM83::RST }
		};

		const std::map<uint8_t, pfn_instruction> m_RandomInstructions = 
		{
			{ 0x00, SharpSM83::NOP }, { 0x07, SharpSM83::RLCA }, { 0x08, SharpSM83::LD_IMM }, { 0x0F, SharpSM83::RRCA },
			
			{ 0x10, SharpSM83::STOP }, { 0x17, SharpSM83::RLA }, { 0x18, SharpSM83::JR }, { 0x1F, SharpSM83::RRA },
			
			{ 0x20, SharpSM83::JR }, { 0x27, SharpSM83::DAA }, { 0x28, SharpSM83::JR }, { 0x2F, SharpSM83::CPL },
			
			{ 0x30, SharpSM83::JR }, { 0x37, SharpSM83::SCF }, { 0x38, SharpSM83::JR }, { 0x3F, SharpSM83::CCF },

			{ 0xC0, SharpSM83::RET }, { 0xC2, SharpSM83::JP }, { 0xC3, SharpSM83::JP }, { 0xC4, SharpSM83::CALL },
			{ 0xC8, SharpSM83::RET }, { 0xC9, SharpSM83::RET }, { 0xCA, SharpSM83::JP }, { 0xCC, SharpSM83::CALL },
			{ 0xCD, SharpSM83::CALL }, 
			
			{ 0xD0, SharpSM83::RET }, { 0xD2, SharpSM83::JP }, { 0xD4, SharpSM83::CALL }, { 0xD8, SharpSM83::RET },
			{ 0xD9, SharpSM83::RETI }, {0xDA, SharpSM83::JP }, { 0xDC, SharpSM83::CALL }, 
			
			{ 0xE0, SharpSM83::LD_IO }, { 0xE2, SharpSM83::LD_IO }, { 0xE8, SharpSM83::ADD }, { 0xE9, SharpSM83::JP }, 
			{ 0xEA, SharpSM83::LD },
			
			{ 0xF0, SharpSM83::LD_IO }, { 0xF2, SharpSM83::LD_IO }, { 0xF3, SharpSM83::DI }, { 0xF8, SharpSM83::LD_IMM }, 
			{ 0xF9, SharpSM83::LD }, { 0xFA, SharpSM83::LD }, { 0xFB, SharpSM83::EI },
		};


	private: //STUFF

		AddressBus& m_Bus;

		uint8_t m_CyclesToFinish;
	};
}