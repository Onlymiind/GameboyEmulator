#pragma once
#include "core/gb/cpu/Operation.h"

#include <cstdint>
#include <cstddef>
#include <array>
#include <unordered_map>

namespace gb
{
    class Decoder
    {
        struct column_hash
        {
            inline size_t operator()(uint8_t value) const
            {
                return value & 0b1100'1111;
            }
        };
        struct collideable_equal
        {
            inline bool operator()(const uint8_t& lhs, const uint8_t& rhs) const
            {
                return column_hash{}(lhs) == column_hash{}(rhs);
            }
        };
        
    public:
        Decoder() = default;
        ~Decoder() = default;

        Instruction decodeUnprefixed(opcode code) const;
        Instruction decodePrefixed(opcode code) const;

        inline bool isPrefix(opcode code) const { return code.code == 0xCB; }
    private:
        void setRegisterInfo(uint8_t registerIndex, ArgumentInfo& registerInfo) const;
        void setALUInfo(opcode code, Instruction& instruction, bool hasImmediate) const;
        void decodeRandomInstructions(opcode code, Instruction& instruction) const;
        void decodeADD(opcode code, Instruction& instruction) const;
        void decodeLD(opcode code, Instruction& instruction) const;
        void decodeJR(opcode code, Instruction& instruction) const;
        void decodeJP(opcode code, Instruction& instruction) const;
        void decodeINC_DEC(opcode code, Instruction& instruction) const;

    private:
        const std::array<Registers, 8> m_8BitRegisters = 
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

        const std::unordered_map<uint8_t, InstructionType, column_hash, collideable_equal> m_Columns = 
        {
            { 0x01, InstructionType::LD }, { 0x02, InstructionType::LD }, { 0x03, InstructionType::INC },
            { 0x04, InstructionType::INC }, { 0x05, InstructionType::DEC }, { 0x06, InstructionType::LD },
            { 0x09, InstructionType::ADD }, { 0x0A, InstructionType::LD }, { 0x0B, InstructionType::DEC },
            { 0x0C, InstructionType::INC }, { 0x0D, InstructionType::DEC }, { 0x0E, InstructionType::LD },
            { 0xC1, InstructionType::POP }, { 0xC5, InstructionType::PUSH }, { 0xC7, InstructionType::RST },
            { 0xCF, InstructionType::RST }
        };

        const std::unordered_map<uint8_t, InstructionType> m_RandomInstructions = 
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

        //Some LD instructions are a pain to decode, so it is done with this lookup table
        const std::unordered_map<uint8_t, Instruction> m_RandomLD = 
        {
            {0x08, 
                {
                    {}, //Reset vector
                    LoadSubtype::LD_SP, //LD Subtype
                    {}, //Condition
                    {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP}, //Source
                    {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None}, //Destination
                    InstructionType::LD //Instruction type
                }},
            {0xE0, 
                {
                    {},
                    LoadSubtype::LD_IO,
                    {},
                    {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                    {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned8, Registers::None},
                    InstructionType::LD
                }}, 
            {0xF0, 
                {
                    {},
                    LoadSubtype::LD_IO,
                    {},
                    {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned8, Registers::None},
                    {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                    InstructionType::LD
                }},
            {0xE2, 
                {
                    {},
                    LoadSubtype::LD_IO,
                    {},
                    {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                    {ArgumentSource::Indirect, ArgumentType::Unsigned8, Registers::C},
                    InstructionType::LD
                }},
            {0xF2, 
                {
                    {},
                    LoadSubtype::LD_IO,
                    {},
                    {ArgumentSource::Indirect, ArgumentType::Unsigned8, Registers::C},
                    {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                    InstructionType::LD
                }},
            {0xF8, 
                {
                    {},
                    LoadSubtype::LD_Offset_SP,
                    {},
                    {ArgumentSource::Immediate, ArgumentType::Signed8, Registers::None},
                    {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::HL},
                    InstructionType::LD
                }},
            {0xF9, 
                {
                    {},
                    LoadSubtype::Typical,
                    {},
                    {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::HL},
                    {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP},
                    InstructionType::LD
                }},
            {0xEA, 
                {
                    {},
                    LoadSubtype::Typical,
                    {},
                    {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                    {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None},
                    InstructionType::LD
                }},
            {0xFA, 
                {
                    {},
                    LoadSubtype::Typical,
                    {},
                    {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None},
                    {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                    InstructionType::LD
                }}
        };
    };
}

