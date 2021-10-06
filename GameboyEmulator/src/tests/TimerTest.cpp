#include "Common.h"
#include "gb/Timer.h"
#include "gb/InterruptRegister.h"

#include <cassert>
#include <iostream>

//TODO: write tests for timer bugs

void TestReadWrite()
{
    gb::InterruptRegister reg;
    gb::Timer timer(reg);

    assert(timer.read(0x00) == 0xCC);
    assert(timer.read(0x01) == 0);
    assert(timer.read(0x02) == 0);
    assert(timer.read(0x03) == 0);

    timer.write(0x00, 0x10);
    assert(timer.read(0x00) == 0);

    timer.write(0x01, 0x11);
    assert(timer.read(0x01) == 0x11);

    timer.write(0x02, 0x12);
    assert(timer.read(0x02) == 0x12);

    timer.write(0x03, 0b11100101);
    assert(timer.read(0x03) == 0b00000101);
}

void TestInterrupt()
{
    gb::InterruptRegister reg;
    gb::Timer timer(reg);



    //This should cause timer with default register values to set interrupt bit
    for(int i = 1024; i > 0; --i)
    {
        timer.update();
    }

    //DIV is the upper 8 bits of internal counter, so it should equal 4
    assert(timer.read(0x00) == 4);
    assert((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
}

void TestFrequencies()
{
    //TODO
}

int main()
{
    RUN_TEST(TestReadWrite);
    RUN_TEST(TestInterrupt);
    RUN_TEST(TestFrequencies);
    std::cin.get();
    return 0;
}