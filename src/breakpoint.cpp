#include "breakpoint.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

namespace emulator {

    void MemoryBreakpoints::removeBreakpoint(MemoryBreakpointData breakpoint) {
        bool found = false;
        auto it = std::lower_bound(breakpoints_.begin(), breakpoints_.end(), breakpoint);
        for (; !(it == breakpoints_.end() || breakpoint < *it); ++it) {
            if (breakpoint == *it) {
                found = true;
                break;
            }
        }
        if (found) {
            breakpoints_.erase(it);
        }
    }

    void MemoryBreakpoints::onRead(uint16_t address, uint8_t data) noexcept {
        if (breakpoints_.empty()) {
            return;
        }
        bool found = false;
        auto it = std::lower_bound(breakpoints_.begin(), breakpoints_.end(),
                                   MemoryBreakpointData{.address = address,
                                                        .break_on = MemoryBreakpointData::BreakOn::READ});
        for (;
             it != breakpoints_.end() && address == it->address && it->break_on != MemoryBreakpointData::BreakOn::WRITE;
             ++it) {
            if (!it->value || *it->value == data) {
                found = true;
                break;
            }
        }

        if (found) {
            callback_();
        }
    }

    void MemoryBreakpoints::onWrite(uint16_t address, uint8_t data) noexcept {
        if (breakpoints_.empty()) {
            return;
        }
        bool found = false;
        auto it = std::lower_bound(breakpoints_.begin(), breakpoints_.end(),
                                   MemoryBreakpointData{.address = address,
                                                        .break_on = MemoryBreakpointData::BreakOn::ALWAYS});
        for (; it != breakpoints_.end() && address == it->address; ++it) {
            if (!it->value || *it->value == data) {
                found = true;
                break;
            }
        }

        if (found) {
            callback_();
        }
    }
} // namespace emulator
