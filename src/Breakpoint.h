#pragma once
#include "gb/AddressBus.h"
#include "gb/Emulator.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>

namespace emulator {

    class Breakpoint {
      public:
        Breakpoint() = default;

        virtual void update(){};

        virtual void reset(){};

        virtual bool isHit() = 0;
    };

    enum class MemoryBreakpointFlags : uint8_t { READ = 1, WRITE = 1 << 1 };

    class MemoryBreakpoint : public Breakpoint, public gb::MemoryObserver {
      public:
        MemoryBreakpoint(uint16_t min_address, uint16_t max_address,
                         uint8_t flags = uint8_t(MemoryBreakpointFlags::READ) |
                                         uint8_t(MemoryBreakpointFlags::WRITE),
                         std::optional<uint8_t> value = {})
            : min_address_(min_address), max_address_(max_address), flags_(flags), value_(value) {}

        void onRead(uint16_t address, uint8_t data) override {
            if (!(flags_ & uint8_t(MemoryBreakpointFlags::READ)) | is_hit_) {
                return;
            }

            if (value_) {
                is_hit_ = *value_ == data;
            } else {
                is_hit_ = true;
            }
        }

        void onWrite(uint16_t address, uint8_t data) override {
            if (!(flags_ & uint8_t(MemoryBreakpointFlags::WRITE)) | is_hit_) {
                return;
            }

            if (value_) {
                is_hit_ = *value_ == data;
            } else {
                is_hit_ = true;
            }
        }

        bool isHit() override { return is_hit_; }

        void reset() override { is_hit_ = false; }

        uint16_t minAddress() const override { return min_address_; }
        uint16_t maxAddress() const override { return max_address_; }
        std::optional<uint8_t> getValue() const { return value_; }
        uint8_t getFlags() const { return flags_; }

      private:
        uint16_t min_address_ = 0;
        uint16_t max_address_ = 0;
        uint8_t flags_;
        std::optional<uint8_t> value_;
        bool is_hit_ = false;
    };

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

    class MemoryBreakpoints : public gb::MemoryObserver {
      public:
        MemoryBreakpoints(std::function<void()> &&callback) : callback_(std::move(callback)) {}

        void addBreakpoint(MemoryBreakpointData breakpoint) {
            breakpoints_.insert(
                std::lower_bound(breakpoints_.begin(), breakpoints_.end(), breakpoint), breakpoint);
        }

        void removeBreakpoint(MemoryBreakpointData breakpoint);

        const std::vector<MemoryBreakpointData> &getBreakpoints() const { return breakpoints_; }

        void onRead(uint16_t address, uint8_t data) override;
        void onWrite(uint16_t address, uint8_t data) override;

        uint16_t minAddress() const override { return 0; }
        uint16_t maxAddress() const override { return uint16_t(-1); }

      private:
        std::function<void()> callback_;
        std::vector<MemoryBreakpointData> breakpoints_;
    };
} // namespace emulator
