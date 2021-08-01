#pragma once

#include <cstdint>
#include <optional>

namespace gb
{
    enum class InstructionType : uint8_t
    {
        None = 0,

        NOP, LD, INC, RLA, RLCA,
        ADD, JR, DEC, RRA, RRCA,
        SUB, OR, AND, XOR, PUSH,
        ADC, JP, POP, RST, CALL,
        SBC, DI, RET, CPL, RETI,
        CCF, EI, DAA, SCF, HALT,
        CP, STOP, LD_INC, LD_DEC,

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
        None = 0,
        NotZero, Zero, NotCarry, Carry
    };

    struct ArgumentInfo
    {
        ArgumentSource Source = ArgumentSource::None;
        ArgumentType Type = ArgumentType::None;
        Registers Register = Registers::None;

        inline bool operator ==(ArgumentInfo other) const
        {
            return Source == other.Source && Type == other.Type && Register == other.Register;
        }
    };

    struct UnprefixedInstruction
    {
        //Used only in RST instruction. In other cases is left uninitialized
        std::optional<uint16_t> ResetVector;
        //Used only in LD instructions. In other cases is left uninitialized
        std::optional<LoadSubtype> LDSubtype;

        std::optional<Conditions> Condition;

        ArgumentInfo Source;
        ArgumentInfo Destination;

        InstructionType Type = InstructionType::None;

        //For Debugging
        inline bool operator == (UnprefixedInstruction other) const
        {
            return Type == other.Type &&
                Source == other.Source &&
                Destination == other.Destination &&
                Condition == other.Condition &&
                ResetVector == other.ResetVector &&
                LDSubtype == other.LDSubtype;
        }
    };

    struct PrefixedInstruction
    {
        InstructionType Type = InstructionType::None;

        Registers Target = Registers::None;

        std::optional<uint8_t> Bit;

        inline bool operator == (PrefixedInstruction other)
        {
            return Type == other.Type && Target == other.Target && Bit == other.Bit;
        }
    };



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
}

