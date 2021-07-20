#pragma once
#include "core/gb/MemoryObject.h"

#include <cstdint>

namespace gb
{
    enum class InterruptFlags : uint8_t
    {
        VBlank   = 1 << 0,
        LCD_STAT = 1 << 1,
        Timer    = 1 << 2,
        Serial   = 1 << 3,
        Joypad   = 1 << 4,
    };

    inline const uint8_t g_UnusedInterruptBits = 0b11100000;

    class InterruptRegister : public MemoryObject
    {
    public:
        InterruptRegister()
            : m_Interrupts(g_UnusedInterruptBits)
        {}

        uint8_t read(uint16_t address) override
        {
            return m_Interrupts;
        }

        void write(uint16_t address, uint8_t data) override
        {
            m_Interrupts = (g_UnusedInterruptBits | (data & 0x1F));
        }

        inline void setFlag(InterruptFlags flag)
        {
            m_Interrupts |= static_cast<uint8_t>(flag);
        }

        inline void clearFlag(InterruptFlags flag)
        {
            m_Interrupts &= ~static_cast<uint8_t>(flag);
        }

        inline uint8_t getFlags()
        {
            return m_Interrupts;
        }

    private:
        uint8_t m_Interrupts;
    };
}