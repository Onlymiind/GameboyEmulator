#include "gb/cpu/CPU.h"
#include "gb/AddressBus.h"
#include "utils/Utils.h"
#include "gb/cpu/Operation.h"
#include "gb/cpu/Decoder.h"

#include <climits>
#include <iostream>
#include <cstdint>
#include <string_view>
#include <sstream>
#include <string>
#include <array>
#include <functional>
#include <exception>

//TODO: DAA
namespace gb 
{
    SharpSM83::SharpSM83(const AddressBus& bus, InterruptRegister& interruptEnable, InterruptRegister& interruptFlags, const decoding::Decoder& decoder) 
        : m_InterruptEnable(interruptEnable), m_InterruptFlags(interruptFlags), m_Bus(bus), m_Decoder(decoder), m_CyclesToFinish(0)
    {
        reset();
    }


    void SharpSM83::tick()
    {
        if (m_CyclesToFinish == 0)
        {
            uint8_t pending_interrupts = m_InterruptEnable.getFlags() & m_InterruptFlags.getFlags() & (~g_UnusedInterruptBits);
            if(pending_interrupts)
            {
                //TODO: handle interrupt
                //Can actually safely cast this to InterruptFlags
                InterruptFlags interrupt = static_cast<InterruptFlags>(pending_interrupts & -pending_interrupts);
                if(IME)
                {
                    m_InterruptEnable.clearFlag(interrupt);
                    //TODO: jump to interrupt vector
                }
            }
            else
            {
                decoding::opcode code = fetch();
                //TODO: HALT state (with bug)
                //TODO: Check if opcode is invalid
                m_CyclesToFinish = dispatch(code);
            }

        }

        if (m_EnableIME) 
        { 
            // Enable jumping to interrupt vectors if it is scheduled by EI
            IME = true;
            m_EnableIME = false;
        }

        --m_CyclesToFinish;
    }

    std::string SharpSM83::registersOut()
    {
        std::stringstream stream{};
        
        stream << "CPU registers:\n";

        stream << "A: "; toHexOutput(stream, REG.A); stream << " F: "; toHexOutput(stream, static_cast<uint8_t>(REG.AF & 0x00FF)); stream << "\n";

        stream << "B: "; toHexOutput(stream, REG.B); stream << " C: "; toHexOutput(stream, REG.C); stream << "\n";
        stream << "D: "; toHexOutput(stream, REG.D); stream << " E: "; toHexOutput(stream, REG.E); stream << "\n";
        stream << "H: "; toHexOutput(stream, REG.H); stream << " L: "; toHexOutput(stream, REG.L); stream << "\n";

        stream << "SP: "; toHexOutput(stream, REG.SP); stream << "\n";
        stream << "PC: "; toHexOutput(stream, REG.PC); stream;

        return stream.str();
    }

    bool SharpSM83::halfCarryOccured8Add(uint8_t lhs, uint8_t rhs)
    {
        return (((lhs & 0x0F) + (rhs & 0x0F)) & 0x10) != 0;
    }

    bool SharpSM83::halfCarryOccured8Sub(uint8_t lhs, uint8_t rhs)
    {
        return (lhs & 0x0F) < (rhs & 0x0F);
    }

    bool SharpSM83::halfCarryOccured16Add(uint16_t lhs, uint16_t rhs)
    {
        return (((lhs & 0x0FFF) + (rhs & 0x0FFF)) & 0x1000) != 0;
    }

    void SharpSM83::write(uint16_t address, uint8_t data)
    {
        m_Bus.write(address, data);
    }

    uint8_t SharpSM83::dispatch(decoding::opcode code)
    {
        if(m_Decoder.isPrefix(code))
        {
            code.code = fetch();
            decoding::PrefixedInstruction instr = m_Decoder.decodePrefixed(code);
            return dispatchPrefixed(instr);
        }
        else
        {
            decoding::UnprefixedInstruction instr = m_Decoder.decodeUnprefixed(code);
            return dispatchUnprefixed(instr);
        }
    }

    uint8_t SharpSM83::dispatchPrefixed(decoding::PrefixedInstruction instr)
    {
        using type = decoding::PrefixedType;
        switch(instr.Type)
        {
            case type::RLC: return RLC(instr.Target);
            case type::RRC: return RRC(instr.Target);
            case type::RL: return RL(instr.Target);
            case type::RR: return RR(instr.Target);
            case type::SLA: return SLA(instr.Target);
            case type::SRA: return SRA(instr.Target);
            case type::SRL: return SRL(instr.Target);
            case type::SWAP: return SWAP(instr.Target);
            case type::BIT: return BIT(instr);
            case type::RES: return RES(instr);
            case type::SET: return SET(instr);
            default:
                throw std::invalid_argument(PrintToString("", "Unknown prefixed instruction: ", instr.Type));
                return 0;
        }
    }

    uint8_t SharpSM83::dispatchUnprefixed(decoding::UnprefixedInstruction instr)
    {
        using type = decoding::UnprefixedType;
        switch(instr.Type)
        {
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
            case type::RST: return RST(*instr.ResetVector);
            case type::PUSH: return PUSH(instr.Source.Register);
            case type::POP: return POP(instr.Destination.Register);
            case type::SUB: return SUB(instr.Source);
            case type::OR: return OR(instr.Source);
            case type::AND: return AND(instr.Source);
            case type::XOR: return XOR(instr.Source);
            case type::ADC: return ADC(instr.Source);
            case type::SBC: return SBC(instr.Source);
            case type::CP: return CP(instr.Source);
            case type::JR: return JR(instr.Condition);
            case type::CALL: return CALL(instr.Condition);
            case type::RET: return RET(instr.Condition);
            case type::JP: return JP(instr);
            case type::INC: return INC(instr.Source);
            case type::DEC: return DEC(instr.Source);
            case type::LD: return LD(instr);
            case type::ADD: return ADD(instr);
            default:
                throw std::invalid_argument(PrintToString("", "Unknown unprefixed instruction: ", instr.Type));
                return 0;
        }
    }

    uint8_t SharpSM83::read(uint16_t address) const
    {
        return m_Bus.read(address);
    }

    void SharpSM83::pushStack(uint16_t value)
    {
        uint8_t lsb = 0;
        uint8_t msb = 0;

        lsb = static_cast<uint8_t>(value & 0x00FF);
        msb = static_cast<uint8_t>((value & 0xFF00) >> 8);

        --REG.SP;
        write(REG.SP, msb);
        --REG.SP;
        write(REG.SP, lsb);
    }

    uint8_t SharpSM83::fetch()
    {
        uint8_t value = read(REG.PC);
        ++REG.PC;
        return value;
    }

    uint16_t SharpSM83::fetchWord()
    {
        uint8_t lsb = 0;
        uint8_t msb = 0;
        uint16_t value;
        lsb = fetch();
        msb = fetch();
        value = (static_cast<uint16_t>(msb) << 8) | (static_cast<uint16_t>(lsb));
        return value;
    }

    uint16_t SharpSM83::popStack()
    {
        uint8_t lsb = 0;
        uint8_t msb = 0;
        lsb = read(REG.SP);
        ++REG.SP;
        msb = read(REG.SP);
        ++REG.SP;
        uint16_t result = (static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb);
        return result;
    }

    int8_t SharpSM83::fetchSigned()
    {
        uint8_t u_value = fetch();
        int8_t result = reinterpret_cast<int8_t&>(u_value);
        return result;
    }

    void SharpSM83::reset()
    {
        REG.AF = 0x01B0;
        REG.BC = 0x0013;
        REG.DE = 0x00D8;
        REG.HL = 0x014D;

        REG.SP = 0xFFFE;
        REG.PC = 0x0100;

        IME = false;
    }

    uint8_t SharpSM83::getByte(decoding::ArgumentInfo from)
    {
        switch(from.Source)
        {
            case decoding::ArgumentSource::Immediate: return fetch();
            case decoding::ArgumentSource::IndirectImmediate: return read(fetchWord());
            case decoding::ArgumentSource::Register:
            case decoding::ArgumentSource::Indirect:
                return getByteRegister(from.Register);
            default:
                throw std::invalid_argument("Trying to get byte from unknown source");
                return 0;
        }
    }
    uint16_t SharpSM83::getWord(decoding::ArgumentInfo from)
    {
        switch(from.Source)
        {
            case decoding::ArgumentSource::Immediate: return fetchWord();
            case decoding::ArgumentSource::Register: return getWordRegister(from.Register);
            default:
                throw std::invalid_argument("Trying to get word from unknown source");
                return 0;
        }
    }

    uint8_t SharpSM83::getByteRegister(decoding::Registers reg) const
    {
#define CASE_BYTE_REG(x) case decoding::Registers::##x: return REG.##x
#define CASE_WORD_REG(x) case decoding::Registers::##x: return read(REG.##x)
        switch(reg)
        {
            CASE_BYTE_REG(A);
            CASE_BYTE_REG(B);
            CASE_BYTE_REG(C);
            CASE_BYTE_REG(D);
            CASE_BYTE_REG(E);
            CASE_BYTE_REG(H);
            CASE_BYTE_REG(L);
            CASE_WORD_REG(HL);
            CASE_WORD_REG(BC);
            CASE_WORD_REG(DE);
            default:
                throw std::invalid_argument("Trying to get byte from unknown register");
                return 0;
        }
#undef CASE_BYTE_REG
#undef CASE_WORD_REG
    }

    uint16_t SharpSM83::getWordRegister(decoding::Registers reg) const
    {
#define CASE_REG(x) case decoding::Registers::##x: return REG.##x
        switch(reg)
        {
            CASE_REG(BC);
            CASE_REG(DE);
            CASE_REG(HL);
            CASE_REG(SP);
            case decoding::Registers::AF: return REG.AF & 0xFFF0;
            default:
                throw std::invalid_argument("Trying to get byte from unknown register");
                return 0;
        }
#undef CASE_REG
    }

    void SharpSM83::setByteRegister(decoding::Registers reg, uint8_t data)
    {
#define CASE_BYTE_REG(x) case decoding::Registers::##x: REG.##x = data; return
#define CASE_WORD_REG(x) case decoding::Registers::##x: write(REG.##x, data); return
        switch(reg)
        {
            CASE_BYTE_REG(A);
            CASE_BYTE_REG(B);
            CASE_BYTE_REG(C);
            CASE_BYTE_REG(D);
            CASE_BYTE_REG(E);
            CASE_BYTE_REG(H);
            CASE_BYTE_REG(L);
            CASE_WORD_REG(HL);
            CASE_WORD_REG(BC);
            CASE_WORD_REG(DE);
            default:
                throw std::invalid_argument("Trying to write byte to unknown register");
        }
#undef CASE_BYTE_REG
#undef CASE_WORD_REG
    }

    void SharpSM83::setWordRegister(decoding::Registers reg, uint16_t data)
    {
#define CASE_REG(x) case decoding::Registers::##x:  REG.##x = data; return
        switch(reg)
        {
            CASE_REG(BC);
            CASE_REG(DE);
            CASE_REG(HL);
            CASE_REG(SP);
            case decoding::Registers::AF: REG.AF = data & 0xFFF0; return;
            default:
                throw std::invalid_argument("Trying to write word to unknown register");
        }
#undef CASE_REG
    }

    bool SharpSM83::checkCondition(decoding::Conditions condition)
    {
        switch(condition)
        {
            case decoding::Conditions::Carry: return REG.Flags.C != 0;
            case decoding::Conditions::NotCarry: return REG.Flags.C == 0;
            case decoding::Conditions::Zero: return REG.Flags.Z != 0;
            case decoding::Conditions::NotZero: return REG.Flags.Z == 0;
            default:
                //Never happens
                return false;
        }
    }

}
