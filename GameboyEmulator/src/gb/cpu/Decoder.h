#pragma once
#include "gb/cpu/Operation.h"

#include <cstdint>
#include <cstddef>
#include <array>
#include <unordered_map>

namespace gb
{
    namespace decoding
    {

        class Decoder
        {
            using type = UnprefixedType;
            using pref_type = PrefixedType;
            using arg_src = ArgumentSource;
            using arg_t = ArgumentType;
            using reg = Registers;
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

            UnprefixedInstruction decodeUnprefixed(opcode code) const;
            PrefixedInstruction decodePrefixed(opcode code) const;

            inline bool isPrefix(opcode code) const { return code.code == 0xCB; }
        private:
            void setRegisterInfo(uint8_t registerIndex, ArgumentInfo& registerInfo) const;
            void setALUInfo(opcode code, UnprefixedInstruction& instruction, bool hasImmediate) const;
            void decodeRandomInstructions(opcode code, UnprefixedInstruction& instruction) const;
            void decodeADD(opcode code, UnprefixedInstruction& instruction) const;
            void decodeLD(opcode code, UnprefixedInstruction& instruction) const;
            void decodeJR(opcode code, UnprefixedInstruction& instruction) const;
            void decodeJP(opcode code, UnprefixedInstruction& instruction) const;
            void decodeINC_DEC(opcode code, UnprefixedInstruction& instruction) const;

        private:
            const std::array<Registers, 8> byte_registers_ = 
            {
                Registers::B,  Registers::C,
                Registers::D,  Registers::E,
                Registers::H,  Registers::L,
                Registers::HL, Registers::A 
            };

            //Register pair lookup with SP
            const std::array<Registers, 4> word_registers_SP_ = { Registers::BC, Registers::DE, Registers::HL, Registers::SP };

            //Register pair lookup with AF
            const std::array<Registers, 4> word_registers_AF_ = { Registers::BC, Registers::DE, Registers::HL, Registers::AF };

            //Conditions lookup
            const std::array<Conditions, 4> conditions_ = 
            { 
                Conditions::NotZero,
                Conditions::Zero,
                Conditions::NotCarry,
                Conditions::Carry
            };

            const std::array<UnprefixedType, 8> ALU_ =
            {
                UnprefixedType::ADD, UnprefixedType::ADC, UnprefixedType::SUB, UnprefixedType::SBC,
                UnprefixedType::AND, UnprefixedType::XOR, UnprefixedType::OR,  UnprefixedType::CP
            };

            const std::array<PrefixedType, 8> bit_operations_ =
            {
                PrefixedType::RLC,  PrefixedType::RRC,
                PrefixedType::RL,   PrefixedType::RR,
                PrefixedType::SLA,  PrefixedType::SRA,
                PrefixedType::SWAP, PrefixedType::SRL
            };

            const std::unordered_map<uint8_t, UnprefixedType, column_hash, collideable_equal> columns_ = 
            {
                { 0x01, UnprefixedType::LD }, { 0x02, UnprefixedType::LD }, { 0x03, UnprefixedType::INC },
                { 0x04, UnprefixedType::INC }, { 0x05, UnprefixedType::DEC }, { 0x06, UnprefixedType::LD },
                { 0x09, UnprefixedType::ADD }, { 0x0A, UnprefixedType::LD }, { 0x0B, UnprefixedType::DEC },
                { 0x0C, UnprefixedType::INC }, { 0x0D, UnprefixedType::DEC }, { 0x0E, UnprefixedType::LD },
                { 0xC1, UnprefixedType::POP }, { 0xC5, UnprefixedType::PUSH }, { 0xC7, UnprefixedType::RST },
                { 0xCF, UnprefixedType::RST }
            };

            const std::unordered_map<uint8_t, UnprefixedType> random_instructions_ = 
            {
                { 0x00, UnprefixedType::NOP }, { 0x07, UnprefixedType::RLCA }, { 0x08, UnprefixedType::LD }, { 0x0F, UnprefixedType::RRCA },
                
                { 0x10, UnprefixedType::STOP }, { 0x17, UnprefixedType::RLA }, { 0x18, UnprefixedType::JR }, { 0x1F, UnprefixedType::RRA },
                
                { 0x20, UnprefixedType::JR }, { 0x27, UnprefixedType::DAA }, { 0x28, UnprefixedType::JR }, { 0x2F, UnprefixedType::CPL },
                
                { 0x30, UnprefixedType::JR }, { 0x37, UnprefixedType::SCF }, { 0x38, UnprefixedType::JR }, { 0x3F, UnprefixedType::CCF },

                { 0xC0, UnprefixedType::RET }, { 0xC2, UnprefixedType::JP }, { 0xC3, UnprefixedType::JP }, { 0xC4, UnprefixedType::CALL },
                { 0xC8, UnprefixedType::RET }, { 0xC9, UnprefixedType::RET }, { 0xCA, UnprefixedType::JP }, { 0xCC, UnprefixedType::CALL },
                { 0xCD, UnprefixedType::CALL }, 
                
                { 0xD0, UnprefixedType::RET }, { 0xD2, UnprefixedType::JP }, { 0xD4, UnprefixedType::CALL }, { 0xD8, UnprefixedType::RET },
                { 0xD9, UnprefixedType::RETI }, {0xDA, UnprefixedType::JP }, { 0xDC, UnprefixedType::CALL }, 
                
                { 0xE0, UnprefixedType::LD }, { 0xE2, UnprefixedType::LD }, { 0xE8, UnprefixedType::ADD }, { 0xE9, UnprefixedType::JP }, 
                { 0xEA, UnprefixedType::LD },
                
                { 0xF0, UnprefixedType::LD }, { 0xF2, UnprefixedType::LD }, { 0xF3, UnprefixedType::DI }, { 0xF8, UnprefixedType::LD }, 
                { 0xF9, UnprefixedType::LD }, { 0xFA, UnprefixedType::LD }, { 0xFB, UnprefixedType::EI },
            };

            //Some LD instructions are a pain to decode, so it is done with this lookup table
            const std::unordered_map<uint8_t, UnprefixedInstruction> random_LD_ = 
            {
                {0x08, 
                    {
                        {}, //Reset vector
                        LoadSubtype::LD_SP, //LD Subtype
                        {}, //Condition
                        {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP}, //Source
                        {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None}, //Destination
                        UnprefixedType::LD //Instruction type
                    }},
                {0xE0, 
                    {
                        {},
                        LoadSubtype::LD_IO,
                        {},
                        {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                        {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None},
                        UnprefixedType::LD
                    }}, 
                {0xF0, 
                    {
                        {},
                        LoadSubtype::LD_IO,
                        {},
                        {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None},
                        {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                        UnprefixedType::LD
                    }},
                {0xE2, 
                    {
                        {},
                        LoadSubtype::LD_IO,
                        {},
                        {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                        {ArgumentSource::Indirect, ArgumentType::Unsigned8, Registers::C},
                        UnprefixedType::LD
                    }},
                {0xF2, 
                    {
                        {},
                        LoadSubtype::LD_IO,
                        {},
                        {ArgumentSource::Indirect, ArgumentType::Unsigned8, Registers::C},
                        {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                        UnprefixedType::LD
                    }},
                {0xF8, 
                    {
                        {},
                        LoadSubtype::LD_Offset_SP,
                        {},
                        {ArgumentSource::Immediate, ArgumentType::Signed8, Registers::None},
                        {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::HL},
                        UnprefixedType::LD
                    }},
                {0xF9, 
                    {
                        {},
                        LoadSubtype::Typical,
                        {},
                        {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::HL},
                        {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP},
                        UnprefixedType::LD
                    }},
                {0xEA, 
                    {
                        {},
                        LoadSubtype::Typical,
                        {},
                        {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                        {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None},
                        UnprefixedType::LD
                    }},
                {0xFA, 
                    {
                        {},
                        LoadSubtype::Typical,
                        {},
                        {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None},
                        {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                        UnprefixedType::LD
                    }}
            };
        };
    }
}

