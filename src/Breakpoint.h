#pragma once
#include "gb/AddressBus.h"
#include "gb/Emulator.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>

namespace emulator {

    struct MemoryBreakpointData {
        enum class BreakOn : uint8_t { ALWAYS = 1, READ = 0, WRITE = 2 };

        uint16_t address = 0;

        BreakOn break_on = BreakOn::ALWAYS;

        std::optional<uint8_t> value;

        bool operator<(MemoryBreakpointData other) const {
            if (address == other.address) {
                return break_on < other.break_on;
            }
            return address < other.address;
        }

        bool operator==(MemoryBreakpointData other) const {
            return address == other.address && break_on == other.break_on && value == other.value;
        }
    };

    constexpr std::string_view to_string(MemoryBreakpointData::BreakOn mode) {
        using enum MemoryBreakpointData::BreakOn;
        switch (mode) {
        case ALWAYS: return "Always";
        case READ: return "Read";
        case WRITE: return "Write";
        default: return "";
        }
    }

    class MemoryBreakpoints : public gb::IMemoryObserver {
      public:
        MemoryBreakpoints(std::function<void()> &&callback) : callback_(std::move(callback)) {}

        void addBreakpoint(MemoryBreakpointData breakpoint) {
            breakpoints_.insert(std::lower_bound(breakpoints_.begin(), breakpoints_.end(), breakpoint), breakpoint);
        }

        void removeBreakpoint(MemoryBreakpointData breakpoint);

        const std::vector<MemoryBreakpointData> &getBreakpoints() const { return breakpoints_; }

        void onRead(uint16_t address, uint8_t data) noexcept override;
        void onWrite(uint16_t address, uint8_t data) noexcept override;

        uint16_t minAddress() const noexcept override { return 0; }
        uint16_t maxAddress() const noexcept override { return uint16_t(-1); }

      private:
        std::function<void()> callback_;
        std::vector<MemoryBreakpointData> breakpoints_;
    };
} // namespace emulator
