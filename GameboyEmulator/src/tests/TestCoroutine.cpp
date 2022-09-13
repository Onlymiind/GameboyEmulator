#include "utils/Utils.h"
#include "Common.h"

#include <iostream>
#include <vector>
#include <functional>
#include <cassert>

using TestCoroutine = Coroutine<int, int>;
using Desc = std::vector<std::function<void(int&,int&)>>;

void TestInitialization()
{
    TestCoroutine c(0, Desc{});
    assert(c.IsFinished());
    assert(c.GetResult() == 0);
    assert(c.GetContext() == 0);

    TestCoroutine c1(1, Desc{[](int&, int&){}});
    assert(!c1.IsFinished());
    assert(c1.GetResult() == 0);
    assert(c1.GetContext() == 1);
}

void TestSingleInstruction()
{
    TestCoroutine c(0, Desc{[](int& ctxt, int& res){res = 4; ctxt = 8;}});
    assert(!c.IsFinished());

    c();
    assert(c.IsFinished());
    assert(c.GetContext() == 8);
    assert(c.GetResult() == 4);
}

void TestMultipleInstructions()
{
    Desc instr(4, [](int& ctxt, int& res){++ctxt; res += 2;});

    TestCoroutine c(0, instr);
    
    while(!c.IsFinished())
    {
        c();
    }

    assert(c.GetContext() == 4);
    assert(c.GetResult() == (2 * 4));
}

int main()
{
    RUN_TEST(TestInitialization);
    RUN_TEST(TestSingleInstruction);
    RUN_TEST(TestMultipleInstructions);
    return 0;
}