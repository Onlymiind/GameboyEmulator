#pragma once
#include "src/core/gb/cpu/Operation.h"

#include <array>
#include <map>

namespace gb
{
    class Decoder
    {
    public:
        Decoder() = default;
        ~Decoder() = default;

        Instruction decodeUnprefixed(opcode code) const;
        Instruction decodePrefixed(opcode code) const;

        inline bool isPrefix(opcode code) const { return code.code == 0xCB; }
    private:
        void setRegister8Bit(uint8_t registerIndex, ArgumentInfo& registerInfo) const;
        void decodeRandomInstructions(opcode code, Instruction& instruction) const;
        void decodeADD(opcode code, Instruction& instruction) const;
        void decodeLD(opcode code, Instruction& instruction) const;
        void decodeJR(opcode code, Instruction& instruction) const;
        void decodeJP(opcode code, Instruction& instruction) const;
        void decodeINC_DEC(opcode code, Instruction& instruction) const;

    private:
        const std::array<Registers, 8> m_8Bitregisters = 
		{
			Registers::B,  Registers::C,
			Registers::D,  Registers::E,
			Registers::H,  Registers::L,
			Registers::HL, Registers::A 
		};

		//Register pair lookup with SP
		const std::array<Registers, 4> m_16BitRegisters_SP = { Registers::BC, Registers::DE, Registers::HL, Registers::SP };

		//Register pair lookup with AF
		const std::array<Registers, 4> m_16BitRegisters_AF = { Registers::BC, Registers::DE, Registers::HL, Registers::AF };

		//Conditions lookup
		const std::array<Conditions, 4> m_Conditions = 
		{ 
			Conditions::NotZero,
			Conditions::Zero,
			Conditions::NotCarry,
			Conditions::Carry
		};

		const std::array<InstructionType, 8> m_ALU =
		{
			InstructionType::ADD, InstructionType::ADC, InstructionType::SUB, InstructionType::SBC,
			InstructionType::AND, InstructionType::XOR, InstructionType::OR,  InstructionType::CP
		};

		const std::array<InstructionType, 8> m_BitOperations =
		{
			InstructionType::RLC,  InstructionType::RRC,
			InstructionType::RL,   InstructionType::RR,
			InstructionType::SLA,  InstructionType::SRA,
			InstructionType::SWAP, InstructionType::SRL
		};

		const std::map<uint8_t, InstructionType> m_ColumnsUpperQuarter = 
		{
			{ 0x01, InstructionType::LD }, { 0x02, InstructionType::LD }, { 0x03, InstructionType::INC },
            { 0x04, InstructionType::INC }, { 0x05, InstructionType::DEC }, { 0x06, InstructionType::LD },
			{ 0x09, InstructionType::ADD }, { 0x0A, InstructionType::LD }, { 0x0B, InstructionType::DEC },
            { 0x0C, InstructionType::INC }, { 0x0D, InstructionType::DEC }, { 0x0E, InstructionType::LD }
		};

		const std::map<uint8_t, InstructionType> m_ColumnsLowerQuarter =
		{
			{ 0x01, InstructionType::POP }, { 0x05, InstructionType::PUSH }, { 0x07, InstructionType::RST }, { 0x0F, InstructionType::RST }
		};

		const std::map<uint8_t, InstructionType> m_RandomInstructions = 
		{
			{ 0x00, InstructionType::NOP }, { 0x07, InstructionType::RLCA }, { 0x08, InstructionType::LD }, { 0x0F, InstructionType::RRCA },
			
			{ 0x10, InstructionType::STOP }, { 0x17, InstructionType::RLA }, { 0x18, InstructionType::JR }, { 0x1F, InstructionType::RRA },
			
			{ 0x20, InstructionType::JR }, { 0x27, InstructionType::DAA }, { 0x28, InstructionType::JR }, { 0x2F, InstructionType::CPL },
			
			{ 0x30, InstructionType::JR }, { 0x37, InstructionType::SCF }, { 0x38, InstructionType::JR }, { 0x3F, InstructionType::CCF },

			{ 0xC0, InstructionType::RET }, { 0xC2, InstructionType::JP }, { 0xC3, InstructionType::JP }, { 0xC4, InstructionType::CALL },
			{ 0xC8, InstructionType::RET }, { 0xC9, InstructionType::RET }, { 0xCA, InstructionType::JP }, { 0xCC, InstructionType::CALL },
			{ 0xCD, InstructionType::CALL }, 
			
			{ 0xD0, InstructionType::RET }, { 0xD2, InstructionType::JP }, { 0xD4, InstructionType::CALL }, { 0xD8, InstructionType::RET },
			{ 0xD9, InstructionType::RETI }, {0xDA, InstructionType::JP }, { 0xDC, InstructionType::CALL }, 
			
			{ 0xE0, InstructionType::LD }, { 0xE2, InstructionType::LD }, { 0xE8, InstructionType::ADD }, { 0xE9, InstructionType::JP }, 
			{ 0xEA, InstructionType::LD },
			
			{ 0xF0, InstructionType::LD }, { 0xF2, InstructionType::LD }, { 0xF3, InstructionType::DI }, { 0xF8, InstructionType::LD }, 
			{ 0xF9, InstructionType::LD }, { 0xFA, InstructionType::LD }, { 0xFB, InstructionType::EI },
		};
    };

    constexpr int i = sizeof(Decoder);
}

