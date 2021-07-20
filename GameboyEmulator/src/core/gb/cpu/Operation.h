#pragma once

#include <cstdint>

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
        CP, STOP,

        RLC, RRC, SLA, SRA,
        SRL, BIT, RES, SET,
        RL, RR, SWAP
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
        A, F, B, C, D, E, H, L,
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

        inline bool operator ==(ArgumentInfo other)
        {
            return Source == other.Source && Type == other.Type && Register == other.Register;
        }
    };

    struct Instruction
    {
        InstructionType Type = InstructionType::None;
        ArgumentInfo Source;
        ArgumentInfo Destination;
        Conditions Condition = Conditions::None;

        //For Debugging
        inline bool operator == (Instruction other)
        {
            return Type == other.Type &&
                Source == other.Source &&
                Destination == other.Destination &&
                Condition == other.Condition;
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

