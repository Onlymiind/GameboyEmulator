#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gb {

    // TODO: cartridge RAM, mapper chips
    class Cartridge {
      public:
        Cartridge() = default;
        Cartridge(std::vector<uint8_t> rom) : rom_(std::move(rom)) {}

        void setROM(std::vector<uint8_t> rom) { rom_ = std::move(rom); }
        uint8_t read(uint16_t address) const { return rom_[address]; }
        void write(uint16_t address, uint8_t data) { /*do nothing*/
        }

        bool hasRAM() const { return !ram_.empty(); }
        bool hasROM() const { return !rom_.empty(); }

      private:
        std::vector<uint8_t> rom_;
        std::vector<uint8_t> ram_;
    };

    template <size_t SIZE> using RAM = std::array<uint8_t, SIZE>;

    struct MemoryObjectInfo {
        uint16_t min_address, max_address = 0;
        uint16_t size = static_cast<uint16_t>(max_address - min_address + 1);

        constexpr bool isInRange(uint16_t address) const {
            return address >= min_address && address <= max_address;
        }
    };
} // namespace gb
