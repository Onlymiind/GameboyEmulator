#include "Breakpoint.h"

#include "catch2/catch_test_macros.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <set>

using namespace emulator;
using enum MemoryBreakpointData::BreakOn;
std::array test_data = {MemoryBreakpointData{0, READ},
                        MemoryBreakpointData{0, ALWAYS, 10},
                        MemoryBreakpointData{0, WRITE},
                        MemoryBreakpointData{.address = 8, .value = 10},
                        MemoryBreakpointData{10},
                        MemoryBreakpointData{.address = 12, .value = 10},
                        MemoryBreakpointData{12, WRITE},
                        MemoryBreakpointData{14, READ},
                        MemoryBreakpointData{.address = 14, .value = 10},
                        MemoryBreakpointData{16},
                        MemoryBreakpointData{18},
                        MemoryBreakpointData{20},
                        MemoryBreakpointData{22},
                        MemoryBreakpointData{24, READ},
                        MemoryBreakpointData{24, ALWAYS},
                        MemoryBreakpointData{24, WRITE}};

class InitTest {
  public:
    InitTest() {
        std::iota(indicies_.begin(), indicies_.end(), 0);
        std::random_device d{};
        std::mt19937_64 random{d()};
        std::ranges::shuffle(indicies_, random);

        for (auto idx : indicies_) {
            breakpoints_.addBreakpoint(test_data[idx]);
        }
    }

  protected:
    MemoryBreakpoints breakpoints_{[this]() { callback_called_ = true; }};
    std::array<size_t, test_data.size()> indicies_;
    bool callback_called_ = false;
};

TEST_CASE_METHOD(InitTest, "addBreakpoint, ordering") {

    auto result = breakpoints_.getBreakpoints();
    REQUIRE(result.size() == test_data.size());

    for (size_t i = 0; i < result.size(); ++i) {
        REQUIRE(test_data[i].address == result[i].address);
        REQUIRE(test_data[i].break_on == result[i].break_on);
    }
}

TEST_CASE("minAddress, maxAddress") {
    MemoryBreakpoints br{[]() {}};
    REQUIRE(br.minAddress() == 0);
    REQUIRE(br.maxAddress() == uint16_t(-1));
}

TEST_CASE_METHOD(InitTest, "onRead") {
    SECTION("simple") {
        breakpoints_.onRead(10, 0);
        REQUIRE(callback_called_);
        callback_called_ = false;
    }
    SECTION("multiple breakpoints on same address") {
        breakpoints_.onRead(0, 0);
        REQUIRE(callback_called_);
        callback_called_ = false;
    }
    SECTION("breakpoint with value set") {
        breakpoints_.onRead(8, 0);
        REQUIRE_FALSE(callback_called_);
        breakpoints_.onRead(8, 10);
        REQUIRE(callback_called_);
        callback_called_ = false;
    }
    SECTION("multiple breakpoints with value set") {
        breakpoints_.onRead(12, 0);
        REQUIRE_FALSE(callback_called_);
        breakpoints_.onRead(12, 10);
        REQUIRE(callback_called_);
        callback_called_ = false;
    }
}

TEST_CASE_METHOD(InitTest, "onWrite") {
    SECTION("simple") {
        breakpoints_.onWrite(10, 0);
        REQUIRE(callback_called_);
        callback_called_ = false;
    }
    SECTION("multiple breakpoints on same address") {
        breakpoints_.onWrite(0, 0);
        REQUIRE(callback_called_);
        callback_called_ = false;
    }
    SECTION("breakpoint with value set") {
        breakpoints_.onWrite(8, 0);
        REQUIRE_FALSE(callback_called_);
        breakpoints_.onWrite(8, 10);
        REQUIRE(callback_called_);
        callback_called_ = false;
    }
    SECTION("multiple breakpoints with value set") {
        breakpoints_.onWrite(14, 0);
        REQUIRE_FALSE(callback_called_);
        breakpoints_.onWrite(14, 10);
        REQUIRE(callback_called_);
        callback_called_ = false;
    }
}

TEST_CASE_METHOD(InitTest, "deletion") {
    std::set<MemoryBreakpointData> deleted;
    for (size_t idx : indicies_) {
        breakpoints_.removeBreakpoint(test_data[idx]);
        deleted.insert(test_data[idx]);
        auto rest = breakpoints_.getBreakpoints();
        REQUIRE(rest.size() == test_data.size() - deleted.size());
        for (size_t i = 0; i < rest.size(); ++i) {
            REQUIRE(!deleted.contains(rest[i]));
            if (i != 0) {
                bool le = rest[i - 1] < rest[i] || rest[i - 1] == rest[i];
                REQUIRE(le);
            }
        }
    }
}
