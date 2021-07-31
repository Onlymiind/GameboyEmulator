#pragma once
#include <string>
#include <iostream>

template<typename pfn_Test>
void RunTest(pfn_Test test, std::string name)
{
    std::cout << "Running " << name << std::endl;
    test();
    std::cout << "OK" << std::endl;
}

#define RUN_TEST(test) RunTest(test, #test)