#include "gb/AddressBus.h"
#include "gb/memory/Memory.h"

#include "catch2/catch_test_macros.hpp"

#include <cstdint>
#include <stdexcept>

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

TEST_CASE("connecting")
{
    gb::AddressBus bus;
    DummyMemObject obj;

    bus.connect({1, 2, obj});

    REQUIRE(bus.getObjectCount() == 1);
}

TEST_CASE("reading")
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
    REQUIRE(bus.read(1) == 1);
    REQUIRE(bus.read(2) == 1);
    REQUIRE(bus.read(6) == 3);
    REQUIRE(bus.read(7) == 8);

    //Common case
    REQUIRE(bus.read(5) == 3);
}

TEST_CASE("writing")
{
    gb::AddressBus bus;
    DummyMemObject obj1;
    DummyMemObject obj2;
    DummyMemObject obj3;

    bus.connect({1, 2, obj1});
    bus.connect({4, 6, obj2});
    bus.connect({7, 7, obj3});

    bus.write(1, 3);
    REQUIRE(obj1.value == 3);
    bus.write(2, 4);
    REQUIRE(obj1.value == 4);
    bus.write(7, 1);
    REQUIRE(obj3.value == 1);

    bus.write(5, 6);
    REQUIRE(obj2.value == 6);
}

TEST_CASE("exceptions")
{
    gb::AddressBus bus;

    REQUIRE_THROWS_AS(bus.read(1), std::out_of_range);
    REQUIRE_THROWS_AS(bus.write(10, 1), std::out_of_range);

    DummyMemObject obj;

    bus.connect({1, 6, obj});

    REQUIRE_THROWS_AS(bus.read(7), std::out_of_range);
    REQUIRE_THROWS_AS(bus.write(7, 1), std::out_of_range);    
}

