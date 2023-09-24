#pragma once
#include "gb/memory/Memory.h"

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

    inline const uint8_t g_unused_interrupt_bits = 0b11100000;

    class InterruptRegister : public MemoryObject
    {
    public:
        InterruptRegister()
            : interrupts_(g_unused_interrupt_bits)
        {}

        inline uint8_t read(uint16_t address) const override
        {
            return interrupts_;
        }

        inline void write(uint16_t address, uint8_t data) override
        {
            interrupts_ = (g_unused_interrupt_bits | (data & 0x1F));
        }

        inline void setFlag(InterruptFlags flag)
        {
            interrupts_ |= static_cast<uint8_t>(flag);
        }

        inline void clearFlag(InterruptFlags flag)
        {
            interrupts_ &= ~static_cast<uint8_t>(flag);
        }

        inline uint8_t getFlags()
        {
            return interrupts_;
        }

    private:
        uint8_t interrupts_;
    };
}
