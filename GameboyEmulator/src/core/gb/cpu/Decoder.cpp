#include "core/gb/cpu/Decoder.h"

namespace gb
{
    Instruction Decoder::decodeUnprefixed(opcode code)
    {
        Instruction result;

        switch (code.getX())
        {
            case 0:
            {
                if (m_ColumnsUpperQuarter.count(code.getLowerNibble()))
                {
                    result.Type = m_ColumnsUpperQuarter.at(code.getLowerNibble());
                }
                else if (m_RandomInstructions.count(code.code))
                {
                    result.Type = m_RandomInstructions.at(code.code);
                }
                break;
            }
            case 1:
            {
                if (code.getZ() == 6 && code.getY() == 6)
                {
                    result.Type = InstructionType::HALT;
                }
                else
                {
                    result.Type = InstructionType::LD;
                    setRegister8Bit(code.getY(), result.Destination);
                    setRegister8Bit(code.getZ(), result.Source);
                }
                break;
            }
            case 2:
            {
                result.Type = m_ALU[code.getY()];

                result.Destination.Source = ArgumentSource::Register;
                result.Destination.Type = ArgumentType::Unsigned8;
                result.Destination.Register = Registers::A;

                setRegister8Bit(code.getZ(), result.Source);
                break;
            }
            case 3:
            {
                if (m_ColumnsLowerQuarter.count(code.getLowerNibble()))
                {
                    result.Type = m_ColumnsLowerQuarter.at(code.getLowerNibble());
                }
                else if (code.getZ() == 6)
                {
                    result.Type = m_ALU[code.getY()];

                    result.Destination.Source = ArgumentSource::Register;
                    result.Destination.Type = ArgumentType::Unsigned8;
                    result.Destination.Register = Registers::A;

                    result.Source.Source = ArgumentSource::Immediate;
                    result.Source.Type = ArgumentType::Unsigned8;
                }
                else if (m_RandomInstructions.count(code.code))
                {
                    result.Type = m_RandomInstructions.at(code.code);
                }
                
                break;
            }
        }

        return result;
    }

    Instruction Decoder::decodePrefixed(opcode code)
    {
        Instruction result;

        switch (code.getX())
        {
            case 0: 
            {
                result.Type = m_BitOperations[code.getY()];
                break;
            }
            case 1: 
            {
                result.Type = InstructionType::BIT;
                break;
            }
            case 2: 
            {
                result.Type = InstructionType::RES;
                break;
            }
            case 3: 
            {
                result.Type = InstructionType::SET;
                break;
            }
        }

        setRegister8Bit(code.getZ(), result.Source);
        result.Destination = result.Source;

        return result;
    }

    void Decoder::setRegister8Bit(uint8_t registerIndex, ArgumentInfo& registerInfo)
    {
        registerInfo.Register = m_8Bitregisters[registerIndex];
        registerInfo.Type = ArgumentType::Unsigned8;

        if(registerInfo.Register == Registers::HL)
        {
            registerInfo.Source = ArgumentSource::Indirect;
        }
    }
}

