#include "gb/cpu/CPUUtils.h"

namespace gb
{
    namespace cpu
    {
        uint8_t& WordRegister::getHight()
        {
            return high_;
        }
        uint8_t& WordRegister::getLow()
        {
            return low_;
        }

        uint16_t WordRegister::value() const
        {
            return pack();
        }

        WordRegister& WordRegister::operator+=(uint16_t value)
        {
            uint16_t current = pack();
            current += value;
            unpack(current);
            return *this;
        }

        WordRegister& WordRegister::operator-=(uint16_t value)
        {
            uint16_t current = pack();
            current -= value;
            unpack(current);
            return *this;
        }

        WordRegister& WordRegister::operator=(uint16_t value)
        {
            low_ = value & 0x00FF;
            high_ = (value & 0xFF00) >> 8;
            return *this;
        }

        WordRegister WordRegister::operator--()
        {
            unpack(pack() - 1);
            return *this;
        }

        WordRegister WordRegister::operator++()
        {
            unpack(pack() + 1);
            return *this;
        }

        WordRegister::operator bool()
        {
            return pack() != 0;
        }

        WordRegister::operator uint16_t() const
        {
            return pack();
        }

        uint16_t WordRegister::pack() const
        {
            return (high_ << 8) + low_;
        }

        void WordRegister::unpack(uint16_t value)
        {
            high_ = (value & 0xFF00) >> 8;
            low_ = value & 0x00FF;
        }
    }
}
