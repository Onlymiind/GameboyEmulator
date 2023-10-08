#include "gb/cpu/CPU.h"
#include "gb/cpu/CPUUtils.h"
#include "gb/cpu/Operation.h"
#include "utils/Utils.h"


#include <cstdint>
#include <climits>
#include <stdexcept>

namespace gb::cpu {
    using enum Flags;

    uint8_t SharpSM83::loadByte(decoding::ArgumentInfo destination, decoding::ArgumentInfo source) {
        uint8_t value = getByte(source);
        set_arg_data(last_instruction_.src, source, value);

        if(destination.source != decoding::ArgumentSource::IndirectImmediate) {
            last_instruction_.dst = destination.reg;
            setByteRegister(destination.reg, value);
        } else {
            uint16_t address = fetchWord();
            last_instruction_.dst = address;
            bus_.write(address, value);
        }

        uint8_t cycles = 1;

        if(source.source == decoding::ArgumentSource::Indirect || destination.source == decoding::ArgumentSource::Indirect) {
            ++cycles;
        }
        if(source.source == decoding::ArgumentSource::Immediate) {
            ++cycles;
        }
        if(source.source == decoding::ArgumentSource::IndirectImmediate || destination.source == decoding::ArgumentSource::IndirectImmediate) {
            cycles += 3;
        }

        return cycles;
    }

    uint8_t SharpSM83::LD(decoding::UnprefixedInstruction instr) {
        switch(*instr.LD_subtype) {
            case decoding::LoadSubtype::Typical: {
                bool isloadWord = (instr.destination.source == decoding::ArgumentSource::Register && instr.destination.type == decoding::ArgumentType::Unsigned16);
                if(isloadWord) {
                    last_instruction_.dst = instr.destination.reg;
                    uint16_t value = getWord(instr.source);
                    if(instr.source.source == decoding::ArgumentSource::Register) {
                        last_instruction_.src = instr.source.reg;
                    } else {
                        last_instruction_.src = value;
                    }
                    setWordRegister(instr.destination.reg, value);
                    return instr.source.source == decoding::ArgumentSource::Immediate ? 3 : 2;
                } else {
                    return loadByte(instr.destination, instr.source);
                }
            }
            case decoding::LoadSubtype::LD_DEC:
                loadByte(instr.destination, instr.source);
                --reg_.HL();
                return 2;
            case decoding::LoadSubtype::LD_INC:
                loadByte(instr.destination, instr.source);
                ++reg_.HL();
                return 2;
            case decoding::LoadSubtype::LD_IO: {
                //true if loading from register A, false otherwise
                bool direction = instr.source.reg == decoding::Registers::A;
                bool hasImmediate = (instr.source.source == decoding::ArgumentSource::Immediate) || (instr.destination.source == decoding::ArgumentSource::Immediate);
                uint8_t byte = getByte(direction ? instr.destination : instr.source);
                uint16_t address = 0xFF00 + uint16_t(byte);
                if(direction) {
                    set_arg_data(last_instruction_.dst, instr.destination, byte);
                    last_instruction_.src = decoding::Registers::A;
                    bus_.write(address, reg_.A());
                } else {
                    set_arg_data(last_instruction_.src, instr.source, byte);
                    last_instruction_.dst = decoding::Registers::A;
                    reg_.A() = bus_.read(address);
                }
                return hasImmediate ? 3 : 2;
            }
            case decoding::LoadSubtype::LD_Offset_SP: {
                int8_t offset = fetchSigned();
                last_instruction_.dst = decoding::Registers::SP;
                last_instruction_.src = offset;
                reg_.setFlag(H, halfCarried(uint8_t(reg_.SP), offset));
                reg_.setFlag(C, carried(uint8_t(reg_.SP), reinterpret_cast<uint8_t&>(offset)));
                reg_.setFlag(Z, 0);
                reg_.setFlag(N, 0);
                reg_.HL() = reg_.SP + offset;
                return 3;
            }
            case decoding::LoadSubtype::LD_SP: {
                uint16_t address = fetchWord();
                last_instruction_.dst = decoding::Registers::SP;
                last_instruction_.src = address;

                bus_.write(address, uint8_t(reg_.SP));
                ++address;
                bus_.write(address, (reg_.SP & 0xFF00) >> 8);
                return 5;
            }
            default:
                throw std::invalid_argument("Unknown LD instruction");
        }
    }

    uint8_t SharpSM83::INC(decoding::ArgumentInfo target) {
        last_instruction_.arg() = target.reg;
        if(target.type == decoding::ArgumentType::Unsigned16) {
            uint16_t value = getWordRegister(target.reg);
            ++value;
            setWordRegister(target.reg, value);
            return 2;
        } else {
            uint8_t value = getByteRegister(target.reg);
            reg_.setFlag(H, halfCarried(value, 1));
            reg_.setFlag(N, 0);
            ++value;
            setByteRegister(target.reg, value);
            reg_.setFlag(Z, value == 0);
            return target.source == decoding::ArgumentSource::Register ? 1 : 3;
        }
    }

    uint8_t SharpSM83::DEC(decoding::ArgumentInfo target) {
        last_instruction_.arg() = target.reg;
        if(target.type == decoding::ArgumentType::Unsigned16) {
            uint16_t value = getWordRegister(target.reg);
            --value;
            setWordRegister(target.reg, value);
            return 2;
        } else {
            uint8_t value = getByteRegister(target.reg);
            reg_.setFlag(H, halfBorrowed(value, 1));
            reg_.setFlag(N, 1);
            --value;
            setByteRegister(target.reg, value);
            reg_.setFlag(Z, value == 0);
            return target.source == decoding::ArgumentSource::Register ? 1 : 3;
        }
    }

    uint8_t SharpSM83::ADD(decoding::UnprefixedInstruction instr) {
        reg_.setFlag(N, false);

        switch(instr.destination.reg) {
            case decoding::Registers::HL: {
                last_instruction_.dst = decoding::Registers::HL;
                last_instruction_.src = instr.source.reg;
                uint16_t value = getWordRegister(instr.source.reg);
                reg_.setFlag(H, halfCarried(reg_.HL(), value));
                reg_.setFlag(C, carried(reg_.HL(), value));
                reg_.HL() += value;
                return 2;
            }
            case decoding::Registers::SP: {
                last_instruction_.dst = decoding::Registers::SP;
                reg_.setFlag(Z, false);
                int8_t value = fetchSigned();
                last_instruction_.src = value;
                reg_.setFlag(H, halfCarried(uint8_t(reg_.SP), value)); //According to specification H flag should be set if overflow from bit 3
                reg_.setFlag(C, carried(uint8_t(reg_.SP), uint8_t(value))); //Carry flag should be set if overflow from bit 7
                reg_.SP += value;
                return 4;
            }
            case decoding::Registers::A: {
                uint8_t value = getByte(instr.source);
                set_arg_data(last_instruction_.arg(), instr.source, value);
                reg_.setFlag(H, halfCarried(reg_.A(), value));
                reg_.setFlag(C, carried(reg_.A(), value));
                reg_.A() += value;
                reg_.setFlag(Z, reg_.A() == 0);

                uint8_t cycles = 1;
                if(instr.source.source == decoding::ArgumentSource::Immediate || instr.source.source == decoding::ArgumentSource::Indirect) {
                    ++cycles;
                }
                return cycles;
            }
            default:
                throw std::invalid_argument("Unknown ADD instruction");
        }
    }

    uint8_t SharpSM83::ADC(decoding::ArgumentInfo argument) {
        reg_.setFlag(N, false);
        uint8_t value = getByte(argument);
        set_arg_data(last_instruction_.arg(), argument, value);
        uint8_t regA = reg_.A();

        reg_.A() += value + uint8_t(reg_.getFlag(C));
        reg_.setFlag(Z, reg_.A() == 0);
        reg_.setFlag(H, ((regA & 0x0F) + (value &0x0F) + reg_.getFlag(C)) > 0x0F);
        reg_.setFlag(C, uint16_t(regA) + value + reg_.getFlag(C) > 0xFF);

        return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::SUB(decoding::ArgumentInfo argument) {
        reg_.setFlag(N, true);
        uint8_t value = getByte(argument);
        set_arg_data(last_instruction_.arg(), argument, value);

        reg_.setFlag(H, halfBorrowed(reg_.A(), value));
        reg_.setFlag(C, borrowed(reg_.A(), value));
        reg_.A() -= value;
        reg_.setFlag(Z, reg_.A() == 0);

        return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::SBC(decoding::ArgumentInfo argument) {
        reg_.setFlag(N, true);
        uint8_t value = getByte(argument);
        set_arg_data(last_instruction_.arg(), argument, value);

        uint8_t regA = reg_.A();

        reg_.A() -= value + reg_.getFlag(C);
        reg_.setFlag(Z, reg_.A() == 0);
        reg_.setFlag(H, (regA & 0x0F) < ((value & 0x0F) + reg_.getFlag(C)));
        reg_.setFlag(C, regA < (static_cast<uint16_t>(value) + reg_.getFlag(C)));

        return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::OR(decoding::ArgumentInfo argument) {
        uint8_t value = getByte(argument);
        set_arg_data(last_instruction_.arg(), argument, value);

        reg_.clearFlags(); //Only Z flag can be non-zero as a result of OR
        
        reg_.A() |= value;
        reg_.setFlag(Z, reg_.A() == 0);

        return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::AND(decoding::ArgumentInfo argument) {
        uint8_t value = getByte(argument);
        set_arg_data(last_instruction_.arg(), argument, value);

        reg_.clearFlags();
        reg_.setFlag(H, true);
        reg_.A() &= value;
        reg_.setFlag(Z, reg_.A() == 0);

        return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::XOR(decoding::ArgumentInfo argument) {
        uint8_t value = getByte(argument);
        set_arg_data(last_instruction_.arg(), argument, value);

        reg_.clearFlags(); //Only Z flag can be non-zero as a result of XOR
        reg_.A() ^= value;
        reg_.setFlag(Z, reg_.A() == 0);

        return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::CP(decoding::ArgumentInfo argument) {
        uint8_t value = getByte(argument);
        set_arg_data(last_instruction_.arg(), argument, value);

        reg_.setFlag(N, true);
        reg_.setFlag(H, halfBorrowed(reg_.A(), value));
        reg_.setFlag(C, borrowed(reg_.A(), value));
        reg_.setFlag(Z, (reg_.A() - value) == 0);

        return argument.source == decoding::ArgumentSource::Register ? 1 : 2;
    }

    uint8_t SharpSM83::JP(decoding::UnprefixedInstruction instr) {
        uint16_t address = getWord(instr.source);
        last_instruction_.arg() = address;

        if(instr.condition.has_value()) {
            if(checkCondition(*instr.condition)) {
                reg_.PC = address;
                return 4;
            } else {
                return 3;
            }
        } else {
            reg_.PC = address;
            return instr.source.source == decoding::ArgumentSource::Register ? 1 : 4;
        }
    }

    uint8_t SharpSM83::JR(std::optional<decoding::Conditions> condition) {
        int8_t relAddress = fetchSigned();
        last_instruction_.arg() = relAddress;

        if(condition.has_value() && (!checkCondition(*condition))) {
            return 2;
        }

        reg_.PC += relAddress;
        return 3;
    }

    uint8_t SharpSM83::PUSH(decoding::Registers reg) {
        last_instruction_.arg() = reg;

        pushStack(getWordRegister(reg));
        return 4;
    }

    uint8_t SharpSM83::POP(decoding::Registers reg) {
        last_instruction_.arg() = reg;

        setWordRegister(reg, popStack());
        return 3;
    }

    uint8_t SharpSM83::RST(uint16_t reset_vector) {
        last_instruction_.arg() = reset_vector;

        pushStack(reg_.PC);
        reg_.PC = reset_vector;
        return 4;
    }

    uint8_t SharpSM83::CALL(std::optional<decoding::Conditions> condition) {
        uint16_t address = fetchWord();
        last_instruction_.arg() = address;

        if(condition.has_value() && (!checkCondition(*condition))) {
            return 3;
        }

        pushStack(reg_.PC);
        reg_.PC = address;
        return 6;
    }

    uint8_t SharpSM83::RET(std::optional<decoding::Conditions> condition) {
        bool conditional = condition.has_value();
        if(conditional && (!checkCondition(*condition))) {
            return 2;
        }

        reg_.PC = popStack();
        return conditional ? 5 : 4;
    }

    uint8_t SharpSM83::RETI() {
        reg_.PC = popStack();
        IME_ = true;
        return 4;
    }

    uint8_t SharpSM83::DI() {
        IME_ = false;
        enable_IME_ = false;
        return 1;
    }

    uint8_t SharpSM83::EI() {
        enable_IME_ = true;
        return 1;
    }

    uint8_t SharpSM83::HALT() {
        if(getPendingInterrupt() && !IME_) {
            halt_bug_ = true;
        } else {
            halt_mode_ = true;
        }
        return 1;
    }

    uint8_t SharpSM83::STOP() {
        //TODO: this should turn LCD off
        //TODO: check for corrupted STOP
        halt_mode_ = true;
        return 1;
    }

    uint8_t SharpSM83::DAA() {
        uint8_t correction = 0;

        if (reg_.getFlag(C) || (!reg_.getFlag(N) && reg_.A() > 0x99)) {
            correction |= 0x60;
            reg_.setFlag(C, true);
        }
        if (reg_.getFlag(H) || (!reg_.getFlag(N) && (reg_.A() & 0x0F) > 0x09)) {
            correction |= 0x06;
        }

        if (reg_.getFlag(N)) {
            reg_.A() -= correction;
        } else {
            reg_.A() += correction;
        }

        reg_.setFlag(H, false);
        reg_.setFlag(Z, reg_.A() == 0);

        return 1;
    }

    uint8_t SharpSM83::CPL() {
        reg_.setFlag(N, true);
        reg_.setFlag(H, true);
        reg_.A() = ~reg_.A();
        return 1;
    }

    uint8_t SharpSM83::CCF() {
        reg_.setFlag(N, false);
        reg_.setFlag(H, false);
        reg_.setFlag(C, !reg_.getFlag(C));
        return 1;
    }

    uint8_t SharpSM83::SCF() {
        reg_.setFlag(N, false);
        reg_.setFlag(H, false);
        reg_.setFlag(C, true);
        return 1;
    }

    uint8_t SharpSM83::RLA() {
        uint8_t firstBit = uint8_t(reg_.getFlag(C));
        reg_.clearFlags();
        reg_.setFlag(C, (reg_.A() & 0x80) != 0);
        reg_.A() = (reg_.A() << 1) | firstBit;

        return 1;
    }

    uint8_t SharpSM83::RRA() {
        uint8_t lastBit = uint8_t(reg_.getFlag(C)) << 7;
        reg_.clearFlags();
        reg_.setFlag(C, (reg_.A() & 0x01) != 0);
        reg_.A() = (reg_.A() >> 1) | lastBit;
        return 1;
    }

    uint8_t SharpSM83::RLCA() {
        reg_.clearFlags();
        reg_.setFlag(C, (reg_.A() & 0b10000000) != 0);
        reg_.A() = (reg_.A() << 1) | (reg_.A() >> (sizeof(uint8_t) * CHAR_BIT - 1));

        return 1;
    }

    uint8_t SharpSM83::RRCA() {
        reg_.clearFlags();
        reg_.setFlag(C, (reg_.A() & 0b00000001) != 0);
        reg_.A() = (reg_.A() >> 1) | (reg_.A() << (sizeof(uint8_t) * CHAR_BIT - 1));

        return 1;
    }

    uint8_t SharpSM83::RLC(decoding::Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x80) != 0);
        value = (value << 1) | (value >> (sizeof(uint8_t) * CHAR_BIT - 1)); // (value << n) | (value >> (BIT_COUNT - n))
        setByteRegister(reg, value);
        reg_.setFlag(Z, value == 0);
        return reg == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::RRC(decoding::Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x01) != 0);
        value = (value >> 1) | (value << (sizeof(uint8_t) * CHAR_BIT - 1)); // (value >> n) | (value << (BIT_COUNT - n))
        setByteRegister(reg, value);
        reg_.setFlag(Z, value == 0);
        return reg == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::RL(decoding::Registers reg) {
        uint8_t firstBit = uint8_t(reg_.getFlag(C));
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x80) != 0);
        value = (value << 1) | firstBit;
        setByteRegister(reg, value);
        reg_.setFlag(Z, value == 0);
        return reg == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::RR(decoding::Registers reg) {
        uint8_t lastBit = uint8_t(reg_.getFlag(C)) << 7;
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x01) != 0);
        value = (value >> 1) | lastBit;
        setByteRegister(reg, value);
        reg_.setFlag(Z, value == 0);
        return reg == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SLA(decoding::Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x80) != 0);
        value <<= 1;
        setByteRegister(reg, value);
        reg_.setFlag(Z, value == 0);
        return reg == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SRA(decoding::Registers reg) {
        reg_.clearFlags();
        uint8_t firstBit = 0;
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x01) != 0);
        firstBit = value & 0x80;
        value >>= 1;
        value |= firstBit;
        setByteRegister(reg, value);
        reg_.setFlag(Z, value == 0);
        return reg == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SWAP(decoding::Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);
        uint8_t temp = value & 0xF0;

        temp >>= 4;
        value <<= 4;
        value |= temp;
        setByteRegister(reg, value);
        reg_.setFlag(Z, value == 0);
        return reg == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SRL(decoding::Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x01) != 0);
        value >>= 1;
        setByteRegister(reg, value);
        reg_.setFlag(Z, value == 0);
        return reg == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::BIT(decoding::PrefixedInstruction instr) {
        reg_.setFlag(N, false);
        reg_.setFlag(H, true);
        uint8_t value = getByteRegister(instr.target);

        value &= 1 << *instr.bit;
        reg_.setFlag(Z, value == 0);
        return instr.target == decoding::Registers::HL ? 3 : 2;
    }

    uint8_t SharpSM83::RES(decoding::PrefixedInstruction instr) {
        uint8_t mask = ~(1 << *instr.bit);
        setByteRegister(instr.target, getByteRegister(instr.target) & mask);
        return instr.target == decoding::Registers::HL ? 4 : 2;
    }

    uint8_t SharpSM83::SET(decoding::PrefixedInstruction instr) {
        uint8_t mask = 1 << *instr.bit;
        setByteRegister(instr.target, getByteRegister(instr.target) | mask);
        return instr.target == decoding::Registers::HL ? 4 : 2;
    }
}
