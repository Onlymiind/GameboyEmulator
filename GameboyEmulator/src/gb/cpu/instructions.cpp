#include "gb/cpu/CPU.h"
#include "utils/Utils.h"


#include <cstdint>
#include <climits>

namespace gb {
    namespace cpu
    {
        uint8_t SharpSM83::loadByte(decoding::ArgumentInfo destination, decoding::ArgumentInfo source)
        {
            uint8_t value = getByte(source);

            if(destination.Source != decoding::ArgumentSource::IndirectImmediate)
            {
                setByteRegister(destination.Register, value);
            }
            else
            {
                uint16_t address = fetchWord();
                write(address, value);
            }

            uint8_t cycles = 1;

            if(source.Source == decoding::ArgumentSource::Indirect || destination.Source == decoding::ArgumentSource::Indirect)
            {
                ++cycles;
            }
            if(source.Source == decoding::ArgumentSource::Immediate)
            {
                ++cycles;
            }
            if(source.Source == decoding::ArgumentSource::IndirectImmediate || destination.Source == decoding::ArgumentSource::IndirectImmediate)
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
            switch(*instr.LDSubtype)
            {
                case decoding::LoadSubtype::Typical:
                {
                    bool isloadWord = (instr.Destination.Source == decoding::ArgumentSource::Register && instr.Destination.Type == decoding::ArgumentType::Unsigned16);
                    if(isloadWord)
                    {
                        setWordRegister(instr.Destination.Register, getWord(instr.Source));
                        return instr.Source.Source == decoding::ArgumentSource::Immediate ? 3 : 2;
                    }
                    else
                    {
                        return loadByte(instr.Destination, instr.Source);
                    }
                }
                case decoding::LoadSubtype::LD_DEC:
                {
                    loadByte(instr.Destination, instr.Source);
                    --REG.HL;
                    return 2;
                }
                case decoding::LoadSubtype::LD_INC:
                {
                    loadByte(instr.Destination, instr.Source);
                    ++REG.HL;
                    return 2;
                }
                case decoding::LoadSubtype::LD_IO:
                {
                    //true if loading from register A, false otherwise
                    bool direction = instr.Source.Register == decoding::Registers::A;
                    bool hasImmediate = (instr.Source.Source == decoding::ArgumentSource::Immediate) || (instr.Destination.Source == decoding::ArgumentSource::Immediate);
                    uint16_t address = 0xFF00 + getByte(direction ? instr.Destination : instr.Source);
                    if(direction)
                    {
                        write(address, REG.A);
                    }
                    else
                    {
                        REG.A = read(address);
                    }
                    return hasImmediate ? 3 : 2;
                }
                case decoding::LoadSubtype::LD_Offset_SP:
                {
                    int8_t offset = fetchSigned();
                    REG.Flags.H = halfCarryOccured8Add(REG.SP & 0x00FF, offset);
                    REG.Flags.C = carryOccured(static_cast<uint8_t>(REG.SP & 0x00FF), reinterpret_cast<uint8_t&>(offset));
                    REG.Flags.Z = 0;
                    REG.Flags.N = 0;
                    REG.HL = REG.SP + offset;
                    return 3;
                }
                case decoding::LoadSubtype::LD_SP:
                {
                    uint16_t address = fetchWord();

                    write(address, REG.SP & 0x00FF);
                    ++address;
                    write(address, (REG.SP & 0xFF00) >> 8);
                    return 5;
                }
            }
        }

        uint8_t SharpSM83::INC(decoding::ArgumentInfo target)
        {
            if(target.Type == decoding::ArgumentType::Unsigned16)
            {
                uint16_t value = getWordRegister(target.Register);
                ++value;
                setWordRegister(target.Register, value);
                return 2;
            }
            else
            {
                uint8_t value = getByteRegister(target.Register);
                REG.Flags.H = halfCarryOccured8Add(value, 1);
                REG.Flags.N = 0;
                ++value;
                setByteRegister(target.Register, value);
                REG.Flags.Z = value == 0;
                return target.Source == decoding::ArgumentSource::Register ? 1 : 3;
            }
        }

        uint8_t SharpSM83::DEC(decoding::ArgumentInfo target)
        {
            if(target.Type == decoding::ArgumentType::Unsigned16)
            {
                uint16_t value = getWordRegister(target.Register);
                --value;
                setWordRegister(target.Register, value);
                return 2;
            }
            else
            {
                uint8_t value = getByteRegister(target.Register);
                REG.Flags.H = halfCarryOccured8Sub(value, 1);
                REG.Flags.N = 1;
                --value;
                setByteRegister(target.Register, value);
                REG.Flags.Z = value == 0;
                return target.Source == decoding::ArgumentSource::Register ? 1 : 3;
            }
        }

        uint8_t SharpSM83::ADD(decoding::UnprefixedInstruction instr)
        {
            REG.Flags.N = 0;

            switch(instr.Destination.Register)
            {
                case decoding::Registers::HL:
                {
                    uint16_t value = getWordRegister(instr.Source.Register);
                    REG.Flags.H = halfCarryOccured16Add(REG.HL, value);
                    REG.Flags.C = carryOccured(REG.HL, value);
                    REG.HL += value;
                    return 2;
                }
                case decoding::Registers::SP:
                {
                    REG.Flags.Z = 0;
                    int8_t value = fetchSigned();
                    REG.Flags.H = halfCarryOccured8Add(REG.SP & 0x00FF, value); //According to specification H flag should be set if overflow from bit 3
                    REG.Flags.C = carryOccured(static_cast<uint8_t>(REG.SP & 0x00FF), static_cast<uint8_t>(value)); //Carry flag should be set if overflow from bit 7
                    REG.SP += value;
                    return 4;
                }
                case decoding::Registers::A:
                {
                    uint8_t value = getByte(instr.Source);
                    REG.Flags.H = halfCarryOccured8Add(REG.A, value);
                    REG.Flags.C = carryOccured(REG.A, value);
                    REG.A += value;
                    REG.Flags.Z = REG.A == 0;

                    uint8_t cycles = 1;
                    if(instr.Source.Source == decoding::ArgumentSource::Immediate || instr.Source.Source == decoding::ArgumentSource::Indirect)
                    {
                        ++cycles;
                    }
                    return cycles;
                }
            }
        }

        uint8_t SharpSM83::ADC(decoding::ArgumentInfo argument)
        {
            REG.Flags.N = 0;
            uint8_t value = getByte(argument);
            uint8_t regA = REG.A;

            REG.A += value + REG.Flags.C;
            REG.Flags.Z = REG.A == 0;
            REG.Flags.H = ((regA & 0x0F) + (value &0x0F) + REG.Flags.C) > 0x0F;
            REG.Flags.C = static_cast<uint16_t>(regA) + value + REG.Flags.C > 0xFF;

            return argument.Source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::SUB(decoding::ArgumentInfo argument)
        {
            REG.Flags.N = 1;
            uint8_t value = getByte(argument);

            REG.Flags.H = halfCarryOccured8Sub(REG.A, value);
            REG.Flags.C = carryOccured(REG.A, value, true);
            REG.A -= value;
            REG.Flags.Z = REG.A == 0;

            return argument.Source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::SBC(decoding::ArgumentInfo argument)
        {
            REG.Flags.N = 1;
            uint8_t value = getByte(argument);
            uint8_t regA = REG.A;

            REG.A -= value + REG.Flags.C;
            REG.Flags.Z = REG.A == 0;
            REG.Flags.H = (regA & 0x0F) < ((value & 0x0F) + REG.Flags.C);
            REG.Flags.C = regA < (static_cast<uint16_t>(value) + REG.Flags.C);

            return argument.Source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::OR(decoding::ArgumentInfo argument)
        {
            REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of OR
            
            REG.A |= getByte(argument);
            REG.Flags.Z = REG.A == 0;

            return argument.Source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::AND(decoding::ArgumentInfo argument)
        {
            REG.Flags.Value = 0;
            REG.Flags.H = 1;
            REG.A &= getByte(argument);
            REG.Flags.Z = REG.A == 0;

            return argument.Source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::XOR(decoding::ArgumentInfo argument)
        {
            REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of XOR
            REG.A ^= getByte(argument);
            REG.Flags.Z = REG.A == 0;

            return argument.Source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::CP(decoding::ArgumentInfo argument)
        {
            uint8_t value = getByte(argument);

            REG.Flags.N = 1;
            REG.Flags.H = halfCarryOccured8Sub(REG.A, value);
            REG.Flags.C = carryOccured(REG.A, value, true);
            REG.Flags.Z = (REG.A - value) == 0;

            return argument.Source == decoding::ArgumentSource::Register ? 1 : 2;
        }

        uint8_t SharpSM83::JP(decoding::UnprefixedInstruction instr)
        {
            uint16_t address = getWord(instr.Source);

            if(instr.Condition.has_value())
            {
                if(checkCondition(*instr.Condition))
                {
                    REG.PC = address;
                    return 4;
                }
                else
                {
                    return 3;
                }
            }
            else
            {
                REG.PC = address;
                return instr.Source.Source == decoding::ArgumentSource::Register ? 1 : 4;
            }
        }

        uint8_t SharpSM83::JR(std::optional<decoding::Conditions> condition)
        {
            int8_t relAddress = fetchSigned();

            if(condition.has_value() && (!checkCondition(*condition)))
            {
                return 2;
            }

            REG.PC += relAddress;
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
            pushStack(REG.PC);
            REG.PC = reset_vector;
            return 4;
        }

        uint8_t SharpSM83::CALL(std::optional<decoding::Conditions> condition)
        {
            uint16_t address = fetchWord();

            if(condition.has_value() && (!checkCondition(*condition)))
            {
                return 3;
            }

            pushStack(REG.PC);
            REG.PC = address;
            return 6;
        }

        uint8_t SharpSM83::RET(std::optional<decoding::Conditions> condition)
        {
            bool conditional = condition.has_value();
            if(conditional && (!checkCondition(*condition)))
            {
                return 2;
            }

            REG.PC = popStack();
            return conditional ? 5 : 4;
        }

        uint8_t SharpSM83::RETI()
        {
            REG.PC = popStack();
            IME = true;
            return 4;
        }

        uint8_t SharpSM83::DI()
        {
            IME = false;
            m_EnableIME = false;
            return 1;
        }

        uint8_t SharpSM83::EI()
        {
            m_EnableIME = true;
            return 1;
        }

        uint8_t SharpSM83::HALT()
        {
            return 1;
        }

        uint8_t SharpSM83::STOP()
        {
            return 1;
        }

        uint8_t SharpSM83::DAA()
        {
            uint8_t correction = 0;

            if (REG.Flags.C || (!REG.Flags.N && REG.A > 0x99))
            {
                correction |= 0x60;
                REG.Flags.C = 1;
            }
            if (REG.Flags.H || (!REG.Flags.N && (REG.A & 0x0F) > 0x09))
            {
                correction |= 0x06;
            }

            if (REG.Flags.N) REG.A -= correction;
            else REG.A += correction;

            REG.Flags.H = 0;
            REG.Flags.Z = REG.A == 0;

            return 1;
        }

        uint8_t SharpSM83::CPL()
        {
            REG.Flags.N = 1;
            REG.Flags.H = 1;
            REG.A = ~REG.A;
            return 1;
        }

        uint8_t SharpSM83::CCF()
        {
            REG.Flags.N = 0;
            REG.Flags.H = 0;
            REG.Flags.C = !REG.Flags.C;
            return 1;
        }

        uint8_t SharpSM83::SCF()
        {
            REG.Flags.N = 0;
            REG.Flags.H = 0;
            REG.Flags.C = 1;
            return 1;
        }

        uint8_t SharpSM83::RLA()
        {
            uint8_t firstBit = REG.Flags.C;
            REG.Flags.Value = 0;
            REG.Flags.C = (REG.A & 0x80) != 0;
            REG.A = (REG.A << 1) | firstBit;

            return 1;
        }

        uint8_t SharpSM83::RRA()
        {
            uint8_t lastBit = static_cast<uint8_t>(REG.Flags.C) << 7;
            REG.Flags.Value = 0;
            REG.Flags.C = (REG.A & 0x01) != 0;
            REG.A = (REG.A >> 1) | lastBit;
            return 1;
        }

        uint8_t SharpSM83::RLCA()
        {
            REG.Flags.Value = 0;
            REG.Flags.C = (REG.A & 0b10000000) != 0;
            REG.A = (REG.A << 1) | (REG.A >> (sizeof(uint8_t) * CHAR_BIT - 1));

            return 1;
        }

        uint8_t SharpSM83::RRCA()
        {
            REG.Flags.Value = 0;
            REG.Flags.C = (REG.A & 0b00000001) != 0;
            REG.A = (REG.A >> 1) | (REG.A << (sizeof(uint8_t) * CHAR_BIT - 1));

            return 1;
        }

        uint8_t SharpSM83::RLC(decoding::Registers reg)
        {
            REG.Flags.Value = 0;
            uint8_t value = getByteRegister(reg);

            REG.Flags.C = (value & 0x80) != 0;
            value = (value << 1) | (value >> (sizeof(uint8_t) * CHAR_BIT - 1)); // (value << n) | (value >> (BIT_COUNT - n))
            setByteRegister(reg, value);
            REG.Flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::RRC(decoding::Registers reg)
        {
            REG.Flags.Value = 0;
            uint8_t value = getByteRegister(reg);

            REG.Flags.C = (value & 0x01) != 0;
            value = (value >> 1) | (value << (sizeof(uint8_t) * CHAR_BIT - 1)); // (value >> n) | (value << (BIT_COUNT - n))
            setByteRegister(reg, value);
            REG.Flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::RL(decoding::Registers reg)
        {
            uint8_t firstBit = REG.Flags.C;
            REG.Flags.Value = 0;
            uint8_t value = getByteRegister(reg);

            REG.Flags.C = (value & 0x80) != 0;
            value = (value << 1) | firstBit;
            setByteRegister(reg, value);
            REG.Flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::RR(decoding::Registers reg)
        {
            uint8_t lastBit = static_cast<uint8_t>(REG.Flags.C) << 7;
            REG.Flags.Value = 0;
            uint8_t value = getByteRegister(reg);

            REG.Flags.C = (value & 0x01) != 0;
            value = (value >> 1) | lastBit;
            setByteRegister(reg, value);
            REG.Flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SLA(decoding::Registers reg)
        {
            REG.Flags.Value = 0;
            uint8_t value = getByteRegister(reg);

            REG.Flags.C = (value & 0x80) != 0;
            value <<= 1;
            setByteRegister(reg, value);
            REG.Flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SRA(decoding::Registers reg)
        {
            REG.Flags.Value = 0;
            uint8_t firstBit = 0;
            uint8_t value = getByteRegister(reg);

            REG.Flags.C = (value & 0x01) != 0;
            firstBit = value & 0x80;
            value >>= 1;
            value |= firstBit;
            setByteRegister(reg, value);
            REG.Flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SWAP(decoding::Registers reg)
        {
            REG.Flags.Value = 0;
            uint8_t value = getByteRegister(reg);
            uint8_t temp = value & 0xF0;

            temp >>= 4;
            value <<= 4;
            value |= temp;
            setByteRegister(reg, value);
            REG.Flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SRL(decoding::Registers reg)
        {
            REG.Flags.Value = 0;
            uint8_t value = getByteRegister(reg);

            REG.Flags.C = (value & 0x01) != 0;
            value >>= 1;
            setByteRegister(reg, value);
            REG.Flags.Z = value == 0;
            return reg == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::BIT(decoding::PrefixedInstruction instr)
        {
            REG.Flags.N = 0;
            REG.Flags.H = 1;
            uint8_t value = getByteRegister(instr.Target);

            value &= 1 << *instr.Bit;
            REG.Flags.Z = value == 0;
            return instr.Target == decoding::Registers::HL ? 3 : 2;
        }

        uint8_t SharpSM83::RES(decoding::PrefixedInstruction instr)
        {
            uint8_t mask = ~(1 << *instr.Bit);
            setByteRegister(instr.Target, getByteRegister(instr.Target) & mask);
            return instr.Target == decoding::Registers::HL ? 4 : 2;
        }

        uint8_t SharpSM83::SET(decoding::PrefixedInstruction instr)
        {
            uint8_t mask = 1 << *instr.Bit;
            setByteRegister(instr.Target, getByteRegister(instr.Target) | mask);
            return instr.Target == decoding::Registers::HL ? 4 : 2;
        }
    }
}
