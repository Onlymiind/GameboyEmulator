#include "Timer.h"

#include <limits>

namespace gb
{
    void Timer::update()
    {
        ++counter_;
        bool currentFreqBit = (counter_ & frequency_bit_mask_[TAC_.freqency]);
        if (frequency_bit_was_set_ && !(currentFreqBit && TAC_.enable)) //Falling edge
        {
            if(TIMA_ == std::numeric_limits<uint8_t>::max())
            {
                TIMA_ = TMA_;
                interrupt_flags_.setFlag(InterruptFlags::Timer);
            }
            else
            {
                ++TIMA_;
            }
        }
        frequency_bit_was_set_ = currentFreqBit && TAC_.enable;
    }
    uint8_t Timer::read(uint16_t address) const
    {
        switch (address)
        {
        case 0xFF04: return DIV_;
        case 0xFF05: return TIMA_;
        case 0xFF06: return TMA_;
        case 0xFF07: return (uint8_t(TAC_.enable) << 2) + TAC_.freqency;
        }
    }
    void Timer::write(uint16_t address, uint8_t data)
    {
        switch (address) {
        case 0xFF04:
        {
            counter_ = 0;
            break;
        }
        case 0xFF05:
        {
            TIMA_ = data;
            break;
        }
        case 0xFF06:
        {
            TMA_ = data;
            break;
        }
        case 0xFF07:
        {
            TAC_.enable = (data & 0b00000100) != 0;
            TAC_.freqency = data & 0b00000011;
            break;
        }
        }
    }
}
