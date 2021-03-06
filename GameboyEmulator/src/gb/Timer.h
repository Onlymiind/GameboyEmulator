#pragma once

#include "gb/InterruptRegister.h"
#include "gb/memory/Memory.h"
#include "gb/cpu/CPUUtils.h"

#include <cstdint>
#include <array>

namespace gb
{
    class Timer : public MemoryObject
    {
    public:
        Timer(InterruptRegister& interruptFlags)
            : interrupt_flags_(interruptFlags)
        {}

        void update();
        uint8_t read(uint16_t address) const override;
        void write(uint16_t address, uint8_t data) override;

    private:
        uint8_t DIV_ = 0xAB;
        uint8_t align_ = 0xCC;
        cpu::Word counter_ = cpu::Word(DIV_, align_);

        uint8_t TIMA_ = 0;
        uint8_t TMA_ = 0;

        struct 
        {
            uint8_t freqency = 0;
            bool enable = false;
        } TAC_;

        bool frequency_bit_was_set_ = false;

        //Frequencies of a timer: 4096 Hz, 262144 Hz, 65536 Hz and 16386 Hz respectively. Frequency is set from bits 0-1 of TAC
        const std::array<uint16_t, 4> frequency_bit_mask_ = { 1 << 9, 1 << 3, 1 << 5, 1 << 7 };
        InterruptRegister& interrupt_flags_;
    };
}