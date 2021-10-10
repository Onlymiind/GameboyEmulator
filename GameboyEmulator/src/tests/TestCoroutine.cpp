#include "utils/Utils.h"
#include "Common.h"

#include <iostream>
#include <vector>
#include <functional>
#include <cassert>

using TestCoroutine = Coroutine<int>;
using Desc = std::vector<std::function<void(int&)>>;

void TestInitialization()
{
    TestCoroutine c(0, Desc{});
    assert(c.IsFinished());
    assert(c.GetContext() == 0);

    TestCoroutine c1(1, Desc{[](int&){}});
    assert(!c1.IsFinished());
    assert(c1.GetContext() == 1);
}

void TestSingleInstruction()
{
    TestCoroutine c(0, Desc{[](int& ctxt){ctxt = 8;}});
    assert(!c.IsFinished());

    c();
    assert(c.IsFinished());
    assert(c.GetContext() == 8);
}

void TestMultipleInstructions()
{
    Desc instr(4, [](int& ctxt){++ctxt;});

    TestCoroutine c(0, instr);
    
    while(!c.IsFinished())
    {
        c();
    }

    assert(c.GetContext() == 4);
}

int main()
{
    RUN_TEST(TestInitialization);
    RUN_TEST(TestSingleInstruction);
    RUN_TEST(TestMultipleInstructions);
    std::cin.get();
    return 0;
}