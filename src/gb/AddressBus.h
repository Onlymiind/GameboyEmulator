#pragma once
#include "gb/memory/Memory.h"
#include "gb/memory/BasicComponents.h"
#include "gb/InterruptRegister.h"
#include "gb/Timer.h"

#include <cstdint>
#include <string>
#include <vector>


namespace gb 
{
    struct MemoryObjectInfo {
        uint16_t min_address, max_address = 0;
        uint16_t size = static_cast<uint16_t>(max_address - min_address + 1);
    };

    constexpr MemoryObjectInfo g_memory_rom = {0x0000, 0x7FFF};
    constexpr MemoryObjectInfo g_memory_ram = {0x8000, 0xFF03};
    constexpr MemoryObjectInfo g_memory_timer = {0xFF04, 0xFF07};
    constexpr MemoryObjectInfo g_memory_leftover2 = {0xFF08, 0xFF0E};
    constexpr MemoryObjectInfo g_memory_interrupt_enable = {0xFFFF, 0xFFFF};
    constexpr MemoryObjectInfo g_memory_leftover = {0xFF10, 0xFFFE};
    constexpr MemoryObjectInfo g_memory_interrupt_flags = {0xFF0F, 0xFF0F};


    class AddressBus {
    public:
        AddressBus()
        {}

        ~AddressBus() = default;

        void setRomData(std::vector<uint8_t> data) { rom_.setData(std::move(data)); }
        void update() { timer_.update(); }

        void addObserver(MemoryObject& observer);

        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data);

        InterruptRegister& getInterruptFlags() { return interrupt_flags_; }
        InterruptRegister& getInterruptEnable() { return interrupt_enable_; }

        const InterruptRegister& getInterruptFlags() const { return interrupt_flags_; }
        const InterruptRegister& getInterruptEnable() const { return interrupt_enable_; }

    private:
        std::string getErrorDescription(uint16_t address, int value = -1) const;

        std::vector<MemoryObject*> observers_;

        RAM ram_ = RAM(g_memory_ram.size);
        ROM rom_;
        RAM leftover_ = RAM(g_memory_leftover.size);
        RAM leftover2_ = RAM(g_memory_leftover2.size);
        InterruptRegister interrupt_enable_;
        InterruptRegister interrupt_flags_;
        Timer timer_{interrupt_flags_};

        uint8_t data_ = 0;
    };

}
