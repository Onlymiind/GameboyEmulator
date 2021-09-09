#pragma once
#include "ConsoleInput.h"

#include <iostream>

namespace emulator
{
    class Printer
    {
    public:
        Printer(std::ostream& output)
            : output_(output)
        {}

        template<typename T>
        void fmtHex();
    private:
        std::ostream& output_;
    };
}