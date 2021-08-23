#include "Common.h"

#include <limits>
#include <cstddef>
#include <cassert>

template<typename T>
bool carryOccured(T lhs, T rhs, bool substract = false)
{
    uint32_t a(lhs), b(rhs);
    if(substract)
    {
        return a < b;
    }
    else
    {
        return (a + b) > std::numeric_limits<T>::max();
    }
}

bool carryOccuredOld(uint8_t lhs, uint8_t rhs)
{
    uint16_t lhs16{ lhs }, rhs16{ rhs };

    return (lhs16 + rhs16) > 0x00FF;
}

//bool carryOccuredSubOld(uint8_t lhs, )

void TestAddByte()
{
    uint8_t lhs, rhs;
    lhs = 0;
    rhs = 0;

    assert(carryOccured(lhs, rhs) == carryOccuredOld(lhs, rhs));

    lhs = 0xFF;
    rhs = 1;

    assert(carryOccured(lhs, rhs) == carryOccuredOld(lhs, rhs));

    lhs = 0xF0;
    rhs = 0x10;

    assert(carryOccured(lhs, rhs) == carryOccuredOld(lhs, rhs));
}

void TestSubByte()
{

}

int main()
{
    RUN_TEST(TestAddByte);
    return 0;
}