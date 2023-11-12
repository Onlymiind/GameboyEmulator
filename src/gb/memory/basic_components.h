#ifndef GB_EMULATOR_SRC_GB_MEMORY_BASIC_COMPONENTS_HDR_
#define GB_EMULATOR_SRC_GB_MEMORY_BASIC_COMPONENTS_HDR_

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

namespace gb {

    // TODO: cartridge RAM, mapper chips
    class MemoryBankController {
      public:
        MemoryBankController() = default;
        virtual ~MemoryBankController() = default;

        virtual void write(uint16_t address, uint8_t value) = 0;
        virtual size_t getEffectiveROMAddress(uint16_t address) const = 0;
        virtual size_t getEffectiveRAMAddress(uint16_t address) const = 0;
        virtual bool ramEnabled() const = 0;
    };

    constexpr size_t getAddressMask(size_t size) {
        return size_t(-1) >> (std::numeric_limits<size_t>{}.digits - std::bit_width(size - 1));
    }

    class MBC1 : public MemoryBankController {
      public:
        MBC1(size_t rom_size, size_t ram_size)
            : rom_address_mask_(getAddressMask(rom_size)), ram_address_mask_(getAddressMask(ram_size)) {}
        ~MBC1() override = default;

        void write(uint16_t address, uint8_t value) override;
        size_t getEffectiveROMAddress(uint16_t address) const override;
        size_t getEffectiveRAMAddress(uint16_t address) const override {
            return ((mode_ ? (size_t(ram_bank_) << 13) : 0) | size_t(address & 0xfff)) & ram_address_mask_;
        }
        bool ramEnabled() const override { return ram_enabled_; }

      private:
        size_t rom_address_mask_ = 0;
        size_t ram_address_mask_ = 0;
        uint8_t rom_bank_ = 1;
        uint8_t ram_bank_ = 0;
        bool mode_ = false;
        bool ram_enabled_ = false;
    };

    constexpr uint16_t g_mapper_type_address = 0x147;
    constexpr uint16_t g_rom_size_address = 0x148;
    constexpr uint16_t g_cartridge_ram_size_address = 0x149;

    struct MemoryObjectInfo {
        uint16_t min_address = 0;
        uint16_t max_address = 0;
        uint16_t size = static_cast<uint16_t>(max_address - min_address + 1);

        constexpr bool isInRange(uint16_t address) const { return address >= min_address && address <= max_address; }
    };

    constexpr MemoryObjectInfo g_memory_rom = {.min_address = 0x0000, .max_address = 0x7FFF};
    constexpr MemoryObjectInfo g_memory_cartridge_ram = {.min_address = 0xa000, .max_address = 0xbfff};

    class Cartridge {
      public:
        Cartridge() = default;
        Cartridge(std::vector<uint8_t> rom) : rom_(std::move(rom)) {}

        bool setROM(std::vector<uint8_t> rom);
        uint8_t readROM(uint16_t address) const;
        void writeROM(uint16_t address, uint8_t data);

        uint8_t readRAM(uint16_t address) const;
        void writeRAM(uint16_t address, uint8_t data);

        bool hasRAM() const { return !ram_.empty(); }
        bool hasROM() const { return !rom_.empty(); }

      private:
        std::unique_ptr<MemoryBankController> mbc_;
        std::vector<uint8_t> rom_;
        std::vector<uint8_t> ram_;
    };

    template <size_t SIZE>
    using RAM = std::array<uint8_t, SIZE>;
} // namespace gb

#endif
