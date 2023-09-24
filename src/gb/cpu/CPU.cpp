#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "utils/Utils.h"
#include "gb/cpu/Operation.h"
#include "gb/cpu/Decoder.h"

#include <cstdint>
#include <exception>

namespace gb::cpu {

    SharpSM83::SharpSM83(const AddressBus& bus, InterruptRegister& interruptEnable, InterruptRegister& interruptFlags) 
        : interrupt_enable_(interruptEnable), interrupt_flags_(interruptFlags), bus_(bus), cycles_to_finish_(0)
    {
        reset();
    }


    void SharpSM83::tick() {
        std::optional<InterruptFlags> interrupt = getPendingInterrupt();

        //FIXME: possible bug: what if interrupt flag with higher priority is set during HALT "execution"?
        if(halt_mode_ && interrupt) {
            halt_mode_ = false;
        }

        if (cycles_to_finish_ == 0) {
            if (enable_IME_) {
                // Enable jumping to interrupt vectors if it is scheduled by EI
                IME_ = true;
                enable_IME_ = false;
            }
            if(interrupt && IME_) {
                handleInterrupt(*interrupt);
            } else {
                decoding::opcode code = halt_bug_ ? read(reg_.PC) : fetch();
                cycles_to_finish_ = dispatch(code);
            }

            if (enable_IME_) { 
                // Enable jumping to interrupt vectors if it is scheduled by EI
                IME_ = true;
                enable_IME_ = false;
            }
        }
        if(!halt_mode_) {
            --cycles_to_finish_;
        }
    }

    std::optional<InterruptFlags> SharpSM83::getPendingInterrupt() const {
        uint8_t pending_interrupts = (interrupt_enable_.getFlags() & interrupt_flags_.getFlags()) & (~g_unused_interrupt_bits);
        if(pending_interrupts != 0) {
            return static_cast<InterruptFlags>(pending_interrupts & -pending_interrupts);
        } else {
            return {};
        }
    }

    void SharpSM83::handleInterrupt(InterruptFlags interrupt) {
        interrupt_enable_.clearFlag(interrupt);
        pushStack(reg_.PC);
        reg_.PC = interrupt_vectors_.at(interrupt);
        cycles_to_finish_ = 5;
    }

    Registers SharpSM83::getRegisters() const {
        return reg_;
    }

    bool SharpSM83::halfCarryOccured8Add(uint8_t lhs, uint8_t rhs) {
        return (((lhs & 0x0F) + (rhs & 0x0F)) & 0x10) != 0;
    }

    bool SharpSM83::halfCarryOccured8Sub(uint8_t lhs, uint8_t rhs) {
        return (lhs & 0x0F) < (rhs & 0x0F);
    }

    bool SharpSM83::halfCarryOccured16Add(uint16_t lhs, uint16_t rhs) {
        return (((lhs & 0x0FFF) + (rhs & 0x0FFF)) & 0x1000) != 0;
    }

    void SharpSM83::write(uint16_t address, uint8_t data) {
        bus_.write(address, data);
    }

    uint8_t SharpSM83::dispatch(decoding::opcode code) {
        if(decoding::isPrefix(code)) {
            code.code = fetch();
            decoding::PrefixedInstruction instr = decoding::decodePrefixed(code);
            return dispatchPrefixed(instr);
        } else {
            decoding::UnprefixedInstruction instr = decoding::decodeUnprefixed(code);
            return dispatchUnprefixed(instr);
        }
    }

    uint8_t SharpSM83::dispatchPrefixed(decoding::PrefixedInstruction instr) {
        using type = decoding::PrefixedType;
        switch(instr.type) {
            case type::RLC: return RLC(instr.target);
            case type::RRC: return RRC(instr.target);
            case type::RL: return RL(instr.target);
            case type::RR: return RR(instr.target);
            case type::SLA: return SLA(instr.target);
            case type::SRA: return SRA(instr.target);
            case type::SRL: return SRL(instr.target);
            case type::SWAP: return SWAP(instr.target);
            case type::BIT: return BIT(instr);
            case type::RES: return RES(instr);
            case type::SET: return SET(instr);
            default:
                throw std::invalid_argument(printToString("", "Unknown prefixed instruction: ", instr.type));
                return 0;
        }
    }

    uint8_t SharpSM83::dispatchUnprefixed(decoding::UnprefixedInstruction instr) {
        using type = decoding::UnprefixedType;
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
            case type::PUSH: return PUSH(instr.source.reg);
            case type::POP: return POP(instr.destination.reg);
            case type::SUB: return SUB(instr.source);
            case type::OR: return OR(instr.source);
            case type::AND: return AND(instr.source);
            case type::XOR: return XOR(instr.source);
            case type::ADC: return ADC(instr.source);
            case type::SBC: return SBC(instr.source);
            case type::CP: return CP(instr.source);
            case type::JR: return JR(instr.condition);
            case type::CALL: return CALL(instr.condition);
            case type::RET: return RET(instr.condition);
            case type::JP: return JP(instr);
            case type::INC: return INC(instr.source);
            case type::DEC: return DEC(instr.source);
            case type::LD: return LD(instr);
            case type::ADD: return ADD(instr);
            default:
                throw std::invalid_argument(printToString("", "Unknown unprefixed instruction: ", instr.type));
                return 0;
        }
    }

    uint8_t SharpSM83::read(uint16_t address) const {
        return bus_.read(address);
    }

    void SharpSM83::pushStack(uint16_t value) {
        uint8_t lsb = 0;
        uint8_t msb = 0;

        lsb = static_cast<uint8_t>(value & 0x00FF);
        msb = static_cast<uint8_t>((value & 0xFF00) >> 8);

        --reg_.SP;
        write(reg_.SP, msb);
        --reg_.SP;
        write(reg_.SP, lsb);
    }

    uint8_t SharpSM83::fetch() {
        uint8_t value = read(reg_.PC);
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
        lsb = read(reg_.SP);
        ++reg_.SP;
        msb = read(reg_.SP);
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
        reg_.AF = 0x01B0;
        reg_.BC = 0x0013;
        reg_.DE = 0x00D8;
        reg_.HL = 0x014D;

        reg_.SP = 0xFFFE;
        reg_.PC = 0x0100;

        IME_ = false;
    }

    uint8_t SharpSM83::getByte(decoding::ArgumentInfo from) {
        switch(from.source) {
            case decoding::ArgumentSource::Immediate: return fetch();
            case decoding::ArgumentSource::IndirectImmediate: return read(fetchWord());
            case decoding::ArgumentSource::Register:
            case decoding::ArgumentSource::Indirect:
                return getByteRegister(from.reg);
            default:
                throw std::invalid_argument("Trying to get byte from unknown source");
                return 0;
        }
    }

    uint16_t SharpSM83::getWord(decoding::ArgumentInfo from) {
        switch(from.source) {
            case decoding::ArgumentSource::Immediate: return fetchWord();
            case decoding::ArgumentSource::Register: return getWordRegister(from.reg);
            default:
                throw std::invalid_argument("Trying to get word from unknown source");
                return 0;
        }
    }

    uint8_t SharpSM83::getByteRegister(decoding::Registers reg) const {
        switch(reg) {
            case decoding::Registers::A: return reg_.A;
            case decoding::Registers::B: return reg_.B;
            case decoding::Registers::C: return reg_.C;
            case decoding::Registers::D: return reg_.D;
            case decoding::Registers::E: return reg_.E;
            case decoding::Registers::H: return reg_.H;
            case decoding::Registers::L: return reg_.L;
            case decoding::Registers::HL: return read(reg_.HL);
            case decoding::Registers::BC: return read(reg_.BC);
            case decoding::Registers::DE: return read(reg_.DE);
            default:
                throw std::invalid_argument("Trying to get byte from unknown register");
                return 0;
        }
    }

    uint16_t SharpSM83::getWordRegister(decoding::Registers reg) const {
        switch(reg) {
            case decoding::Registers::BC: return reg_.BC;
            case decoding::Registers::DE: return reg_.DE;
            case decoding::Registers::HL: return reg_.HL;
            case decoding::Registers::SP: return reg_.SP;
            case decoding::Registers::AF: return reg_.AF & 0xFFF0;
            default:
                throw std::invalid_argument("Trying to get byte from unknown register");
                return 0;
        }
    }

    void SharpSM83::setByteRegister(decoding::Registers reg, uint8_t data) {
        switch(reg) {
            case decoding::Registers::A: reg_.A = data; return;
            case decoding::Registers::B: reg_.B = data; return;
            case decoding::Registers::C: reg_.C = data; return;
            case decoding::Registers::D: reg_.D = data; return;
            case decoding::Registers::E: reg_.E = data; return;
            case decoding::Registers::H: reg_.H = data; return;
            case decoding::Registers::L: reg_.L = data; return;
            case decoding::Registers::HL: write(reg_.HL, data); return;
            case decoding::Registers::BC: write(reg_.BC, data); return;
            case decoding::Registers::DE: write(reg_.DE, data); return;
            default:
                throw std::invalid_argument("Trying to write byte to unknown register");
        }
    }

    void SharpSM83::setWordRegister(decoding::Registers reg, uint16_t data) {
        switch(reg) {
            case decoding::Registers::BC:  reg_.BC = data; return;
            case decoding::Registers::DE:  reg_.DE = data; return;
            case decoding::Registers::HL:  reg_.HL = data; return;
            case decoding::Registers::SP:  reg_.SP = data; return;
            case decoding::Registers::AF: reg_.AF = data & 0xFFF0; return;
            default:
                throw std::invalid_argument("Trying to write word to unknown register");
        }
    }

    bool SharpSM83::checkCondition(decoding::Conditions condition) {
        switch(condition) {
            case decoding::Conditions::Carry: return reg_.flags.C != 0;
            case decoding::Conditions::NotCarry: return reg_.flags.C == 0;
            case decoding::Conditions::Zero: return reg_.flags.Z != 0;
            case decoding::Conditions::NotZero: return reg_.flags.Z == 0;
            default:
                //Never happens
                return false;
        }
    }
}
