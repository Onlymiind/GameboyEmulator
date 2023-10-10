#pragma once
#include "gb/memory/Memory.h"

#include <cstdint>
#include <optional>

namespace emulator {
    struct AddressBreakpoint {
        uint16_t address = 0;
    };

    enum class MemoryBreakpointFlags : uint8_t{
        READ = 1,
        WRITE = 1 << 1
    };

    class MemoryBreakpoint : public gb::MemoryObject {
    public:
        void read();
    private:
        uint8_t flags_;
        std::optional<uint8_t> value_;
    };
}
