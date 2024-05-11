#ifndef GB_EMULATOR_SRC_GB_MEMORY_BREAKPOINT_HDR_
#define GB_EMULATOR_SRC_GB_MEMORY_BREAKPOINT_HDR_

#include <cstdint>

namespace gb {
    enum class BreakpointInfo : uint8_t {
        NONE = 0,
        FETCH = 1 << 0,
        READ = 1 << 1,
        WRITE = 1 << 2,
        HAS_VALUE = 1 << 3,

        ALL_MODES = FETCH | READ | WRITE,
    };

    enum class StopReason : uint8_t { FETCH, READ, WRITE };

    constexpr BreakpointInfo operator|(BreakpointInfo lhs, BreakpointInfo rhs) {
        return static_cast<BreakpointInfo>(static_cast<uint8_t>(lhs) |
                                           static_cast<uint8_t>(rhs));
    }

    constexpr BreakpointInfo operator&(BreakpointInfo lhs, BreakpointInfo rhs) {
        return static_cast<BreakpointInfo>(static_cast<uint8_t>(lhs) &
                                           static_cast<uint8_t>(rhs));
    }

    constexpr bool operator!(BreakpointInfo info) {
        return info == BreakpointInfo::NONE;
    }

    struct Breakpoint {
        BreakpointInfo info{};
        uint8_t data{};

        constexpr bool isHit(BreakpointInfo mode,
                             uint8_t value) const noexcept {
            return !!(info & mode) &&
                   (!(info & BreakpointInfo::HAS_VALUE) || data == value);
        }
    };

} // namespace gb
#endif
