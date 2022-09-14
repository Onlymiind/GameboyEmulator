#include "gb/cpu/Decoder.h"

#include <utility>

namespace gb
{
    namespace decoding
    {

        UnprefixedInstruction Decoder::decodeUnprefixed(opcode code) const
        {
            UnprefixedInstruction result;

            switch (code.getX())
            {
                case 1:
                {
                    if (code.getZ() == 6 && code.getY() == 6)
                    {
                        result.type = type::HALT;
                    }
                    else
                    {
                        result.type = type::LD;
                        result.LD_subtype = LoadSubtype::Typical;
                        setRegisterInfo(code.getY(), result.destination);
                        setRegisterInfo(code.getZ(), result.source);
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

            if(result.type == type::None)
            {
                bool isRandomLD = false;
                if(columns_.count(code.code))
                {
                    result.type = columns_.at(code.code);
                }
                else
                {
                    result.type = random_instructions_.at(code.code);
                    if(result.type == type::LD)
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
                    result = random_LD_.at(code.code);
                }
            }

            return result;
        }

        void Decoder::decodeRandomInstructions(opcode code, UnprefixedInstruction& instruction) const
        {
            switch(instruction.type)
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
                    instruction.reset_vector = code.getY() * 8;
                    return;
                case type::POP:
                    instruction.destination.reg = word_registers_AF_[code.getP()];
                    return;
                case type::PUSH:
                    instruction.source.reg = word_registers_AF_[code.getP()];
                    return;
                case type::RET:
                    if(code.getZ() == 0)
                    {
                        instruction.condition = conditions_[code.getY()];
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
                        instruction.condition = conditions_[code.getY()];
                    }
                    return;
                case type::INC:
                case type::DEC:
                    decodeINC_DEC(code, instruction);
                    return;
                case type::LD:
                    decodeLD(code, instruction);
                    return;
                default:
                    return;
            }
        }

        void Decoder::decodeADD(opcode code, UnprefixedInstruction& instruction) const
        {
            switch(code.getZ())
            {
                case 0:
                    instruction.destination.source = arg_src::Register;
                    instruction.destination.type = arg_t::Unsigned16;
                    instruction.destination.reg = reg::SP;

                    instruction.source.source = arg_src::Immediate;
                    instruction.source.type = arg_t::Signed8;
                    break;
                case 1:
                    instruction.destination.source = arg_src::Register;
                    instruction.destination.type = arg_t::Unsigned16;
                    instruction.destination.reg = reg::HL;

                    instruction.source.source = arg_src::Register;
                    instruction.source.type = arg_t::Unsigned16;
                    instruction.source.reg = word_registers_SP_[code.getP()];
                    break;
            }
        }

        void Decoder::decodeLD(opcode code, UnprefixedInstruction& instruction) const
        {
            switch(code.getZ())
            {
                case 1:
                    instruction.destination.reg = word_registers_SP_[code.getP()];
                    instruction.destination.source = arg_src::Register;
                    instruction.destination.type = arg_t::Unsigned16;

                    instruction.source.source = arg_src::Immediate;
                    instruction.source.type = arg_t::Unsigned16;
                    break;
                case 2:
                    switch(code.getP())
                    {
                        case 0:
                            instruction.destination.reg = reg::BC;
                            break;
                        case 1:
                            instruction.destination.reg = reg::DE;
                            break;
                        case 2:
                            instruction.destination.reg = reg::HL;
                            instruction.LD_subtype = LoadSubtype::LD_INC;
                            break;
                        case 3:
                            instruction.destination.reg = reg::HL;
                            instruction.LD_subtype = LoadSubtype::LD_DEC;
                            break;
                    }

                    instruction.source.source = arg_src::Register;
                    instruction.source.type = arg_t::Unsigned8;
                    instruction.source.reg = reg::A;

                    instruction.destination.source = arg_src::Indirect;
                    instruction.destination.type = arg_t::Unsigned8;

                    if(code.getQ())
                    {
                        std::swap(instruction.destination, instruction.source);
                    }
                    break;
                case 6:
                    instruction.source.source = arg_src::Immediate;
                    instruction.source.type = arg_t::Unsigned8;
                                    
                    setRegisterInfo(code.getY(), instruction.destination);
                    break;
            }

            if(!instruction.LD_subtype)
            {
                instruction.LD_subtype = LoadSubtype::Typical;
            }
        }

        void Decoder::decodeJR(opcode code, UnprefixedInstruction& instruction) const
        {
            if(code.getY() != 3)
            {
                instruction.condition = conditions_[code.getY() - 4];
            }
            instruction.source.source = arg_src::Immediate;
            instruction.source.type = arg_t::Signed8;
        }

        void Decoder::decodeJP(opcode code, UnprefixedInstruction& instruction) const
        {
            switch(code.getZ())
            {
                case 1:
                    instruction.source.source = arg_src::Register;
                    instruction.source.type = arg_t::Unsigned16;
                    instruction.source.reg = reg::HL;
                    break;
                case 2:
                    instruction.condition = conditions_[code.getY()];
                    [[fallthrough]];
                case 3:
                    instruction.source.source = arg_src::Immediate;
                    instruction.source.type = arg_t::Unsigned16;
                    break;
            }
        }

        void Decoder::decodeINC_DEC(opcode code, UnprefixedInstruction& instruction) const
        {
            switch(code.getZ())
            {
                case 3:
                    instruction.source.source = arg_src::Register;
                    instruction.source.type = arg_t::Unsigned16;
                    instruction.source.reg = word_registers_SP_[code.getP()];
                    break;
                case 4:
                case 5:
                    setRegisterInfo(code.getY(), instruction.source);
                    break;
            }
            instruction.destination = instruction.source;
        }

        PrefixedInstruction Decoder::decodePrefixed(opcode code) const
        {
            PrefixedInstruction result;

            switch (code.getX())
            {
                case 0: 
                {
                    result.type = bit_operations_[code.getY()];
                    break;
                }
                case 1: 
                {
                    result.type = pref_type::BIT;
                    result.bit = code.getY();
                    break;
                }
                case 2: 
                {
                    result.type = pref_type::RES;
                    result.bit = code.getY();
                    break;
                }
                case 3: 
                {
                    result.type = pref_type::SET;
                    result.bit = code.getY();
                    break;
                }
            }
            result.target = byte_registers_[code.getZ()];

            return result;
        }

        void Decoder::setRegisterInfo(uint8_t registerIndex, ArgumentInfo& registerInfo) const
        {
            registerInfo.reg = byte_registers_[registerIndex];
            registerInfo.type = arg_t::Unsigned8;

            if(registerInfo.reg != reg::HL)
            {
                registerInfo.source = arg_src::Register;
            }
            else
            {
                registerInfo.source = arg_src::Indirect;
            }
        }

        void Decoder::setALUInfo(opcode code, UnprefixedInstruction& instruction, bool hasImmediate) const
        {
            instruction.type = ALU_[code.getY()];

            instruction.destination.source = arg_src::Register;
            instruction.destination.type = arg_t::Unsigned8;
            instruction.destination.reg = reg::A;
                        
            if(hasImmediate)
            {
                instruction.source.source = arg_src::Immediate;
                instruction.source.type = arg_t::Unsigned8;
            }
            else
            {
                setRegisterInfo(code.getZ(), instruction.source);
            }
        }
    }
}

