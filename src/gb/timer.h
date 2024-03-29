#ifndef GB_EMULATOR_SRC_GB_TIMER_HDR_
#define GB_EMULATOR_SRC_GB_TIMER_HDR_

#include "gb/interrupt_register.h"

#include <array>
#include <cstdint>

namespace gb {

    // Frequencies of a timer: 4096 Hz, 262144 Hz, 65536 Hz and 16386 Hz
    // respectively. Frequency is set from bits 0-1 of TAC
    constexpr std::array g_frequency_bit_mask = {uint16_t(1) << 9, uint16_t(1) << 3, uint16_t(1) << 5,
                                                 uint16_t(1) << 7};

    class Timer {
      public:
        Timer(InterruptRegister &interrupt_flags) : interrupt_flags_(interrupt_flags) { reset(); }

        void update();
        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data);

        void reset();

      private:
        uint16_t counter_ = 0xABCC;

        uint8_t TIMA_ = 0;
        uint8_t TMA_ = 0;

        struct {
            uint8_t freqency = 0;
            bool enable = false;
        } TAC_;

        bool frequency_bit_was_set_ = false;
        InterruptRegister &interrupt_flags_;
    };
} // namespace gb

#endif
