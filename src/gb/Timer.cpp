#include "Timer.h"

#include <limits>
#include <stdexcept>
#include <string>

namespace gb {
    void Timer::update() {
        ++counter_;
        bool currentFreqBit =
            (counter_ & frequency_bit_mask_[TAC_.freqency]) != 0;
        if (frequency_bit_was_set_ &&
            !(currentFreqBit && TAC_.enable)) { // Falling edge
            ++TIMA_;
            if (TIMA_ == 0) {
                TIMA_ = TMA_;
                interrupt_flags_.setFlag(InterruptFlags::Timer);
            }
        }
        frequency_bit_was_set_ = currentFreqBit && TAC_.enable;
    }

    uint8_t Timer::read(uint16_t address) const {
        switch (address) {
        case g_timer_div_address:
            return DIV_;
        case g_timer_tima_address:
            return TIMA_;
        case g_timer_tma_address:
            return TMA_;
        case g_timer_tac_address:
            return (uint8_t(TAC_.enable) << 2) | TAC_.freqency;
        default:
            throw std::invalid_argument(
                "Attempting to read data from timer at invalid adress: " +
                std::to_string(address));
            return 0x00;
        }
    }

    void Timer::write(uint16_t address, uint8_t data) {
        switch (address) {
        case g_timer_div_address:
            counter_ = 0;
            break;
        case g_timer_tima_address:
            TIMA_ = data;
            break;
        case g_timer_tma_address:
            TMA_ = data;
            break;
        case g_timer_tac_address:
            TAC_.enable = (data & 0b00000100) != 0;
            TAC_.freqency = data & 0b00000011;
            break;
        default:
            throw std::invalid_argument(
                "Attempting to write data to timer at invalid adress: " +
                std::to_string(address) + ", data: " + std::to_string(+data));
        }
    }
} // namespace gb
