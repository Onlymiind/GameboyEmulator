#include "ConsoleInput.h"
#include "Common.h"

#include <cassert>

std::vector<std::string> g_Input = 
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

std::vector<emulator::Command> g_Output =
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
    return lhs.Type == rhs.Type && lhs.Argument == rhs.Argument;
}

void TestParser()
{
    emulator::Parser p;

    for(int i = 0; i < g_Input.size(); ++i)
    {
        assert(p.parse(g_Input[i]) == g_Output[i]);
    }
}


int main()
{
    RUN_TEST(TestParser);
    return 0;
}