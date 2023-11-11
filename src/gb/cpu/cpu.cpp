#include "gb/cpu/cpu.h"
#include "gb/address_bus.h"
#include "gb/cpu/cpu_utils.h"
#include "gb/cpu/decoder.h"
#include "gb/cpu/operation.h"
#include "gb/interrupt_register.h"
#include "util/util.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>

namespace gb::cpu {

    SharpSM83::SharpSM83(AddressBus &bus, InterruptRegister &interrupt_enable, InterruptRegister &interrupt_flags)
        : bus_(bus), ie_(interrupt_enable), if_(interrupt_flags) {
        reg_.setAF(0x01B0);
        reg_.BC() = 0x0013;
        reg_.DE() = 0x00D8;
        reg_.HL() = 0x014D;

        reg_.sp = 0xFFFE;
        reg_.PC() = 0x0100;
    }

    void SharpSM83::tick() {
        if (stopped_) {
            return;
        }

        memory_op_executed_ = false;
        finished_ = false;

        if (halt_mode_ && getPendingInterrupt()) {
            // TODO: exiting HALT mode should take 4 cycles
            // if an interrupt is enabled, even if IME = false;
            halt_mode_ = false;
        }

        if (enable_IME_) {
            // Enable jumping to interrupt vectors if it is scheduled by EI
            IME_ = true;
            enable_IME_ = false;
        }
        if (!halt_mode_ && memory_op_queue_.empty()) {
            if (!current_instruction_) {
                stopped_ = true;
                throw std::runtime_error("CPU's memory operation invariant failed");
            }

            if (auto interrupt = getPendingInterrupt(); IME_ && interrupt) {
                handleInterrupt(*interrupt);
            } else {
                dispatch();
                last_instruction_ = instruction_;
                finished_ = true;
                sheduleFetchInstruction();
            }
        }
        executeMemoryOp();
    }

    std::optional<InterruptFlags> SharpSM83::getPendingInterrupt() const {
        uint8_t pending_interrupts = ie_.getFlags() & if_.getFlags();
        if (pending_interrupts != 0) {
            return static_cast<InterruptFlags>(pending_interrupts & -pending_interrupts);
        } else {
            return {};
        }
    }

    void SharpSM83::handleInterrupt(InterruptFlags interrupt) {
        IME_ = false;
        if_.clearFlag(interrupt);
        sheduleMemoryNoOp();
        sheduleMemoryNoOp();
        // last_instruction_.registers.PC() contains address of the fetched instruction
        // reg_.PC() contains address of the byte after the instruction
        shedulePushStack(instruction_.registers.PC());
        sheduleMemoryNoOp();
        reg_.PC() = getInterruptVector(interrupt);
        sheduleFetchInstruction();
    }

    void SharpSM83::decode(Opcode code) {
        instruction_ = Instruction{.registers = reg_, .ime = IME_};
        if (!prefixed_next_ && isPrefix(code)) {
            prefixed_next_ = true;
            sheduleFetchInstruction();
            return;
        }

        if (prefixed_next_) {
            current_instruction_ = decodePrefixed(code);
            instruction_.type = current_instruction_->type;
            instruction_.arg() = current_instruction_->arg().reg;
            if (current_instruction_->bit) {
                instruction_.bit() = *current_instruction_->bit;
            }
        } else {
            current_instruction_ = decodeUnprefixed(code);
            instruction_.type = current_instruction_->type;
            instruction_.load_subtype = current_instruction_->ld_subtype;
            instruction_.condition = current_instruction_->condition;
        }
        prefixed_next_ = false;
    }

    void SharpSM83::dispatch() {
        using type = InstructionType;
        switch (current_instruction_->type) {
        case type::NOP: return NOP();
        case type::RLA: return RLA();
        case type::RLCA: return RLCA();
        case type::RRA: return RRA();
        case type::RRCA: return RRCA();
        case type::DI: return DI();
        case type::RETI: return RETI();
        case type::CPL: return CPL();
        case type::CCF: return CCF();
        case type::EI: return EI();
        case type::DAA: return DAA();
        case type::SCF: return SCF();
        case type::HALT: return HALT();
        case type::STOP: return STOP();
        case type::RST: return RST(*current_instruction_->reset_vector);
        case type::PUSH: return PUSH(current_instruction_->arg().reg);
        case type::POP: return POP(current_instruction_->dst.reg);
        case type::SUB: return SUB(current_instruction_->arg());
        case type::OR: return OR(current_instruction_->arg());
        case type::AND: return AND(current_instruction_->arg());
        case type::XOR: return XOR(current_instruction_->arg());
        case type::ADC: return ADC(current_instruction_->arg());
        case type::SBC: return SBC(current_instruction_->arg());
        case type::CP: return CP(current_instruction_->arg());
        case type::JR: return JR(current_instruction_->condition);
        case type::CALL: return CALL(current_instruction_->condition);
        case type::RET: return RET(current_instruction_->condition);
        case type::JP: return JP(*current_instruction_);
        case type::INC: return INC(current_instruction_->arg());
        case type::DEC: return DEC(current_instruction_->arg());
        case type::LD: return LD(*current_instruction_);
        case type::ADD: return ADD(*current_instruction_);
        case type::RLC: return RLC(current_instruction_->arg().reg);
        case type::RRC: return RRC(current_instruction_->arg().reg);
        case type::RL: return RL(current_instruction_->arg().reg);
        case type::RR: return RR(current_instruction_->arg().reg);
        case type::SLA: return SLA(current_instruction_->arg().reg);
        case type::SRA: return SRA(current_instruction_->arg().reg);
        case type::SRL: return SRL(current_instruction_->arg().reg);
        case type::SWAP: return SWAP(current_instruction_->arg().reg);
        case type::BIT: return BIT(current_instruction_->arg().reg, *current_instruction_->bit);
        case type::RES: return RES(current_instruction_->arg().reg, *current_instruction_->bit);
        case type::SET: return SET(current_instruction_->arg().reg, *current_instruction_->bit);
        default: throw std::runtime_error("unknown instruction");
        }
    }

    void SharpSM83::reset() {
        reg_.setWordRegister(Registers::AF, 0x01b0);
        reg_.setWordRegister(Registers::BC, 0x0013);
        reg_.setWordRegister(Registers::DE, 0x00d8);
        reg_.setWordRegister(Registers::HL, 0x014d);

        reg_.sp = 0xFFFE;
        reg_.PC() = 0x0100;

        IME_ = false;
        instruction_ = Instruction{};
        stopped_ = false;
        sheduleFetchInstruction();
    }

    uint8_t SharpSM83::getByteRegister(Registers reg) {
        if (isByteRegister(reg)) {
            return reg_.getByteRegister(reg);
        }
        return data_buffer_.get();
    }

    uint16_t SharpSM83::getWordRegister(Registers reg) const {
        if (reg == Registers::SP) {
            return reg_.sp;
        }

        return reg_.getWordRegister(reg);
    }

    void SharpSM83::setByteRegister(Registers reg, uint8_t data) {
        if (isByteRegister(reg)) {
            reg_.setLow(reg, data);
        } else {
            sheduleWriteByte(reg_.getWordRegister(reg), data);
        }
    }

    void SharpSM83::setWordRegister(Registers reg, uint16_t data) {
        if (reg == Registers::SP) {
            reg_.sp = data;
        } else {
            reg_.setLow(reg, uint8_t(data));
            reg_.setHigh(reg, uint8_t(data >> 8));
        }
    }

    bool SharpSM83::checkCondition(Conditions condition) {
        switch (condition) {
        case Conditions::CARRY: return reg_.getFlag(Flags::CARRY);
        case Conditions::NOT_CARRY: return !reg_.getFlag(Flags::CARRY);
        case Conditions::ZERO: return reg_.getFlag(Flags::ZERO);
        case Conditions::NOT_ZERO: return !reg_.getFlag(Flags::ZERO);
        default:
            // Never happens
            return false;
        }
    }

    void SharpSM83::setArgData(Instruction::Argument &arg, ArgumentInfo info, uint8_t data) {
        if (info.src == ArgumentSource::REGISTER || info.src == ArgumentSource::INDIRECT) {
            arg = info.reg;
        } else {
            arg = data;
        }
    }

    void SharpSM83::pushMemoryOp(MemoryOp op) {
        memory_op_queue_.push_back(op);
        // Only one operation per CPU cycle is run, check memory_op_executed_
        // flag
        executeMemoryOp();
    }

    void SharpSM83::sheduleMemoryNoOp() { pushMemoryOp(MemoryOp{}); }

    void SharpSM83::sheduleReadByte(uint16_t address) {
        pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::READ, .data = uint8_t(Registers::NONE)});
    }

    void SharpSM83::sheduleReadWord(uint16_t address) {
        pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::READ, .data = uint8_t(Registers::NONE)});
        pushMemoryOp(MemoryOp{.address = ++address, .type = MemoryOp::Type::READ, .data = uint8_t(Registers::NONE)});
    }

    void SharpSM83::sheduleWriteByte(uint16_t address, uint8_t data) {
        pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::WRITE, .data = data});
    }

    void SharpSM83::sheduleWriteWord(uint16_t address, uint16_t data) {
        pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::WRITE, .data = uint8_t(data)});
        pushMemoryOp(MemoryOp{.address = ++address, .type = MemoryOp::Type::WRITE, .data = uint8_t(data >> 8)});
    }

    void SharpSM83::shedulePushStack(uint16_t data) {
        --reg_.sp;
        pushMemoryOp(MemoryOp{.address = uint16_t(reg_.sp), .type = MemoryOp::Type::WRITE, .data = uint8_t(data >> 8)});
        --reg_.sp;
        pushMemoryOp(MemoryOp{.address = uint16_t(reg_.sp), .type = MemoryOp::Type::WRITE, .data = uint8_t(data)});
    }

    void SharpSM83::shedulePopStack(Registers reg) {
        sheduleReadToReg(reg_.sp, reg);
        reg_.sp += 2;
    }

    void SharpSM83::sheduleReadToReg(uint16_t address, Registers reg) {
        if (isByteRegister(reg)) {
            pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::READ, .data = uint8_t(reg)});
        } else {
            pushMemoryOp(
                MemoryOp{.address = address, .type = MemoryOp::Type::READ, .data = reg & Registers::LOW_REG_MASK});
            pushMemoryOp(MemoryOp{.address = ++address,
                                  .type = MemoryOp::Type::READ,
                                  .data = uint8_t((reg & Registers::HIGH_REG_MASK) >> 4)});
        }
    }

    void SharpSM83::sheduleFetchInstruction() {
        current_instruction_ = std::nullopt;
        pushMemoryOp(MemoryOp{.type = MemoryOp::Type::FETCH_INSTRUCTION});
    }

    void SharpSM83::sheduleMemoryAcceses(DecodedInstruction instr) {
        switch (instr.src.src) {
        case ArgumentSource::IMMEDIATE_S8:
        case ArgumentSource::IMMEDIATE_U8:
            sheduleReadByte(reg_.PC());
            ++reg_.PC();
            return;
        case ArgumentSource::IMMEDIATE_U16:
            sheduleReadWord(reg_.PC());
            reg_.PC() += 2;
            return;
        case ArgumentSource::INDIRECT:
            if (instr.src.reg == Registers::C) {
                return;
            }
            sheduleReadByte(getWordRegister(instr.src.reg));
            return;
        }

        if (instr.dst.src == ArgumentSource::IMMEDIATE_U8) {
            sheduleReadByte(reg_.PC());
            ++reg_.PC();

        } else if (instr.dst.src == ArgumentSource::IMMEDIATE_U16) {

            sheduleReadWord(reg_.PC());
            reg_.PC() += 2;
        }
    }

    void SharpSM83::executeMemoryOp() {
        if (memory_op_queue_.empty() || memory_op_executed_) {
            return;
        }
        memory_op_executed_ = true;

        MemoryOp op = memory_op_queue_.pop_front();
        using enum MemoryOp::Type;
        switch (op.type) {
        case NONE: break;
        case READ:
            if (op.data != uint8_t(Registers::NONE)) {
                reg_.setLow(Registers(op.data), bus_.read(op.address));
            } else {
                data_buffer_.put(bus_.read(op.address));
            }
            break;
        case FETCH_INSTRUCTION:
            decode(bus_.read(reg_.PC()));
            if (halt_bug_) {
                halt_bug_ = false;
            } else {
                ++reg_.PC();
            }
            // false if fetched 0xCB
            if (current_instruction_) {
                sheduleMemoryAcceses(*current_instruction_);
            }
            break;

        case WRITE: bus_.write(op.address, op.data); break;
        }
    }
} // namespace gb::cpu
