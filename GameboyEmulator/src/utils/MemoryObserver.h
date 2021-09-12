#pragma once
#include "gb/MemoryObject.h"

#include <cstdint>
#include <utility>

namespace emulator
{
    enum class MemoryType
    {
        ROM, WRAM, HRAM, IO
    };

    class MemoryObserver : public gb::MemoryObject
    {
    public:
        MemoryObserver() = default;
        
        void setMemory(gb::MemoryObject& memory)
        {
            memory_ = &memory;
        }
    protected:
        gb::MemoryObject* memory_ = nullptr;
    };

}