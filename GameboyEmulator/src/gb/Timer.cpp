#include "Timer.h"

#include <limits>
#include <stdexcept>
#include <string>

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
        case 0x00:
            return DIV_;
        case 0x01:
            return TIMA_;
        case 0x02:
            return TMA_;
        case 0x03:
            return (uint8_t(TAC_.enable) << 2) + TAC_.freqency;
        default:
            throw std::invalid_argument("Attempting to read data from timer at invalid adress: " + std::to_string(address));
            return 0x00;
        }
    }
    void Timer::write(uint16_t address, uint8_t data)
    {
        switch (address)
        {
        case 0x00:
            counter_ = 0;
            break;
        case 0x01:
            TIMA_ = data;
            break;
        case 0x02:
            TMA_ = data;
            break;
        case 0x03:
            TAC_.enable = (data & 0b00000100) != 0;
            TAC_.freqency = data & 0b00000011;
            break;
        default:
            throw std::invalid_argument("Attempting to write data to timer at invalid adress: " + std::to_string(address) + ", data: " + std::to_string(+data));
        }
    }
}
