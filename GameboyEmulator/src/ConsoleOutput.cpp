#include "ConsoleOutput.h"
#include <iomanip>

#define FMT_HEX(x) "0x" << std::setfill('0') << std::setw(sizeof(x) * 2) << std::hex << +x

namespace emulator
{
    void Printer::printTitle() const
    {
        output_ << R"(
****************************
*  Welcome to GB emulator  *
****************************
            )" << '\n';
    }

    void Printer::printHelp() const
    {
        output_ << R"(
            -help - show this text
            -romdir <path> -set ROM directory
            -run <name> -run a ROM
            -ls - list all ROMs in current directory
            -config - show current ROM directory
            -quit - quit the emulator)" << '\n';
    }

    void Printer::print(std::string_view text) const
    {
        output_ << text << '\n';
    }

    void Printer::reportError(InputError err) const
    {
        switch(err)
        {
            case InputError::InvalidCommand:
                output_ << "Invalid command. Use -help for command list";
                break;
            case InputError::InvalidDirectory:
                output_ << "Selected directory doesn't exist";
                break;
            case InputError::InvalidRomName:
                output_ << "ROM doesn't exist in the specified directory";
                break;
        }

        output_ << '\n';
    }

    void Printer::reportError(std::string_view description, uint16_t program_counter) const
    {
        output_ << "Emulation terminated due to runtime error.\n"
            << "Error description: " << description << '\n'
            << "CPU program counter: " << FMT_HEX(program_counter) << '\n';
    }
}