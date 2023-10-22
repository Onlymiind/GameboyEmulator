#include "gb/AddressBus.h"
#include "gb/InterruptRegister.h"
#include "utils/Utils.h"

#include <algorithm>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace gb {

    bool less(const MemoryObserver *mem, uint16_t address) {
        return mem->maxAddress() < address;
    }

    uint8_t AddressBus::read(uint16_t address) const {
        uint8_t data = 0;
        if (g_memory_rom.isInRange(address) && cartridge_.hasROM()) {
            data = cartridge_.read(address);
        } else if (g_memory_vram.isInRange(address)) {
            data = vram_[address - g_memory_vram.min_address];
        } else if (g_memory_cartridge_ram.isInRange(address) &&
                   cartridge_.hasRAM()) {
            data = cartridge_.read(address);
        } else if (g_memory_wram.isInRange(address)) {
            data = wram_[address - g_memory_wram.min_address];
        } else if (g_memory_mirror.isInRange(address)) {
            // only lower 13 bits of the address are used
            data = wram_[address & 0x1fff];
        } else if (g_memory_oam.isInRange(address)) {
            data = oam_[address - g_memory_oam.min_address];
        } else if (g_memory_forbidden.isInRange(address)) {
            // TODO: value depends on PPU behaviour
            data = 0xff;
        } else if (g_memory_io_unused.isInRange(address)) {
            data = unused_io_[address - g_memory_io_unused.min_address];
        } else if (g_memory_io_unused2.isInRange(address)) {
            data = unused_io2_[address - g_memory_io_unused2.min_address];
        } else if (g_memory_timer.isInRange(address)) {
            data = timer_.read(address);
        } else if (g_memory_io_unused3.isInRange(address)) {
            data = unused_io3_[address - g_memory_io_unused3.min_address];
        } else if (g_memory_hram.isInRange(address)) {
            data = hram_[address - g_memory_hram.min_address];
        } else if (address == g_interrupt_enable_address) {
            data = interrupt_enable_.read();
        } else if (address == g_interrupt_flags_address) {
            data = interrupt_flags_.read();
        } else {
            throw std::invalid_argument("trying to access invalid memory");
        }

        if (observer_) {
            observer_->onRead(address, data);
        }

        return data;
    }

    void AddressBus::write(uint16_t address, uint8_t data) {
        if (g_memory_rom.isInRange(address) && cartridge_.hasROM()) {
            cartridge_.write(address, data);
        } else if (g_memory_vram.isInRange(address)) {
            vram_[address - g_memory_vram.min_address] = data;
        } else if (g_memory_cartridge_ram.isInRange(address) &&
                   cartridge_.hasRAM()) {
            cartridge_.write(address, data);
        } else if (g_memory_wram.isInRange(address)) {
            wram_[address - g_memory_wram.min_address] = data;
        } else if (g_memory_mirror.isInRange(address)) {
            // only lower 13 bits of the address are used
            wram_[address & 0x1fff] = data;
        } else if (g_memory_oam.isInRange(address)) {
            oam_[address - g_memory_oam.min_address] = data;
        } else if (g_memory_io_unused.isInRange(address)) {
            unused_io_[address - g_memory_io_unused.min_address] = data;
        } else if (g_memory_io_unused2.isInRange(address)) {
            unused_io2_[address - g_memory_io_unused2.min_address] = data;
        } else if (g_memory_timer.isInRange(address)) {
            timer_.write(address, data);
        } else if (g_memory_io_unused3.isInRange(address)) {
            unused_io3_[address - g_memory_io_unused3.min_address] = data;
        } else if (g_memory_hram.isInRange(address)) {
            hram_[address - g_memory_hram.min_address] = data;
        } else if (address == g_interrupt_enable_address) {
            interrupt_enable_.write(data);
        } else if (address == g_interrupt_flags_address) {
            interrupt_flags_.write(data);
        } else if (g_memory_forbidden.isInRange(address)) {
            // ignore
        } else {
            throw std::invalid_argument("trying to access invalid memory");
        }

        if (observer_) {
            observer_->onWrite(address, data);
        }
    }

    std::string AddressBus::getErrorDescription(uint16_t address,
                                                int value) const {
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
} // namespace gb
