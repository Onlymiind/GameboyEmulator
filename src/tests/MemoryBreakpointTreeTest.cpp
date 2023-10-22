#include "Breakpoint.h"

#include "catch2/catch_test_macros.hpp"
#include <array>

using namespace emulator;
std::array full_tree = {
    MemoryBreakpointData{0, 1},   MemoryBreakpointData{2, 3},   MemoryBreakpointData{4, 5},
    MemoryBreakpointData{6, 7},   MemoryBreakpointData{8, 9},   MemoryBreakpointData{10, 11},
    MemoryBreakpointData{12, 13}, MemoryBreakpointData{14, 15}, MemoryBreakpointData{16, 17},
    MemoryBreakpointData{18, 19}, MemoryBreakpointData{20, 21}, MemoryBreakpointData{22, 23},
    MemoryBreakpointData{24, 25}, MemoryBreakpointData{26, 27}, MemoryBreakpointData{27, 28}};

TEST_CASE("insertion") {
    SECTION("construction from range of elements") {
        MemoryBreakpointTree t{full_tree};
        REQUIRE(t.size() == full_tree.size());
        size_t idx = 0;
        for (auto data : t) {
            REQUIRE(data == full_tree[idx]);
            ++idx;
        }
    }
    SECTION("individual insertion") {
        MemoryBreakpointTree t;
        for (auto data : full_tree) {
            t.insert(data);
        }
        REQUIRE(t.size() == full_tree.size());
        size_t idx = 0;
        for (auto data : t) {
            REQUIRE(data == full_tree[idx]);
            ++idx;
        }
    }

    SECTION("rebuilding") {
        MemoryBreakpointTree t;
        for (auto data : full_tree) {
            t.insert(data);
        }
        t.rebuildTree();
        REQUIRE(t.size() == full_tree.size());
        size_t idx = 0;
        for (auto data : t) {
            REQUIRE(data == full_tree[idx]);
            ++idx;
        }
    }
}

// TODO: deletion, search
