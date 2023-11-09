#include "gb/memory/basic_components.h"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace gb {
    bool Cartridge::setROM(std::vector<uint8_t> rom) {
        if (rom.size() < 32 * 1024) {
            return false;
        }

        bool has_ram = false;
        size_t rom_size = 32 * 1024 * (1 << size_t(rom[g_rom_size_address]));
        // is this check necessary?
        if (rom_size != rom.size()) {
            return false;
        }

        size_t ram_size = 0;
        switch (rom[g_cartridge_ram_size_address]) {
        case 0: break;
        case 2: ram_size = 8 * 1024; break;
        case 3: ram_size = 32 * 1024; break;
        case 4: ram_size = 128 * 1024; break;
        case 5: ram_size = 64 * 1024; break;
        default: return false;
        }

        switch (rom[g_mapper_type_address]) {
        case 0:
            if (ram_size != 0) {
                return false;
            }
            mbc_ = nullptr;
            break;
        case 1:
            if (ram_size != 0) {
                return false;
            }
            mbc_ = std::make_unique<MBC1>(rom_size, ram_size);
            break;
        case 2: mbc_ = std::make_unique<MBC1>(rom_size, ram_size); break;
        case 3: mbc_ = std::make_unique<MBC1>(rom_size, ram_size); break;
        default: return false; // MBC chip not supported
        }

        rom_ = std::move(rom);
        ram_.resize(ram_size);

        return true;
    }

    uint8_t Cartridge::read(uint16_t address) const {
        if (g_memory_cartridge_ram.isInRange(address)) {
            if (ram_.empty()) {
                throw std::invalid_argument("accessing unmapped cartridge RAM");
            }

            if (mbc_ && mbc_->ramEnabled()) {
                return ram_[mbc_->getEffectiveAddress(address)];
            }
            return 0xff;
        }

        if (mbc_) {
            return rom_[mbc_->getEffectiveAddress(address)];
        }

        return rom_[address];
    }
    void Cartridge::write(uint16_t address, uint8_t data) {
        if (!mbc_) {
            return;
        }
        // it seems that MBC chips' registers are always in ROM,
        // thus write to MBC and to cartridge RAM never occur on the same write
        mbc_->write(address, data);

        if (!ram_.empty() && mbc_->ramEnabled()) {
            ram_[mbc_->getEffectiveAddress(address)] = data;
        }
    }

    void MBC1::write(uint16_t address, uint8_t value) {
        if (address <= 0x1fff) {
            ram_enabled_ = (value & 0xa) == 0xa;
        } else if (address <= 0x3fff) {
            rom_bank_ = value & 0b11111;
            if (rom_bank_ == 0) {
                rom_bank_ = 1;
            }
        } else if (address <= 0x5fff) {
            ram_bank_ = value & 0b11;
        } else if (address <= 0x7fff) {
            mode_ = value & 1;
        }
    }

    size_t MBC1::getEffectiveAddress(uint16_t address) const {
        if (mode_) {
            if (g_memory_cartridge_ram.isInRange(address)) {
                return (size_t(ram_bank_) << 13) | (address & 0xfff);
            } else if (address <= 0x3fff) {
                return (size_t(ram_bank_) << 19) | address;
            }

            return (size_t(ram_bank_) << 19) | (size_t(rom_bank_) << 14) | size_t(address & 0x3fff);
        }

        if (g_memory_cartridge_ram.isInRange(address)) {
            return address & 0xfff;
        }

        if (address <= 0x3fff) {
            return address;
        }

        return (size_t(ram_bank_) << 19) | (size_t(rom_bank_) << 14) | size_t(address & 0x3fff);
    }
} // namespace gb
