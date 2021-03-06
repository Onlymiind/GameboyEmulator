#include "gb/cpu/Instructions.h"
#include "gb/cpu/CPU.h"
#include "utils/Utils.h"


#include <cstdint>
#include <climits>
#include <stdexcept>

namespace gb {
    namespace cpu
    {
        uint8_t SharpSM83::loadByte(decoding::ArgumentInfo destination, decoding::ArgumentInfo source)
        {
            uint8_t value = getByte(source);

            if(destination.source != decoding::ArgumentSource::IndirectImmediate)
            {
                setByteRegister(destination.reg, value);
            }
            else
            {
                uint16_t address = fetchWord();
                write(address, value);
            }

            uint8_t cycles = 1;

            if(source.source == decoding::ArgumentSource::Indirect || destination.source == decoding::ArgumentSource::Indirect)
            {
                ++cycles;
            }
            if(source.source == decoding::ArgumentSource::Immediate)
            {
                ++cycles;
            }
            if(source.source == decoding::ArgumentSource::IndirectImmediate || destination.source == decoding::ArgumentSource::IndirectImmediate)
            {
                cycles += 3;
            }

            return cycles;
        }

        uint8_t SharpSM83::NONE()
        {
            return 0;
        }

        uint8_t SharpSM83::NOP()
        {
            return 1;
        }

        uint8_t SharpSM83::LD(decoding::UnprefixedInstruction instr)
        {
            switch(*instr.LD_subtype)
            {
                case decoding::LoadSubtype::Typical:
                {
                    bool isloadWord = (instr.destination.source == decoding::ArgumentSource::Register && instr.destination.type == decoding::ArgumentType::Unsigned16);
                    if(isloadWord)
                    {
                        setWordRegister(instr.destination.reg, getWord(instr.source));
                        return instr.source.source == decoding::ArgumentSource::Immediate ? 3 : 2;
                    }
                    else
                    {
                        return loadByte(instr.destination, instr.source);
                    }
                }
                case decoding::LoadSubtype::LD_DEC:
                {
                    loadByte(instr.destination, instr.source);
                    --reg_.HL;
                    return 2;
                }
                case decoding::LoadSubtype::LD_INC:
                {
                    loadByte(instr.destination, instr.source);
                    ++reg_.HL;
                    return 2;
                }
                case decoding::LoadSubtype::LD_IO:
                {
                    //true if loading from register A, false otherwise
                    bool direction = instr.source.reg == decoding::Registers::A;
                    bool hasImmediate = (instr.source.source == decoding::ArgumentSource::Immediate) || (instr.destination.source == decoding::ArgumentSource::Immediate);
                    uint16_t address = 0xFF00 + getByte(direction ? instr.destination : instr.source);
                    if(direction)
                    {
                        write(address, reg_.A);
                    }
                    else
                    {
                        reg_.A = read(address);
                    }
                    return hasImmediate ? 3 : 2;
                }
                case decoding::LoadSubtype::LD_Offset_SP:
                {
                    int8_t offset = fetchSigned();
                    reg_.flags.H = halfCarried(static_cast<uint8_t>(reg_.SP), offset);
                    reg_.flags.C = carried(static_cast<uint8_t>(reg_.SP & 0x00FF), reinterpret_cast<uint8_t&>(offset));
                    reg_.flags.Z = 0;
                    reg_.flags.N = 0;
                    reg_.HL = reg_.SP + offset;
                    return 3;
                }
                case decoding::LoadSubtype::LD_SP:
                {
                    uint16_t address = fetchWord();

                    write(address, reg_.SP & 0x00FF);
                    ++address;
                    write(address, (reg_.SP & 0xFF00) >> 8);
                    return 5;
                }
                default:
                {
                    throw std::invalid_argument("Unknown LD instruction");
                }
            }
        }

        uint8_t SharpSM83::INC(decoding::ArgumentInfo target)
        {
            if(target.type == decoding::ArgumentType::Unsigned16)
            {
                uint16_t value = getWordRegister(target.reg);
                ++value;
                setWordRegister(target.reg, value);
                return 2;
            }
            else
            {
                uint8_t value = getByteRegister(target.reg);
                reg_.flags.H = halfCarried(value, 1);
                reg_.flags.N = 0;
                ++value;
                setByteRegister(target.reg, value);
                reg_.flags.Z = value == 0;
                return target.source == decoding::ArgumentSource::Register ? 1 : 3;
            }
        }

        uint8_t SharpSM83::DEC(decoding::ArgumentInfo target)
        {
            if(target.type == decoding::ArgumentType::Unsigned16)
            {
                uint16_t value = getWordRegister(target.reg);
                --value;
                setWordRegister(target.reg, value);
                return 2;
            }
            else
            {
                uint8_t value = getByteRegister(target.reg);
                reg_.flags.H = halfBorrowed(value, 1);
                reg_.flags.N = 1;
                --value;
                setByteRegister(target.reg, value);
                reg_.flags.Z = value == 0;
                return target.source == decoding::ArgumentSource::Register ? 1 : 3;
            }
        }

        uint8_t SharpSM83::ADD(decoding::UnprefixedInstruction instr)
        {
            reg_.flags.N = 0;

            switch(instr.destination.reg)
            {
                case decoding::Registers::HL:
                {
                    uint16_t value = getWordRegister(instr.source.reg);
                    reg_.flags.H = halfCarried(reg_.HL.value(), value);
                    reg_.flags.C = carried(reg_.HL.value(), value);
                    reg_.HL += value;
                    return 2;
                }
                case decoding::Registers::SP:
                {
                    reg_.flags.Z = 0;
                    int8_t value = fetchSigned();
                    reg_.flags.H = halfCarried(static_cast<uint8_t>(reg_.SP), value); //According to specification H flag should be set if overflow from bit 3
                    reg_.flags.C = carried(static_cast<uint8_t>(reg_.SP), static_cast<uint8_t>(value)); //Carry flag should be set if overflow from bit 7
                    reg_.SP += value;
                    return 4;
                }
                case decoding::Registers::A:
                {
                    uint8_t value = getByte(instr.source);
                    reg_.flags.H = halfCarried(reg_.A, value);
                    reg_.flags.C = carried(reg_.A, value);
                    reg_.A += value;
                    reg_.flags.Z = reg_.A == 0;

                    uint8_t cycles = 1;
                    if(instr.source.source == decoding::ArgumentSource::Immediate || instr.source.source == decoding::ArgumentSource::Indirect)
                    {
                        ++cycles;
                    }
                    return cycles;
                }
                default:
                {
                    throw std::invalid_argument("Unknown ADD instruction");
                }
            }
        }

        uint8_t SharpSM83::ADC(decoding::ArgumentInfo argument)
        {
            reg_.flags.N = 0;
            uint8_t value = getByte(argument);

            reg_.flags.H = ((reg_.A & 0x0F) + (value &0x0F) + reg_.flags.C) > 0x0F;
            reg_.flags.C = (static_cast<uint16_t>(reg_.A) + value + reg_.flags.C) > 0xFF;
            reg_.A += value + reg_.flags.C;
            reg_.flags.Z = reg_.A == 0;

            return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::SUB(decoding::ArgumentInfo argument)
        {
            reg_.flags.N = 1;
            uint8_t value = getByte(argument);

            reg_.flags.H = halfBorrowed(reg_.A, value);
            reg_.flags.C = borrowed(reg_.A, value);
            reg_.A -= value;
            reg_.flags.Z = reg_.A == 0;

            return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::SBC(decoding::ArgumentInfo argument)
        {
            reg_.flags.N = 1;
            uint8_t value = getByte(argument);

            reg_.flags.H = (reg_.A & 0x0F) < ((value & 0x0F) + reg_.flags.C);
            reg_.flags.C = reg_.A < (static_cast<uint16_t>(value) + reg_.flags.C);
            reg_.A -= value + reg_.flags.C;
            reg_.flags.Z = reg_.A == 0;

            return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::OR(decoding::ArgumentInfo argument)
        {
            reg_.flags.value = 0; //Only Z flag can be non-zero as a result of OR
            
            reg_.A |= getByte(argument);
            reg_.flags.Z = reg_.A == 0;

            return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::AND(decoding::ArgumentInfo argument)
        {
            reg_.flags.value = 0;
            reg_.flags.H = 1;
            reg_.A &= getByte(argument);
            reg_.flags.Z = reg_.A == 0;

            return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::XOR(decoding::ArgumentInfo argument)
        {
            reg_.flags.value = 0; //Only Z flag can be non-zero as a result of XOR
            reg_.A ^= getByte(argument);
            reg_.flags.Z = reg_.A == 0;

            return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::CP(decoding::ArgumentInfo argument)
        {
            uint8_t value = getByte(argument);

            reg_.flags.N = 1;
            reg_.flags.H = halfBorrowed(reg_.A, value);
            reg_.flags.C = borrowed(reg_.A, value);
            reg_.flags.Z = (reg_.A - value) == 0;

            return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::JP(decoding::UnprefixedInstruction instr)
        {
            uint16_t address = getWord(instr.source);

            if(instr.condition.has_value())
            {
                if(checkCondition(*instr.condition))
                {
                    reg_.PC = address;
                    return 4;
                }
                else
                {
                    return 3;
                }
            }
            else
            {
                reg_.PC = address;
                return instr.source.source == decoding::ArgumentSource::Register ? 1 : 4;
            }
        }

        uint8_t SharpSM83::JR(std::optional<decoding::Conditions> condition)
        {
            int8_t relAddress = fetchSigned();

            if(condition.has_value() && (!checkCondition(*condition)))
            {
                return 2;
            }

            reg_.PC += relAddress;
            return 3;
        }

        uint8_t SharpSM83::PUSH(decoding::Registers reg)
        {
            pushStack(getWordRegister(reg));
            return 4;
        }

        uint8_t SharpSM83::POP(decoding::Registers reg)
        {
            setWordRegister(reg, popStack());
            return 3;
        }

        uint8_t SharpSM83::RST(uint16_t reset_vector)
        {
            pushStack(reg_.PC);
            reg_.PC = reset_vector;
            return 4;
        }

        uint8_t SharpSM83::CALL(std::optional<decoding::Conditions> condition)
        {
            uint16_t address = fetchWord();

            if(condition.has_value() && (!checkCondition(*condition)))
            {
                return 3;
            }

            pushStack(reg_.PC);
            reg_.PC = address;
            return 6;
        }

        uint8_t SharpSM83::RET(std::optional<decoding::Conditions> condition)
        {
            bool conditional = condition.has_value();
            if(conditional && (!checkCondition(*condition)))
            {
                return 2;
            }

            reg_.PC = popStack();
            return conditional ? 5 : 4;
        }

        uint8_t SharpSM83::RETI()
        {
            reg_.PC = popStack();
            IME_ = true;
            return 4;
        }

        uint8_t SharpSM83::DI()
        {
            IME_ = false;
            enable_IME_ = false;
            return 1;
        }

        uint8_t SharpSM83::EI()
        {
            enable_IME_ = true;
            return 1;
        }

        uint8_t SharpSM83::HALT()
        {
            if(getPendingInterrupt() && !IME_)
            {
                halt_bug_ = true;
            }
            else
            {
                halt_mode_ = true;
            }
            return 1;
        }

        uint8_t SharpSM83::STOP()
        {
            //TODO: this should turn LCD off
            //TODO: check for corrupted STOP
            halt_mode_ = true;
            return 1;
        }

        uint8_t SharpSM83::DAA()
        {
            uint8_t correction = 0;

            if (reg_.flags.C || (!reg_.flags.N && reg_.A > 0x99))
            {
                correction |= 0x60;
                reg_.flags.C = 1;
            }
            if (reg_.flags.H || (!reg_.flags.N && (reg_.A & 0x0F) > 0x09))
            {
                correction |= 0x06;
            }

            if (reg_.flags.N) 
            {
                reg_.A -= correction;
            }
            else 
            {
                reg_.A += correction;
            }

            reg_.flags.H = 0;
            reg_.flags.Z = reg_.A == 0;

            return 1;
        }

        uint8_t SharpSM83::CPL()
        {
            reg_.flags.N = 1;
            reg_.flags.H = 1;
            reg_.A = ~reg_.A;
            return 1;
        }

        uint8_t SharpSM83::CCF()
        {
            reg_.flags.N = 0;
            reg_.flags.H = 0;
            reg_.flags.C = !reg_.flags.C;
            return 1;
        }

        uint8_t SharpSM83::SCF()
        {
            reg_.flags.N = 0;
            reg_.flags.H = 0;
            reg_.flags.C = 1;
            return 1;
        }

        uint8_t SharpSM83::RLA()
        {
            uint8_t firstBit = reg_.flags.C;
            reg_.flags.value = 0;
            reg_.flags.C = (reg_.A & 0x80) != 0;
            reg_.A = (reg_.A << 1) | firstBit;

            return 1;
        }

        uint8_t SharpSM83::RRA()
        {
            uint8_t lastBit = static_cast<uint8_t>(reg_.flags.C) << 7;
            reg_.flags.value = 0;
            reg_.flags.C = (reg_.A & 0x01) != 0;
            reg_.A = (reg_.A >> 1) | lastBit;
            return 1;
        }

        uint8_t SharpSM83::RLCA()
        {
            reg_.flags.value = 0;
            reg_.flags.C = (reg_.A & 0b10000000) != 0;
            reg_.A = (reg_.A << 1) | (reg_.A >> 7);

            return 1;
        }

        uint8_t SharpSM83::RRCA()
        {
            reg_.flags.value = 0;
            reg_.flags.C = (reg_.A & 0b00000001) != 0;
            reg_.A = (reg_.A >> 1) | (reg_.A << 7);

            return 1;
        }

        uint8_t SharpSM83::RLC(decoding::Registers reg)
        {
            uint8_t value = getByteRegister(reg);

            reg_.flags.value = 0;
            reg_.flags.C = (value & 0x80) != 0;
            value = (value << 1) | (value >> 7); // (value << n) | (value >> (BIT_COUNT - n))
            reg_.flags.Z = value == 0;

            setByteRegister(reg, value);

            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::RRC(decoding::Registers reg)
        {
            uint8_t value = getByteRegister(reg);

            reg_.flags.value = 0;
            reg_.flags.C = (value & 0x01) != 0;
            value = (value >> 1) | (value << 7); // (value >> n) | (value << (BIT_COUNT - n))
            reg_.flags.Z = value == 0;

            setByteRegister(reg, value);

            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::RL(decoding::Registers reg)
        {
            uint8_t value = getByteRegister(reg);

            uint8_t firstBit = reg_.flags.C;
            reg_.flags.value = 0;
            reg_.flags.C = (value & 0x80) != 0;
            value = (value << 1) | firstBit;
            reg_.flags.Z = value == 0;

            setByteRegister(reg, value);

            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::RR(decoding::Registers reg)
        {
            uint8_t value = getByteRegister(reg);

            uint8_t lastBit = static_cast<uint8_t>(reg_.flags.C) << 7;
            reg_.flags.value = 0;
            reg_.flags.C = (value & 0x01) != 0;
            value = (value >> 1) | lastBit;
            reg_.flags.Z = value == 0;

            setByteRegister(reg, value);
            
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SLA(decoding::Registers reg)
        {
            reg_.flags.value = 0;
            uint8_t value = getByteRegister(reg);

            reg_.flags.C = (value & 0x80) != 0;
            value <<= 1;
            setByteRegister(reg, value);
            reg_.flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SRA(decoding::Registers reg)
        {
            reg_.flags.value = 0;
            uint8_t firstBit = 0;
            uint8_t value = getByteRegister(reg);

            reg_.flags.C = (value & 0x01) != 0;
            firstBit = value & 0x80;
            value >>= 1;
            value |= firstBit;
            setByteRegister(reg, value);
            reg_.flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SWAP(decoding::Registers reg)
        {
            reg_.flags.value = 0;
            uint8_t value = getByteRegister(reg);
            uint8_t temp = value & 0xF0;

            temp >>= 4;
            value <<= 4;
            value |= temp;
            setByteRegister(reg, value);
            reg_.flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SRL(decoding::Registers reg)
        {
            reg_.flags.value = 0;
            uint8_t value = getByteRegister(reg);

            reg_.flags.C = (value & 0x01) != 0;
            value >>= 1;
            setByteRegister(reg, value);
            reg_.flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::BIT(decoding::PrefixedInstruction instr)
        {
            reg_.flags.N = 0;
            reg_.flags.H = 1;
            uint8_t value = getByteRegister(instr.target);

            value &= 1 << *instr.bit;
            reg_.flags.Z = value == 0;
            return instr.target == decoding::Registers::HL ? 3 : 2;
        }

        uint8_t SharpSM83::RES(decoding::PrefixedInstruction instr)
        {
            uint8_t mask = ~(1 << *instr.bit);
            setByteRegister(instr.target, getByteRegister(instr.target) & mask);
            return instr.target == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SET(decoding::PrefixedInstruction instr)
        {
            uint8_t mask = 1 << *instr.bit;
            setByteRegister(instr.target, getByteRegister(instr.target) | mask);
            return instr.target == decoding::Registers::HL ? 4 : 2;
        }

        /*
        Instruction SharpSM83::SET(decoding::PrefixedInstruction instr)
        {
            InstructionContext ctxt;
            ctxt.registers = &reg_;
            std::vector<std::function<void(InstrcutionContext&)>>

            uint8_t mask = (1 << *instr.bit);


        }
        */
    }
}
