#include "gb/AddressBus.h"
#include "gb/memory/Memory.h"
#include "utils/Utils.h"

#include <string>
#include <sstream>
#include <exception>
#include <algorithm>
#include <iostream>


namespace gb {

    bool less(const MemoryObject* mem, uint16_t address) {
        return mem->maxAddress() < address;
    }

    uint8_t AddressBus::read(uint16_t address) const {
        uint8_t data = 0;
#define ELIF(mem_range, object) else if(address >= mem_range.min_address && address <= mem_range.max_address) { data =  object.read(address - mem_range.min_address); }
        if(address >= g_memory_rom.min_address && address <= g_memory_rom.max_address) {
            data =  rom_.read(address);
        }
        ELIF(g_memory_ram, ram_)
        ELIF(g_memory_leftover2, leftover2_)
        ELIF(g_memory_leftover, leftover_)
        ELIF(g_memory_interrupt_flags, interrupt_flags_)
        ELIF(g_memory_interrupt_enable, interrupt_enable_)
        ELIF(g_memory_timer, timer_)
        else {
            throw std::out_of_range(getErrorDescription(address));
        }
#undef ELIF

        if(!observers_.empty()) {
            auto begin = std::lower_bound(observers_.begin(), observers_.end(), address, less);
            for(;begin != observers_.end() && (*begin)->isInRange(address); ++begin) {
                (*begin)->onRead(address, data);
            }
        }

        return data;
    }

    void AddressBus::write(uint16_t address, uint8_t data) {
        if(!observers_.empty()) {
            auto begin = std::lower_bound(observers_.begin(), observers_.end(), address, less);
            for(;begin != observers_.end() && (*begin)->isInRange(address); ++begin) {
                (*begin)->onWrite(address, data);
            }
        }

#define ELIF(mem_range, object) else if(address >= mem_range.min_address && address <= mem_range.max_address) { object.write(address - mem_range.min_address, data); }
        if(address >= g_memory_rom.min_address && address <= g_memory_rom.max_address) {
            rom_.write(address, data);
        }
        ELIF(g_memory_ram, ram_)
        ELIF(g_memory_leftover2, leftover2_)
        ELIF(g_memory_leftover, leftover_)
        ELIF(g_memory_interrupt_flags, interrupt_flags_)
        ELIF(g_memory_interrupt_enable, interrupt_enable_)
        ELIF(g_memory_timer, timer_)
        else {
            throw std::out_of_range(getErrorDescription(address, int(data)));
        }
#undef ELIF

        if(address == 0xFF01) {
            data_ = data;
        } else if (address == 0xFF02 && data == 0x81) {
            std::cout << data_;
        }

    }

    std::string AddressBus::getErrorDescription(uint16_t address, int value) const {
        std::stringstream err;
        
        if (value == -1) {
            err << "Attempting to read from invalid memory address: ";
            toHexOutput(err, address);
        } else {
            err << "Attempting to write to invalid memory address: ";
            toHexOutput(err, address);
            err << ". Data: ";
            toHexOutput(err, value);
        }

        return err.str();
    }

    void AddressBus::addObserver(MemoryObject& observer) {
        auto it = std::lower_bound(observers_.begin(), observers_.end(), &observer, [&observer](MemoryObject* lhs, MemoryObject* rhs){
            return lhs->maxAddress() < rhs->minAddress();
        });
        observers_.insert(it, &observer);
    }
}
