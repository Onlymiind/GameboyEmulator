#include "core/gb/cpu/Decoder.h"

#include <utility>

namespace gb
{
    using type = InstructionType;
    using arg_src = ArgumentSource;
    using arg_t = ArgumentType;
    using reg = Registers;

    UnprefixedInstruction Decoder::decodeUnprefixed(opcode code) const
    {
        UnprefixedInstruction result;

        switch (code.getX())
        {
            case 1:
            {
                if (code.getZ() == 6 && code.getY() == 6)
                {
                    result.Type = InstructionType::HALT;
                }
                else
                {
                    result.Type = InstructionType::LD;
                    setRegisterInfo(code.getY(), result.Destination);
                    setRegisterInfo(code.getZ(), result.Source);
                }
                break;
            }
            case 2:
            {
                setALUInfo(code, result, false);
                break;
            }
            case 3:
            {
                if (code.getZ() == 6)
                {
                    setALUInfo(code, result, true);
                }
                break;
            }
        }

        if(result.Type == InstructionType::None)
        {
            bool isRandomLD = false;
            if(m_Columns.count(code.code))
            {
                result.Type = m_Columns.at(code.code);
            }
            else
            {
                result.Type = m_RandomInstructions.at(code.code);
                if(result.Type == InstructionType::LD)
                {
                    isRandomLD = true;
                }
            }
            if(!isRandomLD)
            {
                decodeRandomInstructions(code, result);
            }
            else
            {
                result = m_RandomLD.at(code.code);
            }
        }

        return result;
    }

    void Decoder::decodeRandomInstructions(opcode code, UnprefixedInstruction& instruction) const
    {
        switch(instruction.Type)
        {
            case type::NOP:
            case type::STOP:
            case type::SCF:
            case type::CCF:
            case type::CPL:
            case type::DAA:
            case type::DI:
            case type::EI:
            case type::RETI:
            case type::RLCA:
            case type::RRCA:
            case type::RLA:
            case type::RRA:
                return;
            case type::RST:
                instruction.ResetVector = code.getY() * 8;
                return;
            case type::POP:
                instruction.Destination.Register = m_16BitRegisters_AF[code.getP()];
                return;
            case type::PUSH:
                instruction.Source.Register = m_16BitRegisters_AF[code.getP()];
                return;
            case type::RET:
                if(code.getZ() == 0)
                {
                    instruction.Condition = m_Conditions[code.getY()];
                }
                return;
            case type::ADD:
                decodeADD(code, instruction);
                return;
            case type::JR:
                decodeJR(code, instruction);
                return;
            case type::JP:
                decodeJP(code, instruction);
                return;
            case type::CALL:
                if(code.getZ() == 4)
                {
                    instruction.Condition = m_Conditions[code.getY()];
                }
                return;
            case type::INC:
            case type::DEC:
                decodeINC_DEC(code, instruction);
                return;
            case type::LD:
                decodeLD(code, instruction);
                return;
        }
    }

    void Decoder::decodeADD(opcode code, UnprefixedInstruction& instruction) const
    {
        switch(code.getZ())
        {
            case 0:
                instruction.Destination.Source = arg_src::Register;
                instruction.Destination.Type = arg_t::Unsigned16;
                instruction.Destination.Register = reg::SP;

                instruction.Source.Source = arg_src::Immediate;
                instruction.Source.Type = arg_t::Signed8;
                break;
            case 1:
                instruction.Destination.Source = arg_src::Register;
                instruction.Destination.Type = arg_t::Unsigned16;
                instruction.Destination.Register = reg::HL;

                instruction.Source.Source = arg_src::Register;
                instruction.Source.Type = arg_t::Unsigned16;
                instruction.Source.Register = m_16BitRegisters_SP[code.getP()];
                break;
        }
    }

    void Decoder::decodeLD(opcode code, UnprefixedInstruction& instruction) const
    {
        switch(code.getZ())
        {
            case 2:
                switch(code.getP())
                {
                    case 0:
                        instruction.Destination.Register = reg::BC;
                        break;
                    case 1:
                        instruction.Destination.Register = reg::DE;
                        break;
                    case 2:
                        instruction.Destination.Register = reg::HL;
                        instruction.LDSubtype = LoadSubtype::LD_INC;
                        break;
                    case 3:
                        instruction.Destination.Register = reg::HL;
                        instruction.LDSubtype = LoadSubtype::LD_DEC;
                        break;
                }

                instruction.Source.Source = arg_src::Register;
                instruction.Source.Type = arg_t::Unsigned8;
                instruction.Source.Register = reg::A;

                instruction.Destination.Source = arg_src::Indirect;
                instruction.Destination.Type = arg_t::Unsigned8;

                if(code.getQ())
                {
                    std::swap(instruction.Destination, instruction.Source);
                }
                break;
            case 6:
                instruction.Source.Source = arg_src::Immediate;
                instruction.Source.Type = arg_t::Unsigned8;
                                
                setRegisterInfo(code.getY(), instruction.Destination);
                break;
        }
    }

    void Decoder::decodeJR(opcode code, UnprefixedInstruction& instruction) const
    {
        if(code.getY() != 3)
        {
            instruction.Condition = m_Conditions[code.getY() - 4];
        }
        instruction.Source.Source = arg_src::Immediate;
        instruction.Source.Type = arg_t::Signed8;
    }

    void Decoder::decodeJP(opcode code, UnprefixedInstruction& instruction) const
    {
        switch(code.getZ())
        {
            case 1:
                instruction.Source.Source = arg_src::Register;
                instruction.Source.Type = arg_t::Unsigned16;
                instruction.Source.Register = reg::HL;
                break;
            case 2:
                instruction.Condition = m_Conditions[code.getY()];
                [[fallthrough]];
            case 3:
                instruction.Source.Source = arg_src::Immediate;
                instruction.Source.Type = arg_t::Unsigned16;
                break;
        }
    }

    void Decoder::decodeINC_DEC(opcode code, UnprefixedInstruction& instruction) const
    {
        switch(code.getZ())
        {
            case 3:
                instruction.Source.Source = arg_src::Register;
                instruction.Source.Type = arg_t::Unsigned16;
                instruction.Source.Register = m_16BitRegisters_SP[code.getP()];
                break;
            case 4:
                setRegisterInfo(code.getY(), instruction.Source);
                break;
        }
        instruction.Destination = instruction.Source;
    }

    PrefixedInstruction Decoder::decodePrefixed(opcode code) const
    {
        PrefixedInstruction result;

        switch (code.getX())
        {
            case 0: 
            {
                result.Type = m_BitOperations[code.getY()];
                break;
            }
            case 1: 
            {
                result.Type = type::BIT;
                result.Bit = code.getY();
                break;
            }
            case 2: 
            {
                result.Type = type::RES;
                result.Bit = code.getY();
                break;
            }
            case 3: 
            {
                result.Type = type::SET;
                result.Bit = code.getY();
                break;
            }
        }
        result.Target = m_8BitRegisters[code.getZ()];

        return result;
    }

    void Decoder::setRegisterInfo(uint8_t registerIndex, ArgumentInfo& registerInfo) const
    {
        registerInfo.Register = m_8BitRegisters[registerIndex];
        registerInfo.Type = arg_t::Unsigned8;

        if(registerInfo.Register != reg::HL)
        {
            registerInfo.Source = arg_src::Register;
        }
        else
        {
            registerInfo.Source = arg_src::Indirect;
        }
    }

    void Decoder::setALUInfo(opcode code, UnprefixedInstruction& instruction, bool hasImmediate) const
    {
        instruction.Type = m_ALU[code.getY()];

        instruction.Destination.Source = arg_src::Register;
        instruction.Destination.Type = arg_t::Unsigned8;
        instruction.Destination.Register = reg::A;
                    
        if(hasImmediate)
        {
            instruction.Source.Source = arg_src::Immediate;
            instruction.Source.Type = arg_t::Unsigned8;
        }
        else
        {
            setRegisterInfo(code.getZ(), instruction.Source);
        }
    }
}

