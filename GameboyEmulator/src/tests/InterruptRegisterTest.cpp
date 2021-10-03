#include "gb/InterruptRegister.h"

#include "Common.h"

#include <cassert>
#include <cstdint>
#include <iostream>


void TestInterruptRegister()
{
    gb::InterruptRegister reg;

    assert(reg.getFlags() == gb::g_unused_interrupt_bits);

    reg.setFlag(gb::InterruptFlags::LCD_STAT);
    assert(reg.read(0x0000) == reg.getFlags());
    assert(reg.getFlags() == (gb::g_unused_interrupt_bits | static_cast<uint8_t>(gb::InterruptFlags::LCD_STAT)));

    reg.clearFlag(gb::InterruptFlags::LCD_STAT);
    assert(reg.getFlags() == gb::g_unused_interrupt_bits);

    reg.clearFlag(gb::InterruptFlags::Joypad);
    assert(reg.getFlags() == gb::g_unused_interrupt_bits);

    reg.write(0x0000, gb::g_unused_interrupt_bits | static_cast<uint8_t>(gb::InterruptFlags::VBlank));
    assert(reg.read(0x0000) == reg.getFlags());
    assert(reg.getFlags() == (gb::g_unused_interrupt_bits | static_cast<uint8_t>(gb::InterruptFlags::VBlank)));

    reg.write(0x0000, 0x00);
    assert(reg.getFlags() == gb::g_unused_interrupt_bits);
}

int main()
{
    RUN_TEST(TestInterruptRegister);
    std::cin.get();
    return 0;
}

