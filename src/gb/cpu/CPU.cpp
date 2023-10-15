#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/CPUUtils.h"
#include "utils/Utils.h"
#include "gb/cpu/Operation.h"
#include "gb/cpu/Decoder.h"

#include <cstdint>
#include <exception>
#include <stdexcept>

namespace gb::cpu {

    SharpSM83::SharpSM83(AddressBus& bus) 
        :bus_(bus)
    {
        reset();
    }


    void SharpSM83::tick() {
        std::optional<InterruptFlags> interrupt = getPendingInterrupt();

        //FIXME: possible bug: what if interrupt flag with higher priority is set during HALT "execution"?
        if(halt_mode_ && bus_.getInterruptFlags().getFlags()) {
            halt_mode_ = false;
        }

        if (cycles_to_finish_ == 0) {
            last_instruction_ = Instruction{};
            if (enable_IME_) {
                // Enable jumping to interrupt vectors if it is scheduled by EI
                IME_ = true;
                enable_IME_ = false;
            }
            if(interrupt && IME_) {
                handleInterrupt(*interrupt);
            } else {
                last_instruction_.pc = reg_.PC;
                opcode code = halt_bug_ ? bus_.read(reg_.PC) : fetch();
                bool is_prefixed = isPrefix(code);
                if(is_prefixed) {
                    code = fetch();
                }
                cycles_to_finish_ = dispatch(code, is_prefixed);
            }
        }
        if(!halt_mode_) {
            --cycles_to_finish_;
        }
    }

    std::optional<InterruptFlags> SharpSM83::getPendingInterrupt() const {
        uint8_t pending_interrupts = bus_.getInterruptEnable().getFlags() & bus_.getInterruptFlags().getFlags();
        if(pending_interrupts != 0) {
            return static_cast<InterruptFlags>(pending_interrupts & -pending_interrupts);
        } else {
            return {};
        }
    }

    void SharpSM83::handleInterrupt(InterruptFlags interrupt) {
        bus_.getInterruptEnable().clearFlag(interrupt);
        bus_.getInterruptFlags().clearFlag(interrupt);
        pushStack(reg_.PC);
        reg_.PC = g_interrupt_vectors.at(interrupt);
        cycles_to_finish_ = 5;
    }

    uint8_t SharpSM83::dispatch(opcode code, bool prefixed) {
        DecodedInstruction instr = prefixed ? decodePrefixed(code) : decodeUnprefixed(code);
        if(prefixed) {
            instr = decodePrefixed(code);
            last_instruction_.type = instr.type;
            last_instruction_.arg() = instr.arg().reg;
            if(instr.bit) {
                last_instruction_.bit() = *instr.bit;
            }
        } else {
            instr = decodeUnprefixed(code);
            last_instruction_.type = instr.type;
            last_instruction_.load_subtype = instr.LD_subtype;
            last_instruction_.condition = instr.condition;
        }
        using type = InstructionType;
        switch(instr.type) {
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
        case type::RST: return RST(*instr.reset_vector);
        case type::PUSH: return PUSH(instr.arg().reg);
        case type::POP: return POP(instr.destination.reg);
        case type::SUB: return SUB(instr.arg());
        case type::OR: return OR(instr.arg());
        case type::AND: return AND(instr.arg());
        case type::XOR: return XOR(instr.arg());
        case type::ADC: return ADC(instr.arg());
        case type::SBC: return SBC(instr.arg());
        case type::CP: return CP(instr.arg());
        case type::JR: return JR(instr.condition);
        case type::CALL: return CALL(instr.condition);
        case type::RET: return RET(instr.condition);
        case type::JP: return JP(instr);
        case type::INC: return INC(instr.arg());
        case type::DEC: return DEC(instr.arg());
        case type::LD: return LD(instr);
        case type::ADD: return ADD(instr);
        case type::RLC: return RLC(instr.arg().reg);
        case type::RRC: return RRC(instr.arg().reg);
        case type::RL: return RL(instr.arg().reg);
        case type::RR: return RR(instr.arg().reg);
        case type::SLA: return SLA(instr.arg().reg);
        case type::SRA: return SRA(instr.arg().reg);
        case type::SRL: return SRL(instr.arg().reg);
        case type::SWAP: return SWAP(instr.arg().reg);
        case type::BIT: return BIT(instr.arg().reg, *instr.bit);
        case type::RES: return RES(instr.arg().reg, *instr.bit);
        case type::SET: return SET(instr.arg().reg, *instr.bit);
        default:
            throw std::runtime_error("unknown instruction");
        }
    }

    void SharpSM83::pushStack(uint16_t value) {
        uint8_t lsb = 0;
        uint8_t msb = 0;

        lsb = static_cast<uint8_t>(value);
        msb = static_cast<uint8_t>(value >> 8);

        --reg_.SP;
        bus_.write(reg_.SP, msb);
        --reg_.SP;
        bus_.write(reg_.SP, lsb);
    }

    uint8_t SharpSM83::fetch() {
        uint8_t value = bus_.read(reg_.PC);
        ++reg_.PC;
        return value;
    }

    uint16_t SharpSM83::fetchWord() {
        uint8_t lsb = 0;
        uint8_t msb = 0;
        uint16_t value;
        lsb = fetch();
        msb = fetch();
        value = (static_cast<uint16_t>(msb) << 8) | (static_cast<uint16_t>(lsb));
        return value;
    }

    uint16_t SharpSM83::popStack() {
        uint8_t lsb = 0;
        uint8_t msb = 0;
        lsb = bus_.read(reg_.SP);
        ++reg_.SP;
        msb = bus_.read(reg_.SP);
        ++reg_.SP;
        uint16_t result = (static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb);
        return result;
    }

    int8_t SharpSM83::fetchSigned() {
        uint8_t u_value = fetch();
        int8_t result = reinterpret_cast<int8_t&>(u_value);
        return result;
    }

    void SharpSM83::reset() {
        reg_.setAF(0x01B0);
        reg_.BC() = 0x0013;
        reg_.DE() = 0x00D8;
        reg_.HL() = 0x014D;

        reg_.SP = 0xFFFE;
        reg_.PC = 0x0100;

        IME_ = false;

        last_instruction_ = Instruction{};
    }

    uint8_t SharpSM83::getByte(ArgumentInfo from) {
        switch(from.source) {
            case ArgumentSource::Immediate: return fetch();
            case ArgumentSource::IndirectImmediate: return bus_.read(fetchWord());
            case ArgumentSource::Register:
            case ArgumentSource::Indirect:
                return getByteRegister(from.reg);
            default:
                throw std::invalid_argument("Trying to get byte from unknown source");
                return 0;
        }
    }

    uint16_t SharpSM83::getWord(ArgumentInfo from) {
        switch(from.source) {
            case ArgumentSource::Immediate: return fetchWord();
            case ArgumentSource::Register: return getWordRegister(from.reg);
            default:
                throw std::invalid_argument("Trying to get word from unknown source");
                return 0;
        }
    }

    uint8_t SharpSM83::getByteRegister(Registers reg) const {
        switch(reg) {
            case Registers::A: return reg_.A();
            case Registers::B: return reg_.B();
            case Registers::C: return reg_.C();
            case Registers::D: return reg_.D();
            case Registers::E: return reg_.E();
            case Registers::H: return reg_.H();
            case Registers::L: return reg_.L();
            case Registers::HL: return bus_.read(reg_.HL());
            case Registers::BC: return bus_.read(reg_.BC());
            case Registers::DE: return bus_.read(reg_.DE());
            default:
                throw std::invalid_argument("Trying to get byte from unknown register");
                return 0;
        }
    }

    uint16_t SharpSM83::getWordRegister(Registers reg) const {
        switch(reg) {
            case Registers::BC: return reg_.BC();
            case Registers::DE: return reg_.DE();
            case Registers::HL: return reg_.HL();
            case Registers::SP: return reg_.SP;
            case Registers::AF: return reg_.AF();
            default:
                throw std::invalid_argument("Trying to get byte from unknown register");
                return 0;
        }
    }

    void SharpSM83::setByteRegister(Registers reg, uint8_t data) {
        switch(reg) {
            case Registers::A: reg_.A() = data; return;
            case Registers::B: reg_.B() = data; return;
            case Registers::C: reg_.C() = data; return;
            case Registers::D: reg_.D() = data; return;
            case Registers::E: reg_.E() = data; return;
            case Registers::H: reg_.H() = data; return;
            case Registers::L: reg_.L() = data; return;
            case Registers::HL: bus_.write(reg_.HL(), data); return;
            case Registers::BC: bus_.write(reg_.BC(), data); return;
            case Registers::DE: bus_.write(reg_.DE(), data); return;
            default:
                throw std::invalid_argument("Trying to write byte to unknown register");
        }
    }

    void SharpSM83::setWordRegister(Registers reg, uint16_t data) {
        switch(reg) {
            case Registers::BC:  reg_.BC() = data; return;
            case Registers::DE:  reg_.DE() = data; return;
            case Registers::HL:  reg_.HL() = data; return;
            case Registers::SP:  reg_.SP = data; return;
            case Registers::AF: reg_.setAF(data); return;
            default:
                throw std::invalid_argument("Trying to write word to unknown register");
        }
    }

    bool SharpSM83::checkCondition(Conditions condition) {
        switch(condition) {
            case Conditions::Carry: return reg_.getFlag(Flags::CARRY);
            case Conditions::NotCarry: return !reg_.getFlag(Flags::CARRY);
            case Conditions::Zero: return reg_.getFlag(Flags::ZERO);
            case Conditions::NotZero: return !reg_.getFlag(Flags::ZERO);
            default:
                //Never happens
                return false;
        }
    }

    void SharpSM83::setArgData(Instruction::Argument& arg, ArgumentInfo info, uint8_t data) {
        if(info.source == ArgumentSource::Register || info.source == ArgumentSource::Indirect) {
            arg = info.reg;
        } else {
            arg = data;
        }
    }

    void SharpSM83::sheduleReadByte(uint16_t address) {
        memory_op_queue_.push_back(MemoryOp{.address = address, .type = MemoryOp::Type::READ});
    }

    void SharpSM83::sheduleReadWord(uint16_t address) {
        memory_op_queue_.push_back(MemoryOp{.address = address, .type = MemoryOp::Type::READ_LOW});
        memory_op_queue_.push_back(MemoryOp{.address = ++address, .type = MemoryOp::Type::READ_HIGH});
    }

    void SharpSM83::sheduleWriteByte(uint16_t address, uint8_t data) {
        memory_op_queue_.push_back(MemoryOp{.address = address, .type = MemoryOp::Type::WRITE, .data = data});
    }

    void SharpSM83::sheduleWriteWord(uint16_t address, uint16_t data) {
        memory_op_queue_.push_back(MemoryOp{.address = address, .type = MemoryOp::Type::WRITE, .data = uint8_t(data)});
        memory_op_queue_.push_back(MemoryOp{.address = ++address, .type = MemoryOp::Type::WRITE, .data = uint8_t(data >> 8)});
    }

    void SharpSM83::shedulePushStack(uint16_t data) {
        memory_op_queue_.push_back(MemoryOp{.address = uint16_t(reg_.SP - 1), .type = MemoryOp::Type::WRITE, .data = uint8_t(data >> 8)});
        memory_op_queue_.push_back(MemoryOp{.address = uint16_t(reg_.SP - 2), .type = MemoryOp::Type::WRITE, .data = uint8_t(data)});
    }

    void SharpSM83::sheduleMemoryAcceses(DecodedInstruction instr) {
        switch(instr.source.source) {
        case ArgumentSource::Immediate:
        case ArgumentSource::IndirectImmediate:
            //Just read the immediate
            if(instr.source.type == ArgumentType::Unsigned16) {
                sheduleReadWord(reg_.PC);
                reg_.PC += 2;
            } else {
                sheduleReadByte(reg_.PC);
                ++reg_.PC;
            }
            return;
        case ArgumentSource::Indirect:
            if(instr.source.reg == Registers::C) {
                sheduleReadByte(uint16_t(0xFF00) + uint16_t(reg_.C()));
            } else {
                sheduleReadByte(getWordRegister(instr.source.reg));
            }
            return;
        }

        if(instr.destination.source == ArgumentSource::IndirectImmediate || instr.destination.source == ArgumentSource::Immediate) {
            if(instr.source.type == ArgumentType::Unsigned16) {
                sheduleReadWord(reg_.PC);
                reg_.PC += 2;
            } else {
                sheduleReadByte(reg_.PC);
                ++reg_.PC;
            }
        }
    }
}
