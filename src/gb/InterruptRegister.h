#pragma once
#include <cstdint>

namespace gb {
    enum class InterruptFlags : uint8_t {
        VBLANK = 1 << 0,
        LCD_STAT = 1 << 1,
        TIMER = 1 << 2,
        SERIAL = 1 << 3,
        JOYPAD = 1 << 4,
    };

    constexpr uint8_t g_unused_interrupt_bits = 0b11100000;
    constexpr uint16_t g_interrupt_enable_address = 0xFFFF;
    constexpr uint16_t g_interrupt_flags_address = 0xFF0F;

    class InterruptRegister {
      public:
        InterruptRegister() : interrupts_(g_unused_interrupt_bits) {}

        inline uint8_t read(uint16_t address) const { return interrupts_; }

        inline void write(uint16_t address, uint8_t data) {
            interrupts_ = (g_unused_interrupt_bits | (data & 0x1F));
        }

        uint8_t read() const { return interrupts_; }

        void write(uint8_t data) { interrupts_ = g_unused_interrupt_bits | data; }

        inline void setFlag(InterruptFlags flag) { interrupts_ |= static_cast<uint8_t>(flag); }

        inline void clearFlag(InterruptFlags flag) { interrupts_ &= ~static_cast<uint8_t>(flag); }

        inline uint8_t getFlags() const { return interrupts_ & (~g_unused_interrupt_bits); }

      private:
        uint8_t interrupts_;
    };
} // namespace gb
