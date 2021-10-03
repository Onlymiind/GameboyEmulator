#pragma once
#include "Error.h"

#include <iostream>
#include <string_view>
#include <iomanip>

namespace emulator
{
    class Printer
    {
    public:
        Printer(std::ostream& output)
            : output_(output)
        {}

        template<typename T>
        void printHex(T value) const;

        void printTitle() const;
        void printHelp() const;

        void print(std::string_view text) const;
        void println(std::string_view text) const;

        template<typename... Args>
        void print(Args... args) const;

        template<typename... Args>
        void println(Args... args) const;

        void reportError(InputError err) const;
        void reportError(std::string_view description, uint16_t program_counter) const;
    private:
        std::ostream& output_;
    };

    template<typename T>
    void Printer::printHex(T value) const
    {
        output_ << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +value;
    }

    template<typename... Args>
    void Printer::print(Args... args) const
    {
        ((output_ << args), ...);
    }

    template<typename... Args>
    void Printer::println(Args... args) const
    {
        ((output_ << args), ...);
        output_ << '\n';
    }
}