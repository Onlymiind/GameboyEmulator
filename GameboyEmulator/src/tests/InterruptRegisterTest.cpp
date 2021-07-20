#include "core/gb/InterruptRegister.h"

#include <cassert>
#include <cstdint>
#include <iostream>


void TestInterruptRegister()
{
    gb::InterruptRegister reg;

    assert(reg.getFlags() == gb::g_UnusedInterruptBits);

    reg.setFlag(gb::InterruptFlags::LCD_STAT);
    assert(reg.read(0x0000) == reg.getFlags());
    assert(reg.getFlags() == (gb::g_UnusedInterruptBits | static_cast<uint8_t>(gb::InterruptFlags::LCD_STAT)));

    reg.clearFlag(gb::InterruptFlags::LCD_STAT);
    assert(reg.getFlags() == gb::g_UnusedInterruptBits);

    reg.clearFlag(gb::InterruptFlags::Joypad);
    assert(reg.getFlags() == gb::g_UnusedInterruptBits);

    reg.write(0x0000, gb::g_UnusedInterruptBits | static_cast<uint8_t>(gb::InterruptFlags::VBlank));
    assert(reg.read(0x0000) == reg.getFlags());
    assert(reg.getFlags() == (gb::g_UnusedInterruptBits | static_cast<uint8_t>(gb::InterruptFlags::VBlank)));

    reg.write(0x0000, 0x00);
    assert(reg.getFlags() == gb::g_UnusedInterruptBits);
}

int main()
{
    TestInterruptRegister();
    std::cout << "OK\n";
    return 0;
}

