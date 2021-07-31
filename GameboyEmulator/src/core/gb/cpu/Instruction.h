#pragma once
#include "Operation.h"

#include <cstdint>

namespace gb
{
    struct Instruction
    {
        InstructionType Type = InstructionType::None;
    };

    struct RSTInstruction : public Instruction
    {
        uint16_t ResetVector;
    };

    struct LDInstruction : public Instruction
    {
        LoadSubtype Subtype = LoadSubtype::Typical;

        ArgumentInfo Source;
        ArgumentInfo Destination;
    };

    struct ALUInstruction : public Instruction
    {
        ArgumentInfo Argument;
    };

    struct ADDInstruction : public ALUInstruction
    {
        Registers Accumulator;
    };


    struct ConditionalInstruction : public Instruction
    {
        Conditions Condition = Conditions::None;
    };

    struct JumpInstruction : public ConditionalInstruction
    {
        ArgumentType AddressType;
    };
}