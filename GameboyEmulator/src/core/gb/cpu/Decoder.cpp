#include "core/gb/cpu/Decoder.h"

#include <utility>

namespace gb
{
    using type = InstructionType;
    using arg_src = ArgumentSource;
    using arg_t = ArgumentType;
    using reg = Registers;

    Instruction Decoder::decodeUnprefixed(opcode code) const
    {
        Instruction result;

        switch (code.getX())
        {
            case 0:
            {
                if (m_ColumnsUpperQuarter.count(code.getLowerNibble()))
                {
                    result.Type = m_ColumnsUpperQuarter.at(code.getLowerNibble());
                    decodeRandomInstructions(code, result);
                }
                else if (m_RandomInstructions.count(code.code))
                {
                    result.Type = m_RandomInstructions.at(code.code);
                    decodeRandomInstructions(code, result);
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
                    decodeRandomInstructions(code, result);
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
                    decodeRandomInstructions(code, result);
                }
                
                break;
            }
        }

        return result;
    }

    void Decoder::decodeRandomInstructions(opcode code, Instruction& instruction) const
    {
        bool LD_INC = false;
        bool LD_DEC = false;

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
                switch(code.getX())
                {
                    case 1:
                        switch(code.getZ())
                        {
                            case 0:
                                instruction.Destination.Source = arg_src::IndirectImmediate;
                                instruction.Destination.Type = arg_t::Unsigned16;

                                instruction.Source.Source = arg_src::Register;
                                instruction.Source.Type = arg_t::Unsigned16;
                                instruction.Source.Register = reg::SP;
                                break;
                            case 1:
                                instruction.Destination.Source = arg_src::Register;
                                instruction.Destination.Type = arg_t::Unsigned16;
                                instruction.Destination.Register = m_16BitRegisters_SP[code.getP()];

                                instruction.Source.Source = arg_src::Immediate;
                                instruction.Source.Type = arg_t::Unsigned16;
                                break;
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
                                        LD_INC = true;
                                        break;
                                    case 3:
                                        instruction.Destination.Register = reg::HL;
                                        LD_DEC = true;
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
                                
                                setRegister8Bit(code.getY(), instruction.Destination);
                                break;
                        }
                        break;
                    case 3:
                        switch(code.getZ())
                        {
                            case 0:
                                switch(code.getY())
                                {
                                    case 4:
                                        instruction.Source.Source = arg_src::Register;
                                        instruction.Source.Type = arg_t::Unsigned8;
                                        instruction.Source.Register = reg::A;

                                        instruction.Destination.Source = arg_src::IndirectImmediate;
                                        instruction.Destination.Type = arg_t::Unsigned8;
                                        break;
                                    case 6:
                                        instruction.Source.Source = arg_src::IndirectImmediate;
                                        instruction.Source.Type = arg_t::Unsigned8;
        
                                        instruction.Destination.Source = arg_src::Register;
                                        instruction.Destination.Type = arg_t::Unsigned8;
                                        instruction.Destination.Register = reg::A;
                                        break;
                                    case 7:
                                        instruction.Source.Source = arg_src::Immediate;
                                        instruction.Source.Type = arg_t::Signed8;
                                        instruction.Source.Register = reg::SP;
        
                                        instruction.Destination.Source = arg_src::Register;
                                        instruction.Destination.Type = arg_t::Unsigned16;
                                        instruction.Destination.Register = reg::HL;
                                        break;
                                }
                            case 1:
                                instruction.Source.Source = arg_src::Register;
                                instruction.Source.Type = arg_t::Unsigned16;
                                instruction.Source.Register = reg::HL;

                                instruction.Destination.Source = arg_src::Register;
                                instruction.Destination.Type = arg_t::Unsigned16;
                                instruction.Destination.Register = reg::SP;
                                break;
                            case 2:
                                switch(code.getY())
                                {
                                    case 4:
                                        instruction.Source.Source = arg_src::Register;
                                        instruction.Source.Type = arg_t::Unsigned8;
                                        instruction.Source.Register = reg::A;

                                        instruction.Destination.Source = arg_src::Indirect;
                                        instruction.Destination.Type = arg_t::Unsigned8;
                                        instruction.Destination.Register = reg::C;
                                        break;
                                    case 5:
                                        instruction.Source.Source = arg_src::Register;
                                        instruction.Source.Type = arg_t::Unsigned8;
                                        instruction.Source.Register = reg::A;
        
                                        instruction.Destination.Source = arg_src::IndirectImmediate;
                                        instruction.Destination.Type = arg_t::Unsigned16;
                                        break;
                                    case 6:
                                        instruction.Source.Source = arg_src::Indirect;
                                        instruction.Source.Type = arg_t::Unsigned8;
                                        instruction.Source.Register = reg::C;
        
                                        instruction.Destination.Source = arg_src::Register;
                                        instruction.Destination.Type = arg_t::Unsigned8;
                                        instruction.Destination.Register = reg::A;
                                        break;
                                    case 7:
                                        instruction.Source.Source = arg_src::IndirectImmediate;
                                        instruction.Source.Type = arg_t::Unsigned16;
        
                                        instruction.Destination.Source = arg_src::Register;
                                        instruction.Destination.Type = arg_t::Unsigned8;
                                        instruction.Destination.Register = reg::A;
                                        break;
                                }
                                break;
                        }
                        break;
                }
                break;
        }

        if(LD_DEC)
        {
            instruction.Type = type::LD_DEC;
        }
        else if(LD_INC)
        {
            instruction.Type = type::LD_INC;
        }
    }

    void Decoder::decodeADD(opcode code, Instruction& instruction) const
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

    void Decoder::decodeLD(opcode code, Instruction& instruction) const
    {

    }

    void Decoder::decodeJR(opcode code, Instruction& instruction) const
    {
        if(code.getY() != 3)
        {
            instruction.Condition = m_Conditions[code.getY() - 4];
        }
        instruction.Source.Source = arg_src::Immediate;
        instruction.Source.Type = arg_t::Signed8;
    }

    void Decoder::decodeJP(opcode code, Instruction& instruction) const
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

    void Decoder::decodeINC_DEC(opcode code, Instruction& instruction) const
    {
        switch(code.getZ())
        {
            case 3:
                instruction.Source.Source = arg_src::Register;
                instruction.Source.Type = arg_t::Unsigned16;
                instruction.Source.Register = m_16BitRegisters_SP[code.getP()];
                break;
            case 4:
                setRegister8Bit(code.getY(), instruction.Source);
                break;
        }
        instruction.Destination = instruction.Source;
    }

    Instruction Decoder::decodePrefixed(opcode code) const
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

    void Decoder::setRegister8Bit(uint8_t registerIndex, ArgumentInfo& registerInfo) const
    {
        registerInfo.Register = m_8Bitregisters[registerIndex];
        registerInfo.Type = ArgumentType::Unsigned8;

        if(registerInfo.Register == Registers::HL)
        {
            registerInfo.Source = ArgumentSource::Indirect;
        }
    }
}

