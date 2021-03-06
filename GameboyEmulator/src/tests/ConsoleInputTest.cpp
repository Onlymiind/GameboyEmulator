#include "ConsoleInput.h"
#include "Common.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> input = 
{
    "-help",
    "   -help",
    "-ls  ",
    "  -quit  ",
    "-ls arg",
    "",
    "     ",
    "kjl",
    "-romdir arg",
    "-run      arg    ",
    "-romdir",
    "-run "
};

std::vector<emulator::Command> output =
{
    {emulator::CommandType::Help, {}},
    {emulator::CommandType::Help, {}},
    {emulator::CommandType::List, {}},
    {emulator::CommandType::Quit, {}},
    {emulator::CommandType::List, {}},
    {emulator::CommandType::None, {}},
    {emulator::CommandType::None, {}},
    {emulator::CommandType::Invalid, {}},
    {emulator::CommandType::SetRomDir, "arg"},
    {emulator::CommandType::RunRom, "arg"},
    {emulator::CommandType::Invalid, {}},
    {emulator::CommandType::Invalid, {}}
};

bool operator==(const emulator::Command& lhs, const emulator::Command& rhs)
{
    return lhs.type == rhs.type && lhs.argument == rhs.argument;
}

void TestParser()
{
    emulator::Reader r(std::cin);

    for(int i = 0; i < input.size(); ++i)
    {
        assert(r.parse(input[i]) == output[i]);
    }
}


int main()
{
    RUN_TEST(TestParser);
    std::cin.get();
    return 0;
}