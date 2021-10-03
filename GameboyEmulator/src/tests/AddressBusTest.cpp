#include "gb/AddressBus.h"
#include "gb/memory/Memory.h"
#include "Common.h"

#include <cassert>
#include <cstdint>

class DummyMemObject : public gb::MemoryObject
{
public:
    uint8_t read(uint16_t /*address*/) const override
    {
        return value;
    }

    void write(uint16_t /*address*/, uint8_t val) override
    {
        value = val;
    }

    uint8_t value;
};

void TestConnecting()
{
    gb::AddressBus bus;
    DummyMemObject obj;

    bus.connect({1, 2, obj});

    assert(bus.getObjectCount() == 1);
}

void TestReading()
{
    gb::AddressBus bus;
    DummyMemObject obj1;
    DummyMemObject obj2;
    DummyMemObject obj3;

    bus.connect({1, 2, obj1});
    bus.connect({4, 6, obj2});
    bus.connect({7, 7, obj3});

    obj1.value = 1;
    obj2.value = 3;
    obj3.value = 8;

    //Edge cases
    assert(bus.read(1) == 1);
    assert(bus.read(2) == 1);
    assert(bus.read(7) == 8);

    //Common case
    assert(bus.read(5) == 3);
}

void TestWriting()
{
    gb::AddressBus bus;
    DummyMemObject obj1;
    DummyMemObject obj2;
    DummyMemObject obj3;

    bus.connect({1, 2, obj1});
    bus.connect({4, 6, obj2});
    bus.connect({7, 7, obj3});

    bus.write(1, 3);
    assert(obj1.value == 3);
    bus.write(2, 4);
    assert(obj1.value == 4);
    bus.write(7, 1);
    assert(obj3.value == 1);

    bus.write(5, 6);
    assert(obj2.value == 6);
}

void TestExceptions()
{
    gb::AddressBus bus;

    try
    {
        bus.read(1);
        assert(false);
    }
    catch(const std::exception&)
    {
        
    }

    try
    {
        bus.write(10, 1);
        assert(false);
    }
    catch(const std::exception&)
    {
        
    }

    DummyMemObject obj;

    bus.connect({1, 6, obj});

    try
    {
        bus.read(7);
        assert(false);
    }
    catch(const std::exception&)
    {
        
    }

    try
    {
        bus.write(7, 1);
        assert(false);
    }
    catch(const std::exception&)
    {
        
    }
    
}


int main()
{
    RUN_TEST(TestConnecting);
    RUN_TEST(TestReading);
    RUN_TEST(TestWriting);
    RUN_TEST(TestExceptions);
    return 0;
}