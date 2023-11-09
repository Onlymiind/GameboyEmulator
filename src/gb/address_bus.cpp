#include "gb/address_bus.h"
#include "gb/interrupt_register.h"
#include "gb/memory/basic_components.h"
#include "gb/ppu/ppu.h"
#include "util/util.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace gb {

    bool less(const IMemoryObserver *mem, uint16_t address) { return mem->maxAddress() < address; }

    uint8_t AddressBus::read(uint16_t address) const {
        std::optional<uint8_t> data = peek(address);

        if (!data) {
            throw std::invalid_argument("trying to read from invalid address");
        }

        if (observer_) {
            observer_->onRead(address, *data);
        }

        return *data;
    }

    void AddressBus::write(uint16_t address, uint8_t data) {
        if (g_memory_rom.isInRange(address) && cartridge_.hasROM()) {
            cartridge_.write(address, data);
        } else if (g_memory_vram.isInRange(address)) {
            ppu_.write(address, data);
        } else if (g_memory_cartridge_ram.isInRange(address) && cartridge_.hasRAM()) {
            cartridge_.write(address, data);
        } else if (g_memory_wram.isInRange(address)) {
            wram_[address - g_memory_wram.min_address] = data;
        } else if (g_memory_mirror.isInRange(address)) {
            // only lower 13 bits of the address are used
            wram_[address & 0x1fff] = data;
        } else if (g_memory_oam.isInRange(address)) {
            ppu_.write(address, data);
        } else if (g_memory_timer.isInRange(address)) {
            timer_.write(address, data);
        } else if (g_memory_ppu_registers.isInRange(address)) {
            ppu_.write(address, data);
        } else if (g_memory_hram.isInRange(address)) {
            hram_[address - g_memory_hram.min_address] = data;
        } else if (address == g_interrupt_enable_address) {
            interrupt_enable_.write(data);
        } else if (address == g_interrupt_flags_address) {
            interrupt_flags_.write(data);
        } else if (g_memory_io_unused.isInRange(address)) {
            // catch all writes to io range
            unused_io_[address - g_memory_io_unused.min_address] = data;
        } else if (g_memory_forbidden.isInRange(address)) {
            // ignore
        } else {
            throw std::invalid_argument("trying to access invalid memory");
        }

        if (observer_) {
            observer_->onWrite(address, data);
        }
    }

    void AddressBus::reset() {
        // values set according to mooneye test suite, see accepnance/boot_hwio_dmg0.gb
        // initial values for all implemented hardware are reset by corresponding class
        std::memcpy(unused_io_.data(), g_io_initail_values.data(), g_io_initail_values.size());
        uint8_t *ptr = unused_io_.data() + g_io_initail_values.size();
        memset(ptr, 0xff, unused_io_.end() - ptr);
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

    std::optional<uint8_t> AddressBus::peek(uint16_t address) const {
        if (g_memory_rom.isInRange(address) && cartridge_.hasROM()) {
            return cartridge_.read(address);
        } else if (g_memory_vram.isInRange(address)) {
            return ppu_.read(address);
        } else if (g_memory_cartridge_ram.isInRange(address) && cartridge_.hasRAM()) {
            return cartridge_.read(address);
        } else if (g_memory_wram.isInRange(address)) {
            return wram_[address - g_memory_wram.min_address];
        } else if (g_memory_mirror.isInRange(address)) {
            // only lower 13 bits of the address are used
            return wram_[address & 0x1fff];
        } else if (g_memory_oam.isInRange(address)) {
            return ppu_.read(address);
        } else if (g_memory_forbidden.isInRange(address)) {
            // TODO: value depends on PPU behaviour
            return 0xff;
        } else if (g_memory_timer.isInRange(address)) {
            return timer_.read(address);
        } else if (g_memory_ppu_registers.isInRange(address)) {
            return ppu_.read(address);
        } else if (g_memory_hram.isInRange(address)) {
            return hram_[address - g_memory_hram.min_address];
        } else if (address == g_interrupt_enable_address) {
            return interrupt_enable_.read();
        } else if (address == g_interrupt_flags_address) {
            return interrupt_flags_.read();
        } else if (g_memory_io_unused.isInRange(address)) {
            // catch all reads from io range
            return unused_io_[address - g_memory_io_unused.min_address];
        }

        return {};
    }

} // namespace gb
