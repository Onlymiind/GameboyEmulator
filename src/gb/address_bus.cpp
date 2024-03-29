#include "gb/address_bus.h"
#include "gb/gb_input.h"
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
        if (address <= g_memory_rom.max_address) {
            cartridge_.writeROM(address, data);
        } else if (address <= g_memory_vram.max_address) {
            ppu_.writeVRAM(address, data);
        } else if (address <= g_memory_cartridge_ram.max_address) {
            cartridge_.writeRAM(address, data);
        } else if (address <= g_memory_wram.max_address) {
            wram_[address - g_memory_wram.min_address] = data;
        } else if (address <= g_memory_mirror.max_address) {
            // only lower 13 bits of the address are used
            wram_[address & 0x1fff] = data;
        } else if (address <= g_memory_oam.max_address) {
            ppu_.writeOAM(address, data);
        } else if (address <= g_memory_forbidden.max_address) {
            // ignore
        } else if (address == uint16_t(IO::JOYPAD)) {
            input_.write(data);
        } else if (g_memory_timer.isInRange(address)) {
            timer_.write(address, data);
        } else if (g_memory_ppu_registers.isInRange(address)) {
            ppu_.writeIO(address, data);
        } else if (address == uint16_t(IO::IE)) {
            interrupt_enable_.write(data);
        } else if (address == uint16_t(IO::IF)) {
            interrupt_flags_.write(data);
        } else if (address <= g_memory_io_unused.max_address) {
            // catch all reads from io range
            unused_io_[address - g_memory_io_unused.min_address] = data;
        } else if (address <= g_memory_hram.max_address) {
            hram_[address - g_memory_hram.min_address] = data;
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
        memset(ptr, 0xff, unused_io_.data() + unused_io_.size() - ptr);
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
        if (address <= g_memory_rom.max_address) {
            return cartridge_.readROM(address);
        } else if (address <= g_memory_vram.max_address) {
            return ppu_.readVRAM(address);
        } else if (address <= g_memory_cartridge_ram.max_address) {
            return cartridge_.readRAM(address);
        } else if (address <= g_memory_wram.max_address) {
            return wram_[address - g_memory_wram.min_address];
        } else if (address <= g_memory_mirror.max_address) {
            // only lower 13 bits of the address are used
            return wram_[address & 0x1fff];
        } else if (address <= g_memory_oam.max_address) {
            return ppu_.readOAM(address);
        } else if (address <= g_memory_forbidden.max_address) {
            // TODO: value depends on PPU behaviour
            return 0xff;
        } else if (address == uint16_t(IO::JOYPAD)) {
            return input_.read();
        } else if (g_memory_timer.isInRange(address)) {
            return timer_.read(address);
        } else if (g_memory_ppu_registers.isInRange(address)) {
            return ppu_.readIO(address);
        } else if (address == uint16_t(IO::IE)) {
            return interrupt_enable_.read();
        } else if (address == uint16_t(IO::IF)) {
            return interrupt_flags_.read();
        } else if (address <= g_memory_io_unused.max_address) {
            // catch all reads from io range
            return unused_io_[address - g_memory_io_unused.min_address];
        } else if (address <= g_memory_hram.max_address) {
            return hram_[address - g_memory_hram.min_address];
        }

        return {};
    }

} // namespace gb
