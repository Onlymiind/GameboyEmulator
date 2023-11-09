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
#include <stdexcept>

namespace gb::cpu {

    SharpSM83::SharpSM83(AddressBus &bus, InterruptRegister &interrupt_enable, InterruptRegister &interrupt_flags)
        : bus_(bus), ie_(interrupt_enable), if_(interrupt_flags) {
        reg_.setAF(0x01B0);
        reg_.BC() = 0x0013;
        reg_.DE() = 0x00D8;
        reg_.HL() = 0x014D;

        reg_.sp = 0xFFFE;
        reg_.pc = 0x0100;
    }

    void SharpSM83::tick() {
        if (stopped_) {
            return;
        }

        memory_op_executed_ = false;

        if (halt_mode_ && if_.getFlags()) {
            // TODO: exiting HALT mode should take 4 cycles
            // if an interrupt is enabled, even if IME = false;
            halt_mode_ = false;
        }

        if (enable_IME_) {
            // Enable jumping to interrupt vectors if it is scheduled by EI
            IME_ = true;
            enable_IME_ = false;
        }
        if (wait_for_pc_read_ && memory_op_queue_.empty()) {
            wait_for_pc_read_ = false;
            sheduleFetchInstruction();
        } else if (!halt_mode_ && memory_op_queue_.empty()) {
            if (!current_instruction_) {
                last_instruction_ = Instruction{.registers = reg_};
                last_instruction_.registers.pc = last_pc_;
                std::optional<InterruptFlags> interrupt = getPendingInterrupt();
                if (interrupt && IME_) {
                    handleInterrupt(*interrupt);
                } else {
                    decode(data_buffer_.get());
                    if (current_instruction_) {
                        sheduleMemoryAcceses(*current_instruction_);
                    }
                }
            }
            if (current_instruction_ && memory_op_queue_.empty()) {
                cycles_to_finish_ = dispatch();
                if (!wait_for_pc_read_) {
                    sheduleFetchInstruction();
                }
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
        ie_.clearFlag(interrupt);
        if_.clearFlag(interrupt);
        sheduleMemoryNoOp();
        sheduleMemoryNoOp();
        // last_pc_ contains address of the fetched instruction
        // reg_.PC contains address of the byte after the instruction
        shedulePushStack(last_pc_);
        sheduleMemoryNoOp();
        reg_.pc = g_interrupt_vectors.at(interrupt);
        sheduleFetchInstruction();
        cycles_to_finish_ = 5;
    }

    void SharpSM83::decode(Opcode code) {
        if (!prefixed_next_ && isPrefix(code)) {
            prefixed_next_ = true;
            sheduleFetchInstruction();
            return;
        }

        if (prefixed_next_) {
            current_instruction_ = decodePrefixed(code);
            last_instruction_.type = current_instruction_->type;
            last_instruction_.arg() = current_instruction_->arg().reg;
            if (current_instruction_->bit) {
                last_instruction_.bit() = *current_instruction_->bit;
            }
        } else {
            current_instruction_ = decodeUnprefixed(code);
            last_instruction_.type = current_instruction_->type;
            last_instruction_.load_subtype = current_instruction_->ld_subtype;
            last_instruction_.condition = current_instruction_->condition;
        }
        prefixed_next_ = false;
    }

    uint8_t SharpSM83::dispatch() {
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
        reg_.setAF(0x01B0);
        reg_.BC() = 0x0013;
        reg_.DE() = 0x00D8;
        reg_.HL() = 0x014D;

        reg_.sp = 0xFFFE;
        reg_.pc = 0x0100;

        IME_ = false;
        last_instruction_ = Instruction{};
        stopped_ = false;
        sheduleFetchInstruction();
    }

    uint8_t SharpSM83::getByteRegister(Registers reg) {
        switch (reg) {
        case Registers::A: return reg_.A();
        case Registers::B: return reg_.B();
        case Registers::C: return reg_.C();
        case Registers::D: return reg_.D();
        case Registers::E: return reg_.E();
        case Registers::H: return reg_.H();
        case Registers::L: return reg_.L();
        case Registers::HL:
        case Registers::BC:
        case Registers::DE: return data_buffer_.get();
        default: throw std::invalid_argument("Trying to get byte from unknown register"); return 0;
        }
    }

    uint16_t SharpSM83::getWordRegister(Registers reg) const {
        switch (reg) {
        case Registers::BC: return reg_.BC();
        case Registers::DE: return reg_.DE();
        case Registers::HL: return reg_.HL();
        case Registers::SP: return reg_.sp;
        case Registers::AF: return reg_.AF();
        default: throw std::invalid_argument("Trying to get byte from unknown register"); return 0;
        }
    }

    void SharpSM83::setByteRegister(Registers reg, uint8_t data) {
        switch (reg) {
        case Registers::A: reg_.A() = data; return;
        case Registers::B: reg_.B() = data; return;
        case Registers::C: reg_.C() = data; return;
        case Registers::D: reg_.D() = data; return;
        case Registers::E: reg_.E() = data; return;
        case Registers::H: reg_.H() = data; return;
        case Registers::L: reg_.L() = data; return;
        case Registers::HL: sheduleWriteByte(reg_.HL(), data); return;
        case Registers::BC: sheduleWriteByte(reg_.BC(), data); return;
        case Registers::DE: sheduleWriteByte(reg_.DE(), data); return;
        default: throw std::invalid_argument("Trying to write byte to unknown register");
        }
    }

    void SharpSM83::setWordRegister(Registers reg, uint16_t data) {
        switch (reg) {
        case Registers::BC: reg_.BC() = data; return;
        case Registers::DE: reg_.DE() = data; return;
        case Registers::HL: reg_.HL() = data; return;
        case Registers::SP: reg_.sp = data; return;
        case Registers::AF: reg_.setAF(data); return;
        default: throw std::invalid_argument("Trying to write word to unknown register");
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
        pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::READ});
    }

    void SharpSM83::sheduleReadWord(uint16_t address) {
        pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::READ_LOW});
        pushMemoryOp(MemoryOp{.address = ++address, .type = MemoryOp::Type::READ_HIGH});
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
        switch (reg) {
        case Registers::AF:
        case Registers::BC:
        case Registers::DE:
        case Registers::HL:
        case Registers::PC:
            pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::READ_LOW, .data = reg});
            pushMemoryOp(MemoryOp{.address = ++address, .type = MemoryOp::Type::READ_HIGH, .data = reg});
            break;
        default: pushMemoryOp(MemoryOp{.address = address, .type = MemoryOp::Type::READ, .data = reg}); break;
        }
    }

    void SharpSM83::sheduleFetchInstruction() {
        sheduleReadByte(reg_.pc);
        if (!prefixed_next_) {
            last_pc_ = reg_.pc;
        }
        if (halt_bug_) {
            halt_bug_ = false;
        } else {
            ++reg_.pc;
        }
        current_instruction_ = {};
    }

    void SharpSM83::sheduleMemoryAcceses(DecodedInstruction instr) {
        switch (instr.src.src) {
        case ArgumentSource::IMMEDIATE_S8:
        case ArgumentSource::IMMEDIATE_U8:
            sheduleReadByte(reg_.pc);
            ++reg_.pc;
            return;
        case ArgumentSource::IMMEDIATE_U16:
            sheduleReadWord(reg_.pc);
            reg_.pc += 2;
            return;
        case ArgumentSource::INDIRECT:
            if (instr.src.reg == Registers::C) {
                return;
            }
            sheduleReadByte(getWordRegister(instr.src.reg));
            return;
        }

        if (instr.dst.src == ArgumentSource::IMMEDIATE_U8) {
            sheduleReadByte(reg_.pc);
            ++reg_.pc;

        } else if (instr.dst.src == ArgumentSource::IMMEDIATE_U16) {

            sheduleReadWord(reg_.pc);
            reg_.pc += 2;
        }
    }

    void SharpSM83::executeMemoryOp() {
        if (memory_op_queue_.empty() || memory_op_executed_) {
            return;
        }

        MemoryOp op = memory_op_queue_.pop_front();
        using enum MemoryOp::Type;
        switch (op.type) {
        case NONE: break;
        case READ:
            if (op.data.is<Registers>()) {
                reg_.setLow(op.data.get<Registers>(), bus_.read(op.address));
            } else {
                data_buffer_.put(bus_.read(op.address));
            }
            break;
        case READ_HIGH:
            if (op.data.is<Registers>()) {
                Registers reg = op.data.get<Registers>();
                if (reg == Registers::PC) {
                    reg_.pc = (reg_.pc & 0x00FF) | (uint16_t(bus_.read(op.address)) << 8);
                } else {
                    reg_.setHigh(reg, bus_.read(op.address));
                }
            } else {
                data_buffer_.putHigh(bus_.read(op.address));
            }
            break;
        case READ_LOW:
            if (op.data.is<Registers>()) {
                Registers reg = op.data.get<Registers>();
                if (reg == Registers::PC) {
                    reg_.pc = (reg_.pc & 0xFF00) | uint16_t(bus_.read(op.address));
                } else {
                    reg_.setLow(reg, bus_.read(op.address));
                }
            } else {
                data_buffer_.putLow(bus_.read(op.address));
            }
            break;
        case WRITE: bus_.write(op.address, op.data.get<uint8_t>()); break;
        }

        memory_op_executed_ = true;
    }
} // namespace gb::cpu