#pragma once

#include "gb/InterruptRegister.h"

#include <cstdint>
#include <array>

namespace gb {

    constexpr uint16_t g_timer_div_address = 0xFF04;
    constexpr uint16_t g_timer_tima_address = 0xFF05;
    constexpr uint16_t g_timer_tma_address = 0xFF06;
    constexpr uint16_t g_timer_tac_address = 0xFF07;

    class Timer {
    public:
        Timer(InterruptRegister& interruptFlags)
            : interrupt_flags_(interruptFlags)
        {}

        void update();
        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data);

    private:

        union {
            //TODO: Wrong init
            uint16_t counter_ = 0xABCC;

            struct {
                uint8_t align;
                uint8_t DIV_;
            };
        };

        uint8_t TIMA_ = 0;
        uint8_t TMA_ = 0;

        struct {
            uint8_t freqency = 0;
            bool enable = false;
        } TAC_;

        bool frequency_bit_was_set_ = false;

        //Frequencies of a timer: 4096 Hz, 262144 Hz, 65536 Hz and 16386 Hz respectively. Frequency is set from bits 0-1 of TAC
        const std::array<uint16_t, 4> frequency_bit_mask_ = { uint16_t(1) << 9, uint16_t(1) << 3, uint16_t(1) << 5, uint16_t(1) << 7 };
        InterruptRegister& interrupt_flags_;

    };
        constexpr uint16_t i0 = 1024 >> 4;
        constexpr uint16_t i01 = 1 << 9;
        constexpr uint16_t i1 = 16 >> 4;
        constexpr uint16_t i11 = 1 << 3;
}
