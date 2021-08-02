#include "core/gb/cpu/CPU.h"
#include "core/gb/AddressBus.h"
#include "utils/Utils.h"
#include "core/gb/cpu/Operation.h"
#include "core/gb/cpu/Decoder.h"

#include <climits>
#include <iostream>
#include <cstdint>
#include <string_view>
#include <sstream>
#include <string>
#include <array>
#include <functional>


namespace gb 
{
    SharpSM83::SharpSM83(const AddressBus& bus, InterruptRegister& interruptEnable, InterruptRegister& interruptFlags, const Decoder& decoder) 
        : m_InterruptEnable(interruptEnable), m_InterruptFlags(interruptFlags), m_Bus(bus), m_Decoder(decoder), m_CyclesToFinish(0)
    {}


    void SharpSM83::tick()
    {
        if (m_CyclesToFinish == 0)
        {
            uint8_t pending_interrupts = m_InterruptEnable.getFlags() & m_InterruptFlags.getFlags() & (~g_UnusedInterruptBits);
            if(pending_interrupts)
            {
                //TODO: handle interrupt
                //Can actually safely cast this to InterruptBits
                InterruptFlags interrupt = static_cast<InterruptFlags>(pending_interrupts & -pending_interrupts);
                if(IME)
                {
                    m_InterruptEnable.clearFlag(interrupt);
                    //TODO: jump to interrupt vector
                }
            }
            else
            {
                opcode code = fetch();
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

    bool SharpSM83::carryOccured8Add(uint8_t lhs, uint8_t rhs)
    {
        uint16_t lhs16{ lhs }, rhs16{ rhs };

        return (lhs16 + rhs16) > 0x00FF;
    }

    bool SharpSM83::carryOccured8Sub(uint8_t lhs, uint8_t rhs)
    {
        return lhs < rhs;
    }

    bool SharpSM83::halfCarryOccured16Add(uint16_t lhs, uint16_t rhs)
    {
        return (((lhs & 0x0FFF) + (rhs & 0x0FFF)) & 0x1000) != 0;
    }

    bool SharpSM83::carryOccured16Add(uint16_t lhs, uint16_t rhs)
    {
        uint32_t lhs32{ lhs }, rhs32{ rhs };

        return (lhs32 + rhs32) > 0xFFFF;
    }

    void SharpSM83::write(uint16_t address, uint8_t data)
    {
        m_Bus.write(address, data);
    }

    uint8_t SharpSM83::dispatch(opcode code)
    {
        if(m_Decoder.isPrefix(code))
        {
            code.code = fetch();
            PrefixedInstruction instr = m_Decoder.decodePrefixed(code);
            return dispatchPrefixed(instr);
        }
        else
        {
            UnprefixedInstruction instr = m_Decoder.decodeUnprefixed(code);
            return dispatchUnprefixed(instr);
        }
    }

    uint8_t SharpSM83::dispatchPrefixed(PrefixedInstruction instr)
    {
        using type = PrefixedType;
        switch(instr.Type)
        {
            case type::RLC: return 0;
            case type::RRC: return 0;
            case type::RL: return 0;
            case type::RR: return 0;
            case type::SLA: return 0;
            case type::SRA: return 0;
            case type::SWAP: return 0;
            case type::BIT: return 0;
            case type::RES: return 0;
            case type::SET: return 0;
        }
    }

    uint8_t SharpSM83::dispatchUnprefixed(UnprefixedInstruction instr)
    {

    }

    uint8_t SharpSM83::read(uint16_t address)
    {
        return m_Bus.read(address);
    }

    void SharpSM83::pushStack(uint16_t value)
    {
        uint8_t msb{ 0 }, lsb{ 0 };

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
        uint8_t lsb{ 0 }, msb{ 0 };
        uint16_t value;
        lsb = fetch();
        msb = fetch();
        value = (static_cast<uint16_t>(msb) << 8) | (static_cast<uint16_t>(lsb));
        return value;
    }

    uint16_t SharpSM83::popStack()
    {
        uint8_t msb{ 0 }, lsb{ 0 };
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
        REG.AF = 0;
        REG.BC = 0;
        REG.DE = 0;
        REG.HL = 0;

        REG.SP = 0xFFFE;
        REG.PC = 0x0100;

        IME = false;
    }

    uint8_t SharpSM83::getByteRegister(Registers reg)
    {
#define CASE_BYTE_REG(x) case Registers::##x: return REG.##x
#define CASE_WORD_REG(x) case Registers::##x: return read(REG.##x)
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
                //TODO: throw an error
                return 0;
        }
#undef CASE_BYTE_REG
#undef CASE_WORD_REG
    }

    uint16_t SharpSM83::getWordRegister(Registers reg)
    {
#define CASE_REG(x) case Registers::##x: return REG.##x
        switch(reg)
        {
            CASE_REG(BC);
            CASE_REG(DE);
            CASE_REG(HL);
            CASE_REG(SP);
        }
        return 0;
#undef CASE_REG
    }

    void SharpSM83::setByteRegister(Registers reg, uint8_t data)
    {
#define CASE_BYTE_REG(x) case Registers::##x: REG.##x = data; break
#define CASE_WORD_REG(x) case Registers::##x: write(REG.##x, data); break
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
                //TODO: throw an error
                return;
        }
#undef CASE_BYTE_REG
#undef CASE_WORD_REG
    }

    void SharpSM83::setWordRegister(Registers reg, uint16_t data)
    {
#define CASE_REG(x) case Registers::##x:  write(REG.##x, data); break
        switch(reg)
        {
            CASE_REG(BC);
            CASE_REG(DE);
            CASE_REG(HL);
            CASE_REG(SP);
        }
#undef CASE_REG
    }

    bool SharpSM83::checkCondition(Conditions condition)
    {
        switch(condition)
        {
            case Conditions::Carry: return REG.Flags.C != 0;
            case Conditions::NotCarry: return REG.Flags.C == 0;
            case Conditions::Zero: return REG.Flags.Z != 0;
            case Conditions::NotZero: return REG.Flags.Z == 0;
        }
        return false;
    }

}
