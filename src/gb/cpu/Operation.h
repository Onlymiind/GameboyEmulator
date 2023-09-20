#pragma once

#include <cstdint>
#include <optional>
#include <iostream>

namespace gb
{
    namespace decoding
    {

        enum class UnprefixedType : uint8_t
        {
            None = 0,

            NOP, LD, INC, RLA, RLCA,
            ADD, JR, DEC, RRA, RRCA,
            SUB, OR, AND, XOR, PUSH,
            ADC, JP, POP, RST, CALL,
            SBC, DI, RET, CPL, RETI,
            CCF, EI, DAA, SCF, HALT,
            CP, STOP
        };

        enum class PrefixedType : uint8_t
        {
            RLC, RRC, SLA, SRA,
            SRL, BIT, RES, SET,
            RL, RR, SWAP
        };

        //Some LD instructions are quite different fron others, this enum is used to mark them
        enum class LoadSubtype : uint8_t
        {
            Typical = 0, 
            LD_INC, LD_DEC, LD_IO, LD_SP, LD_Offset_SP
        };

        enum class ArgumentSource : uint8_t
        {
            None = 0,

            Register, Indirect, Immediate, IndirectImmediate
        };

        enum class ArgumentType : uint8_t
        {
            None = 0,

            Unsigned8, Unsigned16, Signed8
        };

        enum class Registers : uint8_t
        {
            None = 0,
            A, B, C, D, E, H, L,
            AF, BC, DE, HL, PC, SP
        };

        enum class Conditions : uint8_t
        {
            NotZero, Zero, NotCarry, Carry
        };

        struct ArgumentInfo
        {
            ArgumentSource source = ArgumentSource::None;
            ArgumentType type = ArgumentType::None;
            Registers reg = Registers::None;

            inline bool operator ==(ArgumentInfo other) const
            {
                return source == other.source && type == other.type && reg == other.reg;
            }
        };

        struct UnprefixedInstruction
        {
            //Used only in RST instruction. In other cases is left uninitialized
            std::optional<uint16_t> reset_vector;
            //Used only in LD instructions. In other cases is left uninitialized
            std::optional<LoadSubtype> LD_subtype;

            std::optional<Conditions> condition;

            ArgumentInfo source;
            ArgumentInfo destination;

            UnprefixedType type = UnprefixedType::None;

            //For Debugging
            inline bool operator == (UnprefixedInstruction other) const
            {
                return type == other.type &&
                    source == other.source &&
                    destination == other.destination &&
                    condition == other.condition &&
                    reset_vector == other.reset_vector &&
                    LD_subtype == other.LD_subtype;
            }
        };

        struct PrefixedInstruction
        {
            PrefixedType type;

            Registers target = Registers::None;

            std::optional<uint8_t> bit;

        };

        inline bool operator == (PrefixedInstruction lhs, PrefixedInstruction rhs)
        {
            return lhs.type == rhs.type && lhs.target == rhs.target && lhs.bit == rhs.bit;
        }


        //Struct for easy opcode decomposition;
        struct opcode 
        {
            opcode()
                : code(0) 
            {}
            opcode(uint8_t val)
                : code(val) 
            {}

            inline uint8_t getX() const { return (code & 0b11000000) >> 6; }
            inline uint8_t getY() const { return (code & 0b00111000) >> 3; }
            inline uint8_t getZ() const { return (code & 0b00000111) >> 0; }
            inline uint8_t getP() const { return (code & 0b00110000) >> 4; }
            inline uint8_t getQ() const { return (code & 0b00001000) >> 3; }
            
            inline uint8_t getLowerNibble() const { return code & 0x0F; }

            uint8_t code;
        };

        std::ostream& operator<<(std::ostream& os, UnprefixedType type);
        std::ostream& operator<<(std::ostream& os, PrefixedType type);
        //TODO
        // std::ostream& operator<<(std::ostream& os, LoadSubtype subtype);
        // std::ostream& operator<<(std::ostream& os, ArgumentSource source);
        // std::ostream& operator<<(std::ostream& os, ArgumentType arg_type);
        // std::ostream& operator<<(std::ostream& os, Conditions condition);
        // std::ostream& operator<<(std::ostream& os, ArgumentInfo arg_info);
        // std::ostream& operator<<(std::ostream& os, UnprefixedInstruction instr);
        // std::ostream& operator<<(std::ostream& os, PrefixedInstruction instr);
    }
}

