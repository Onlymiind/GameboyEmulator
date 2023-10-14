#pragma once
#include "gb/memory/Memory.h"
#include "gb/Emulator.h"

#include <cstdint>
#include <optional>

namespace emulator {

    class Breakpoint {
    public:
        Breakpoint() = default;

        virtual void update() {};

        virtual void reset() {};

        virtual bool isHit() = 0;
    };

    enum class MemoryBreakpointFlags : uint8_t {
        READ = 1,
        WRITE = 1 << 1
    };

    class MemoryBreakpoint : public Breakpoint, public gb::MemoryObject {
    public:
        MemoryBreakpoint(uint16_t min_address, uint16_t max_address,
            uint8_t flags = uint8_t(MemoryBreakpointFlags::READ) | uint8_t(MemoryBreakpointFlags::WRITE), 
            std::optional<uint8_t> value = {})
            : min_address_(min_address), max_address_(max_address), flags_(flags), value_(value)
        {}

        void onRead(uint16_t address, uint8_t data) override {
            if(!(flags_ & uint8_t(MemoryBreakpointFlags::READ)) | is_hit_) {
                return;
            }

            if(value_) {
                is_hit_ = *value_ == data;
            } else {
                is_hit_ = true;
            }
        }

        void onWrite(uint16_t address, uint8_t data) override {
            if(!(flags_ & uint8_t(MemoryBreakpointFlags::WRITE)) | is_hit_) {
                return;
            }

            if(value_) {
                is_hit_ = *value_ == data;
            } else {
                is_hit_ = true;
            }
        }

        bool isHit() override { return is_hit_; }

        void reset() override { is_hit_ = false; }

        uint16_t getMinAddress() const { return min_address_; }
        uint16_t getMaxAddress() const { return max_address_; }
        std::optional<uint8_t> getValue() const { return value_; }
        uint8_t getFlags() const { return flags_; }

    private:
        uint16_t min_address_ = 0;
        uint16_t max_address_ = 0;
        uint8_t flags_;
        std::optional<uint8_t> value_;
        bool is_hit_ = false;
    };
}
