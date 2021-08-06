#include "core/gb/cpu/CPU.h"
#include "utils/Utils.h"


#include <cstdint>
#include <climits>

namespace gb {


    uint8_t SharpSM83::NONE()
    {
        return 0;
    }

    uint8_t SharpSM83::NOP()
    {
        return 1;
    }

    uint8_t SharpSM83::LD(SharpSM83& cpu, const opcode code)
    {
        switch (code.getX()) {
        case 0: {
            switch (code.getQ()) {
            case 0: {
                switch (code.getP()) {
                case 0: { // LD [BC], A
                    cpu.write(cpu.REG.BC, cpu.REG.A);
                    return 2;
                }
                case 1: { // LD [DE], A
                    cpu.write(cpu.REG.DE, cpu.REG.A);
                    return 2;
                }
                case 2: { // LD [HLI], A
                    cpu.write(cpu.REG.HL, cpu.REG.A);
                    ++cpu.REG.HL;
                    return 2;
                }
                case 3: { // LD [HLD], A
                    cpu.write(cpu.REG.HL, cpu.REG.A);
                    --cpu.REG.HL;
                    return 2;
                }
                }
                break;
            }
            case 1: {
                switch (code.getP()) {
                case 0: { // LD A, [BC]
                    cpu.REG.A = cpu.read(cpu.REG.BC);
                    return 2;
                }
                case 1: { // LD A, [DE]
                    cpu.REG.A = cpu.read(cpu.REG.DE);
                    return 2;
                }
                case 2: { // LD A, [HLI]
                    cpu.REG.A = cpu.read(cpu.REG.HL);
                    ++cpu.REG.HL;
                    return 2;
                }
                case 3: { // LD A, [HLD]
                    cpu.REG.A = cpu.read(cpu.REG.HL);
                    --cpu.REG.HL;
                    return 2;
                }
                }
                break;
            }
            }
            break;
        }
        case 3: {
            switch (code.getZ()) {
            case 1: { // LD SP, HL

                cpu.REG.SP = cpu.REG.HL;

                return 2;
            }
            case 2: {
                uint16_t address = cpu.fetchWord();

                switch (code.getY()) {
                case 5: { // LD [a16], A
                    cpu.write(address, cpu.REG.A);
                    return 4;
                }
                case 7: { // LD A, [a16]
                    cpu.REG.A = cpu.read(address);
                    return 4;
                }
                }
                break;
            }
            }

            break;
        }
        }


        return 0;
    }

    uint8_t SharpSM83::LD_IMM(SharpSM83& cpu, const opcode code)
    {
        switch (code.getX()) {
        case 0: {
            switch (code.getZ()) {
            case 0: { //LD [a16], SP
                uint16_t address = cpu.fetchWord();

                cpu.write(address, cpu.REG.SP & 0x00FF);
                ++address;
                cpu.write(address, (cpu.REG.SP & 0xFF00) >> 8);
                return 5;
            }
            case 1: {                     // LD reg16[code.p], d16
                uint16_t value = cpu.fetchWord();

                *cpu.m_TableREGP_SP[code.getP()] = value;
                return 3;
            }
            case 6: {
                uint8_t value = cpu.fetch();

                if (code.getY() == 6)
                {
                    cpu.write(cpu.REG.HL, value); // LD [HL], d8
                    return 3;
                }
                else
                {
                    *cpu.m_TableREG8[code.getY()] = value;     // LD reg8[code.y], d8
                    return 2;
                }

                break;
            }
            }

            break;
        }
        case 3: { // LD HL, SP + r8
            int8_t offset = cpu.fetchSigned();

            cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.SP & 0x00FF, offset);
            cpu.REG.Flags.C = cpu.carryOccured(static_cast<uint8_t>(cpu.REG.SP & 0x00FF), reinterpret_cast<uint8_t&>(offset));
            cpu.REG.Flags.Z = 0;
            cpu.REG.Flags.N = 0;

            cpu.REG.HL = cpu.REG.SP + offset;
            return 3;
        }
        }

        return 0;
    }

    uint8_t SharpSM83::LD_IO(SharpSM83& cpu, const opcode code)
    {
        switch (code.getZ()) {
        case 0: {
            uint8_t address = cpu.fetch();
            if (code.getY() == 4) cpu.write(0xFF00 + address, cpu.REG.A); // LD [$FF00 + a8], A
            else cpu.REG.A = cpu.read(0xFF00 + address);             // LD A, [$FF00 + a8]
            return 3;
        }
        case 2: {
            if (code.getY() == 4) cpu.write(0xFF00 + cpu.REG.C, cpu.REG.A);   // LD [$FF00 + C], A
            else cpu.REG.A = cpu.read(0xFF00 + cpu.REG.C);               // LD A, [$FF00 + C]
            return 2;
        }
        }
        return 0;
    }

    uint8_t SharpSM83::LD_REG8(SharpSM83& cpu, const opcode code)
    {
        if (code.getY() == 6)
        {
            cpu.write(cpu.REG.HL, *cpu.m_TableREG8[code.getZ()]);      // LD [HL], reg8[code.getZ()]
            return 2;
        }
        else if (code.getZ() == 6)
        {
            *cpu.m_TableREG8[code.getY()] = cpu.read(cpu.REG.HL); // LD reg8[code.getY()], [HL]
            return 2;
        }
        else
        {
            *cpu.m_TableREG8[code.getY()] = *cpu.m_TableREG8[code.getZ()]; // LD reg8[code.getY()], reg8[code.getZ()]
            return 1;
        }

        return 0;
    }

    uint8_t SharpSM83::INC(ArgumentInfo target)
    {
        if(target.Type == ArgumentType::Unsigned16)
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
            ++value;
            setByteRegister(target.Register, value);
            REG.Flags.Z = value == 0;
            return target.Source == ArgumentSource::Register ? 1 : 3;
        }
    }

    uint8_t SharpSM83::DEC(ArgumentInfo target)
    {
        if(target.Type == ArgumentType::Unsigned16)
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
            --value;
            setByteRegister(target.Register, value);
            REG.Flags.Z = value == 0;
            return target.Source == ArgumentSource::Register ? 1 : 3;
        }
    }

    uint8_t SharpSM83::ADD(SharpSM83& cpu, const opcode code)
    {
        cpu.REG.Flags.N = 0;

        switch (code.getX()) {
        case 0: { // ADD HL, reg16[code.getP()]
            cpu.REG.Flags.H = cpu.halfCarryOccured16Add(cpu.REG.HL, *cpu.m_TableREGP_SP[code.getP()]);
            cpu.REG.Flags.C = cpu.carryOccured(cpu.REG.HL, *cpu.m_TableREGP_SP[code.getP()]);

            cpu.REG.HL += *cpu.m_TableREGP_SP[code.getP()];

            return 2;;
        }
        case 2: {
            if (code.getZ() == 6) //ADD A, [HL]
            {
                uint8_t value = cpu.read(cpu.REG.HL);
                cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.A, value);
                cpu.REG.Flags.C = cpu.carryOccured(cpu.REG.A, value);

                cpu.REG.A += value;

                cpu.REG.Flags.Z = (cpu.REG.A == 0);

                return 2;
            }
            else { //ADD A, reg8[code.getZ()]
                cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.A, *cpu.m_TableREG8[code.getZ()]);
                cpu.REG.Flags.C = cpu.carryOccured(cpu.REG.A, *cpu.m_TableREG8[code.getZ()]);

                cpu.REG.A += *cpu.m_TableREG8[code.getZ()];

                cpu.REG.Flags.Z = cpu.REG.A == 0;

                return 1;
            }
            break;
        }
        case 3: {
            if (code.getZ() == 0) //ADD SP, r8
            {
                cpu.REG.Flags.Z = 0;
                int8_t value = cpu.fetchSigned();

                cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.SP & 0x00FF, value); //According to specification H flag should be set if overflow from bit 3
                cpu.REG.Flags.C = cpu.carryOccured(static_cast<uint8_t>(cpu.REG.SP & 0x00FF), static_cast<uint8_t>(value)); //Carry flag should be set if overflow from bit 7

                cpu.REG.SP += value;

                return 4;
            }
            else //ADD A, d8
            {
                uint8_t value = cpu.fetch();
                cpu.REG.Flags.H = cpu.halfCarryOccured8Add(cpu.REG.A, value);
                cpu.REG.Flags.C = cpu.carryOccured(cpu.REG.A, value);

                cpu.REG.A += value;

                cpu.REG.Flags.Z = cpu.REG.A == 0;

                return 2;
            }
            break;
        }
        }
        return 0;
    }

    uint8_t SharpSM83::ADC(ArgumentInfo argument)
    {
        REG.Flags.N = 0;
        uint8_t value = getByte(argument);
        uint8_t regA = REG.A;

        REG.A += value + REG.Flags.C;
        REG.Flags.Z = REG.A == 0;
        REG.Flags.H = ((regA & 0x0F) + (value &0x0F) + REG.Flags.C) > 0x0F;
        REG.Flags.C = static_cast<uint16_t>(regA) + value + REG.Flags.C > 0xFF;

        return argument.Source == ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::SUB(ArgumentInfo argument)
    {
        REG.Flags.N = 1;
        uint8_t value = getByte(argument);

        REG.Flags.H = halfCarryOccured8Sub(REG.A, value);
        REG.Flags.C = carryOccured(REG.A, value, true);
        REG.A -= value;
        REG.Flags.Z = REG.A == 0;

        return argument.Source == ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::SBC(ArgumentInfo argument)
    {
        REG.Flags.N = 1;
        uint8_t value = getByte(argument);
        uint8_t regA = REG.A;

        REG.A -= value + REG.Flags.C;
        REG.Flags.Z = REG.A == 0;
        REG.Flags.H = (regA & 0x0F) < ((value & 0x0F) + REG.Flags.C);
        REG.Flags.C = regA < (static_cast<uint16_t>(value) + REG.Flags.C);

        return argument.Source == ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::OR(ArgumentInfo argument)
    {
        REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of OR
        
        REG.A |= getByte(argument);
        REG.Flags.Z = REG.A == 0;

        return argument.Source == ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::AND(ArgumentInfo argument)
    {
        REG.Flags.Value = 0;
        REG.Flags.H = 1;
        REG.A &= getByte(argument);
        REG.Flags.Z = REG.A == 0;

        return argument.Source == ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::XOR(ArgumentInfo argument)
    {
        REG.Flags.Value = 0; //Only Z flag can be non-zero as a result of XOR
        REG.A ^= getByte(argument);
        REG.Flags.Z = REG.A == 0;

        return argument.Source == ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::CP(ArgumentInfo argument)
    {
        uint8_t value = getByte(argument);

        REG.Flags.N = 1;
        REG.Flags.H = halfCarryOccured8Sub(REG.A, value);
        REG.Flags.C = carryOccured(REG.A, value, true);
        REG.Flags.Z = (REG.A - value) == 0;

        return argument.Source == ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::JP(UnprefixedInstruction instr)
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
            return instr.Source.Source == ArgumentSource::Register ? 1 : 4;
        }
    }

    uint8_t SharpSM83::JR(std::optional<Conditions> condition)
    {
        int8_t relAddress = fetchSigned();

        if(condition.has_value() && (!checkCondition(*condition)))
        {
            return 2;
        }

        REG.PC += relAddress;
        return 3;
    }

    uint8_t SharpSM83::PUSH(Registers reg)
    {
        pushStack(getWordRegister(reg));
        return 4;
    }

    uint8_t SharpSM83::POP(Registers reg)
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

    uint8_t SharpSM83::CALL(std::optional<Conditions> condition)
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

    uint8_t SharpSM83::RET(std::optional<Conditions> condition)
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

    uint8_t SharpSM83::RLC(Registers reg)
    {
        REG.Flags.Value = 0;
        uint8_t value = getByteRegister(reg);

        REG.Flags.C = (value & 0x80) != 0;
        value = (value << 1) | (value >> (sizeof(uint8_t) * CHAR_BIT - 1)); // (value << n) | (value >> (BIT_COUNT - n))
        setByteRegister(reg, value);
        REG.Flags.Z = value == 0;
        return reg == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::RRC(Registers reg)
    {
        REG.Flags.Value = 0;
        uint8_t value = getByteRegister(reg);

        REG.Flags.C = (value & 0x01) != 0;
        value = (value >> 1) | (value << (sizeof(uint8_t) * CHAR_BIT - 1)); // (value >> n) | (value << (BIT_COUNT - n))
        setByteRegister(reg, value);
        REG.Flags.Z = value == 0;
        return reg == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::RL(Registers reg)
    {
        uint8_t firstBit = REG.Flags.C;
        REG.Flags.Value = 0;
        uint8_t value = getByteRegister(reg);

        REG.Flags.C = (value & 0x80) != 0;
        value = (value << 1) | firstBit;
        setByteRegister(reg, value);
        REG.Flags.Z = value == 0;
        return reg == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::RR(Registers reg)
    {
        uint8_t lastBit = static_cast<uint8_t>(REG.Flags.C) << 7;
        REG.Flags.Value = 0;
        uint8_t value = getByteRegister(reg);

        REG.Flags.C = (value & 0x01) != 0;
        value = (value >> 1) | lastBit;
        setByteRegister(reg, value);
        REG.Flags.Z = value == 0;
        return reg == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SLA(Registers reg)
    {
        REG.Flags.Value = 0;
        uint8_t value = getByteRegister(reg);

        REG.Flags.C = (value & 0x80) != 0;
        value <<= 1;
        setByteRegister(reg, value);
        REG.Flags.Z = value == 0;
        return reg == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SRA(Registers reg)
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
        return reg == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SWAP(Registers reg)
    {
        REG.Flags.Value = 0;
        uint8_t value = getByteRegister(reg);
        uint8_t temp = value & 0xF0;

        temp >>= 4;
        value <<= 4;
        value |= temp;
        setByteRegister(reg, value);
        REG.Flags.Z = value == 0;
        return reg == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SRL(Registers reg)
    {
        REG.Flags.Value = 0;
        uint8_t value = getByteRegister(reg);

        REG.Flags.C = (value & 0x01) != 0;
        value >>= 1;
        setByteRegister(reg, value);
        REG.Flags.Z = value == 0;
        return reg == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::BIT(PrefixedInstruction instr)
    {
        REG.Flags.N = 0;
        REG.Flags.H = 1;
        uint8_t value = getByteRegister(instr.Target);

        value &= 1 << *instr.Bit;
        REG.Flags.Z = value == 0;
        return instr.Target == Registers::HL ? 3 : 2;
    }

    uint8_t SharpSM83::RES(PrefixedInstruction instr)
    {
        uint8_t mask = ~(1 << *instr.Bit);
        setByteRegister(instr.Target, getByteRegister(instr.Target) & mask);
        return instr.Target == Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SET(PrefixedInstruction instr)
    {
        uint8_t mask = 1 << *instr.Bit;
        setByteRegister(instr.Target, getByteRegister(instr.Target) | mask);
        return instr.Target == Registers::HL ? 4 : 2;
    }
}
