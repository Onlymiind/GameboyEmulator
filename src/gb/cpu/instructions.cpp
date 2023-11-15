#include "gb/cpu/cpu.h"
#include "gb/cpu/cpu_utils.h"
#include "gb/cpu/operation.h"
#include "util/util.h"

#include <climits>
#include <cstdint>
#include <stdexcept>

namespace gb::cpu {
    using enum Flags;

    void SharpSM83::loadByte(ArgumentInfo dst, ArgumentInfo src) {
        switch (src.src) {
        case ArgumentSource::IMMEDIATE_U16: // LD A, [nn]
            sheduleReadToReg(data_buffer_.getWord(), dst.reg);
            instruction_.src = data_buffer_.getWord();
            instruction_.dst = dst.reg;
            break;
        case ArgumentSource::INDIRECT: // LD r, [HL], LD A, [rr]
            setByteRegister(dst.reg, data_buffer_.get());
            instruction_.src = src.reg;
            instruction_.dst = dst.reg;
            break;
        case ArgumentSource::IMMEDIATE_U8: // LD r, n, LD [HL], n
            setByteRegister(dst.reg, data_buffer_.get());
            instruction_.src = data_buffer_.get();
            instruction_.dst = dst.reg;
            break;
        case ArgumentSource::REGISTER:
            if (dst.src == ArgumentSource::IMMEDIATE_U16) { // LD [nn], A
                sheduleWriteByte(data_buffer_.getWord(), getByteRegister(src.reg));
                instruction_.dst = data_buffer_.getWord();
                instruction_.src = src.reg;
                break;
            } else { // LD r, r, LD [HL], r, LD [rr], A
                setByteRegister(dst.reg, getByteRegister(src.reg));
                instruction_.src = src.reg;
                instruction_.dst = dst.reg;
                break;
            }
        default: throw std::runtime_error("unreachable"); break;
        }
    }

    void SharpSM83::LD(DecodedInstruction instr) {
        switch (*instr.ld_subtype) {
        case LoadSubtype::TYPICAL:
            if (instr.dst.src == ArgumentSource::DOUBLE_REGISTER) {
                instruction_.dst = instr.dst.reg;
                uint16_t value = 0;
                if (instr.src.src == ArgumentSource::IMMEDIATE_U16) {
                    value = data_buffer_.getWord();
                    instruction_.src = value;
                } else { // LD SP, HL
                    value = reg_.HL();
                    instruction_.src = Registers::HL;
                    sheduleMemoryNoOp();
                }
                setWordRegister(instr.dst.reg, value);
            } else {
                loadByte(instr.dst, instr.src);
            }
            break;
        case LoadSubtype::LD_DEC:
            loadByte(instr.dst, instr.src);
            reg_.HL(reg_.HL() - 1);
            break;
        case LoadSubtype::LD_INC:
            loadByte(instr.dst, instr.src);
            reg_.HL(reg_.HL() + 1);
            break;
        case LoadSubtype::LD_IO: {
            // true if loading from register A, false otherwise
            bool direction = instr.src.reg == Registers::A;
            uint8_t byte = 0;
            if (instr.src.src == ArgumentSource::IMMEDIATE_U8 || instr.dst.src == ArgumentSource::IMMEDIATE_U8) {
                // LDH [n], A, LDH A, [n]
                byte = data_buffer_.get();
            } else { // LDH [C], A, LDH A, [C]
                byte = reg_.C();
            }
            uint16_t address = 0xFF00 + uint16_t(byte);
            if (direction) {
                setArgData(instruction_.dst, instr.dst, byte);
                instruction_.src = Registers::A;
                sheduleWriteByte(address, reg_.A());
            } else {
                setArgData(instruction_.src, instr.src, byte);
                instruction_.dst = Registers::A;
                sheduleReadToReg(address, Registers::A);
            }
            break;
        }
        case LoadSubtype::LD_OFFSET_SP: {
            int8_t offset = data_buffer_.getSigned();
            instruction_.dst = Registers::SP;
            instruction_.src = offset;
            reg_.setFlag(H, halfCarried(uint8_t(reg_.sp), offset));
            reg_.setFlag(C, carried(uint8_t(reg_.sp), uint8_t(offset)));
            reg_.setFlag(Z, 0);
            reg_.setFlag(N, 0);
            reg_.HL(reg_.sp + offset);
            sheduleMemoryNoOp();
            break;
        }
        case LoadSubtype::LD_SP: {
            uint16_t address = data_buffer_.getWord();
            instruction_.dst = Registers::SP;
            instruction_.src = address;
            sheduleWriteWord(address, reg_.sp);
            break;
        }
        default: throw std::invalid_argument("Unknown LD instruction"); break;
        }
    }

    void SharpSM83::INC(ArgumentInfo target) {
        instruction_.arg() = target.reg;
        if (target.src == ArgumentSource::DOUBLE_REGISTER) {
            uint16_t value = getWordRegister(target.reg);
            ++value;
            setWordRegister(target.reg, value);
            sheduleMemoryNoOp();
        } else {
            uint8_t value = getByteRegister(target.reg);
            reg_.setFlag(H, halfCarried(value, 1));
            reg_.setFlag(N, 0);
            ++value;
            reg_.setFlag(Z, value == 0);
            setByteRegister(target.reg, value);
        }
    }

    void SharpSM83::DEC(ArgumentInfo target) {
        instruction_.arg() = target.reg;
        if (target.src == ArgumentSource::DOUBLE_REGISTER) {
            uint16_t value = getWordRegister(target.reg);
            --value;
            setWordRegister(target.reg, value);
            sheduleMemoryNoOp();
        } else {
            uint8_t value = getByteRegister(target.reg);
            reg_.setFlag(H, halfBorrowed(value, 1));
            reg_.setFlag(N, 1);
            --value;
            reg_.setFlag(Z, value == 0);
            setByteRegister(target.reg, value);
        }
    }

    void SharpSM83::ADD(DecodedInstruction instr) {
        reg_.setFlag(N, false);

        switch (instr.dst.reg) {
        case Registers::HL: {
            instruction_.dst = Registers::HL;
            instruction_.src = instr.src.reg;
            uint16_t value = getWordRegister(instr.src.reg);
            reg_.setFlag(H, halfCarried(reg_.HL(), value));
            reg_.setFlag(C, carried(reg_.HL(), value));
            reg_.HL(reg_.HL() + value);
            sheduleMemoryNoOp();
            break;
        }
        case Registers::SP: {
            instruction_.dst = Registers::SP;
            reg_.setFlag(Z, false);
            int8_t value = data_buffer_.getSigned();
            instruction_.src = value;
            reg_.setFlag(H, halfCarried(uint8_t(reg_.sp),
                                        value)); // According to specification H flag
                                                 // should be set if overflow from bit 3
            reg_.setFlag(C, carried(uint8_t(reg_.sp),
                                    uint8_t(value))); // Carry flag should be set
                                                      // if overflow from bit 7
            reg_.sp += value;
            sheduleMemoryNoOp();
            sheduleMemoryNoOp();
            break;
        }
        case Registers::A: {
            uint8_t value = 0;
            if (instr.src.src == ArgumentSource::IMMEDIATE_U8) { // ADD n
                value = data_buffer_.get();
            } else { // ADD r, ADD [HL]
                value = getByteRegister(instr.src.reg);
            }
            setArgData(instruction_.arg(), instr.src, value);
            reg_.setFlag(H, halfCarried(reg_.A(), value));
            reg_.setFlag(C, carried(reg_.A(), value));
            reg_.A() += value;
            reg_.setFlag(Z, reg_.A() == 0);
            break;
        }
        default: throw std::invalid_argument("Unknown ADD instruction"); break;
        }
    }

    void SharpSM83::ADC(ArgumentInfo argument) {
        reg_.setFlag(N, false);
        uint8_t value = 0;
        if (argument.src == ArgumentSource::IMMEDIATE_U8) { // ADC n
            value = data_buffer_.get();
        } else { // ADC r, ADC [HL]
            value = getByteRegister(argument.reg);
        }
        setArgData(instruction_.arg(), argument, value);
        uint8_t old_a = reg_.A();

        reg_.A() += value + uint8_t(reg_.getFlag(C));
        reg_.setFlag(Z, reg_.A() == 0);
        reg_.setFlag(H, ((old_a & 0x0F) + (value & 0x0F) + reg_.getFlag(C)) > 0x0F);
        reg_.setFlag(C, uint16_t(old_a) + value + reg_.getFlag(C) > 0xFF);
    }

    void SharpSM83::SUB(ArgumentInfo argument) {
        reg_.setFlag(N, true);
        uint8_t value = 0;
        if (argument.src == ArgumentSource::IMMEDIATE_U8) { // SUB n
            value = data_buffer_.get();
        } else { // SUB r, SUB [HL]
            value = getByteRegister(argument.reg);
        }
        setArgData(instruction_.arg(), argument, value);

        reg_.setFlag(H, halfBorrowed(reg_.A(), value));
        reg_.setFlag(C, borrowed(reg_.A(), value));
        reg_.A() -= value;
        reg_.setFlag(Z, reg_.A() == 0);
    }

    void SharpSM83::SBC(ArgumentInfo argument) {
        reg_.setFlag(N, true);
        uint8_t value = 0;
        if (argument.src == ArgumentSource::IMMEDIATE_U8) { // SBC n
            value = data_buffer_.get();
        } else { // SBC r, SBC [HL]
            value = getByteRegister(argument.reg);
        }
        setArgData(instruction_.arg(), argument, value);

        uint8_t old_a = reg_.A();

        reg_.A() -= value + reg_.getFlag(C);
        reg_.setFlag(Z, reg_.A() == 0);
        reg_.setFlag(H, (old_a & 0x0F) < ((value & 0x0F) + reg_.getFlag(C)));
        reg_.setFlag(C, old_a < (static_cast<uint16_t>(value) + reg_.getFlag(C)));
    }

    void SharpSM83::OR(ArgumentInfo argument) {
        uint8_t value = 0;
        if (argument.src == ArgumentSource::IMMEDIATE_U8) { // OR n
            value = data_buffer_.get();
        } else { // OR r, OR [HL]
            value = getByteRegister(argument.reg);
        }
        setArgData(instruction_.arg(), argument, value);

        reg_.clearFlags(); // Only Z flag can be non-zero as a result of OR

        reg_.A() |= value;
        reg_.setFlag(Z, reg_.A() == 0);
    }

    void SharpSM83::AND(ArgumentInfo argument) {
        uint8_t value = 0;
        if (argument.src == ArgumentSource::IMMEDIATE_U8) { // AND n
            value = data_buffer_.get();
        } else { // AND r, AND [HL]
            value = getByteRegister(argument.reg);
        }
        setArgData(instruction_.arg(), argument, value);

        reg_.clearFlags();
        reg_.setFlag(H, true);
        reg_.A() &= value;
        reg_.setFlag(Z, reg_.A() == 0);
    }

    void SharpSM83::XOR(ArgumentInfo argument) {
        uint8_t value = 0;
        if (argument.src == ArgumentSource::IMMEDIATE_U8) { // XOR n
            value = data_buffer_.get();
        } else { // XOR r, XOR [HL]
            value = getByteRegister(argument.reg);
        }
        setArgData(instruction_.arg(), argument, value);

        reg_.clearFlags(); // Only Z flag can be non-zero as a result of XOR
        reg_.A() ^= value;
        reg_.setFlag(Z, reg_.A() == 0);
    }

    void SharpSM83::CP(ArgumentInfo argument) {
        uint8_t value = 0;
        if (argument.src == ArgumentSource::IMMEDIATE_U8) { // CP n
            value = data_buffer_.get();
        } else { // CP r, CP [HL]
            value = getByteRegister(argument.reg);
        }
        setArgData(instruction_.arg(), argument, value);

        reg_.setFlag(N, true);
        reg_.setFlag(H, halfBorrowed(reg_.A(), value));
        reg_.setFlag(C, borrowed(reg_.A(), value));
        reg_.setFlag(Z, (reg_.A() - value) == 0);
    }

    void SharpSM83::JP(DecodedInstruction instr) {
        if (instr.src.src == ArgumentSource::DOUBLE_REGISTER) { // JP HL
            instruction_.arg() = Registers::HL;
            reg_.PC(reg_.HL());
            return;
        }
        uint16_t address = data_buffer_.getWord();
        instruction_.arg() = address;

        if (instr.condition.has_value() && !checkCondition(*instr.condition)) {
            return;
        }
        reg_.PC(address);
        sheduleMemoryNoOp();
    }

    void SharpSM83::JR(std::optional<Conditions> condition) {
        int8_t rel_address = data_buffer_.getSigned();
        instruction_.arg() = rel_address;

        if (condition.has_value() && (!checkCondition(*condition))) {
            return;
        }

        reg_.PC(reg_.PC() + rel_address);
        sheduleMemoryNoOp();
    }

    void SharpSM83::PUSH(Registers reg) {
        instruction_.arg() = reg;
        sheduleMemoryNoOp();
        shedulePushStack(getWordRegister(reg));
    }

    void SharpSM83::POP(Registers reg) {
        instruction_.arg() = reg;
        // TODO: verify that reg is double register
        shedulePopStack(reg);
    }

    void SharpSM83::RST(uint16_t reset_vector) {
        instruction_.arg() = reset_vector;
        sheduleMemoryNoOp();
        shedulePushStack(reg_.PC());
        reg_.PC(reset_vector);
    }

    void SharpSM83::CALL(std::optional<Conditions> condition) {
        uint16_t address = data_buffer_.getWord();
        instruction_.arg() = address;

        if (condition.has_value() && (!checkCondition(*condition))) {
            return;
        }

        sheduleMemoryNoOp();
        shedulePushStack(reg_.PC());
        reg_.PC(address);
    }

    void SharpSM83::RET(std::optional<Conditions> condition) {
        bool conditional = condition.has_value();
        if (conditional) {
            sheduleMemoryNoOp();
            if (!checkCondition(*condition)) {
                return;
            }
        }

        shedulePopStack(Registers::PC);
        sheduleMemoryNoOp();
    }

    void SharpSM83::RETI() {
        shedulePopStack(Registers::PC);
        sheduleMemoryNoOp();
        IME_ = true;
    }

    void SharpSM83::DI() {
        IME_ = false;
        enable_IME_ = false;
    }

    void SharpSM83::EI() { enable_IME_ = true; }

    void SharpSM83::HALT() {
        if (getPendingInterrupt() && !IME_) {
            halt_bug_ = true;
        } else {
            halt_mode_ = true;
        }
    }

    void SharpSM83::STOP() {
        // TODO: check for corrupted STOP
        stopped_ = true;
    }

    void SharpSM83::DAA() {
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
    }

    void SharpSM83::CPL() {
        reg_.setFlag(N, true);
        reg_.setFlag(H, true);
        reg_.A() = ~reg_.A();
    }

    void SharpSM83::CCF() {
        reg_.setFlag(N, false);
        reg_.setFlag(H, false);
        reg_.setFlag(C, !reg_.getFlag(C));
    }

    void SharpSM83::SCF() {
        reg_.setFlag(N, false);
        reg_.setFlag(H, false);
        reg_.setFlag(C, true);
    }

    void SharpSM83::RLA() {
        uint8_t first_bit = uint8_t(reg_.getFlag(C));
        reg_.clearFlags();
        reg_.setFlag(C, (reg_.A() & 0x80) != 0);
        reg_.A() = (reg_.A() << 1) | first_bit;
    }

    void SharpSM83::RRA() {
        uint8_t last_bit = uint8_t(reg_.getFlag(C)) << 7;
        reg_.clearFlags();
        reg_.setFlag(C, (reg_.A() & 0x01) != 0);
        reg_.A() = (reg_.A() >> 1) | last_bit;
    }

    void SharpSM83::RLCA() {
        reg_.clearFlags();
        reg_.setFlag(C, (reg_.A() & 0b10000000) != 0);
        reg_.A() = (reg_.A() << 1) | (reg_.A() >> (sizeof(uint8_t) * CHAR_BIT - 1));
    }

    void SharpSM83::RRCA() {
        reg_.clearFlags();
        reg_.setFlag(C, (reg_.A() & 0b00000001) != 0);
        reg_.A() = (reg_.A() >> 1) | (reg_.A() << (sizeof(uint8_t) * CHAR_BIT - 1));
    }

    void SharpSM83::RLC(Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x80) != 0);
        value = (value << 1) | (value >> (sizeof(uint8_t) * CHAR_BIT - 1)); // (value << n) | (value >> (BIT_COUNT - n))
        reg_.setFlag(Z, value == 0);
        setByteRegister(reg, value);
    }

    void SharpSM83::RRC(Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x01) != 0);
        value = (value >> 1) | (value << (sizeof(uint8_t) * CHAR_BIT - 1)); // (value >> n) | (value << (BIT_COUNT - n))
        reg_.setFlag(Z, value == 0);
        setByteRegister(reg, value);
    }

    void SharpSM83::RL(Registers reg) {
        uint8_t first_bit = uint8_t(reg_.getFlag(C));
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x80) != 0);
        value = (value << 1) | first_bit;
        reg_.setFlag(Z, value == 0);
        setByteRegister(reg, value);
    }

    void SharpSM83::RR(Registers reg) {
        uint8_t last_bit = uint8_t(reg_.getFlag(C)) << 7;
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x01) != 0);
        value = (value >> 1) | last_bit;
        reg_.setFlag(Z, value == 0);
        setByteRegister(reg, value);
    }

    void SharpSM83::SLA(Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x80) != 0);
        value <<= 1;
        reg_.setFlag(Z, value == 0);
        setByteRegister(reg, value);
    }

    void SharpSM83::SRA(Registers reg) {
        reg_.clearFlags();
        uint8_t first_bit = 0;
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x01) != 0);
        first_bit = value & 0x80;
        value >>= 1;
        value |= first_bit;
        reg_.setFlag(Z, value == 0);
        setByteRegister(reg, value);
    }

    void SharpSM83::SWAP(Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);
        uint8_t temp = value & 0xF0;

        temp >>= 4;
        value <<= 4;
        value |= temp;
        reg_.setFlag(Z, value == 0);
        setByteRegister(reg, value);
    }

    void SharpSM83::SRL(Registers reg) {
        reg_.clearFlags();
        uint8_t value = getByteRegister(reg);

        reg_.setFlag(C, (value & 0x01) != 0);
        value >>= 1;
        reg_.setFlag(Z, value == 0);
        setByteRegister(reg, value);
    }

    void SharpSM83::BIT(Registers reg, uint8_t bit) {
        reg_.setFlag(N, false);
        reg_.setFlag(H, true);
        uint8_t value = getByteRegister(reg);

        value &= 1 << bit;
        reg_.setFlag(Z, value == 0);
    }

    void SharpSM83::RES(Registers reg, uint8_t bit) {
        uint8_t mask = ~(1 << bit);
        setByteRegister(reg, getByteRegister(reg) & mask);
    }

    void SharpSM83::SET(Registers reg, uint8_t bit) {
        uint8_t mask = 1 << bit;
        setByteRegister(reg, getByteRegister(reg) | mask);
    }
} // namespace gb::cpu
