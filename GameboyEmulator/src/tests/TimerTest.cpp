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

    assert(timer.read(0x00) == 0xAB);
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
    timer.write(0x00, 0);
    timer.write(0x03, 4);



    //This should cause timer with default register values to set interrupt bit
    for(int j = 256; j > 0; --j)
    {
        for(int i = 1024; i > 0; --i)
        {
            timer.update();
        }
    }

    assert((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
}

void TestFrequencies()
{
    {
        gb::InterruptRegister reg;
        gb::Timer timer(reg);

        //Enable timer, set frequency to the first one
        timer.write(0x00, 0);
        timer.write(0x03, 4);

        for(int i = 1024 * 256 - 1; i > 0; --i)
        {
            timer.update();
            assert(!(reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
        }

        timer.update();
        assert((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
    }
    {
        gb::InterruptRegister reg;
        gb::Timer timer(reg);

        //Enable timer, set second frequency
        timer.write(0x00, 0);
        timer.write(0x03, 5);

        for(int i = 16 * 256 - 1; i > 0; --i)
        {
            timer.update();
            assert(!(reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
        }

        timer.update();
        assert((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
    }
    {
        gb::InterruptRegister reg;
        gb::Timer timer(reg);

        //Enable timer, set third frequency
        timer.write(0x00, 0);
        timer.write(0x03, 6);

        for(int i = 64 * 256 - 1; i > 0; --i)
        {
            timer.update();
            assert(!(reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
        }

        timer.update();
        assert((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
    }
    {
        gb::InterruptRegister reg;
        gb::Timer timer(reg);

        //Enable timer, set last frequency
        timer.write(0x00, 0);
        timer.write(0x03, 7);

        for(int i = 256 * 256 - 1; i > 0; --i)
        {
            timer.update();
            assert(!(reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
        }

        timer.update();
        assert((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::Timer)));
    }
}

int main()
{
    RUN_TEST(TestReadWrite);
    RUN_TEST(TestInterrupt);
    RUN_TEST(TestFrequencies);
    std::cin.get();
    return 0;
}