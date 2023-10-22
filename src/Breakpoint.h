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
        uint16_t min_address = 0;
        uint16_t max_address = 0;

        bool break_on_read = true;
        bool break_on_write = true;

        std::optional<uint8_t> value;

        bool empty() const { return min_address == max_address; }
        bool isInRange(uint16_t address) const {
            return address >= min_address && address < max_address;
        }
        bool overlaps(MemoryBreakpointData other) const {
            if (empty()) {
                return false;
            }

            return other.isInRange(min_address) || other.isInRange(max_address - 1);
        }

        bool operator<(MemoryBreakpointData other) {
            return min_address != other.min_address ? min_address < other.min_address
                                                    : max_address < other.max_address;
        }
    };

    class MemoryBreakpointTree {
        struct Node {
            MemoryBreakpointData data;
            uint16_t max_address = 0;

            Node *parent = nullptr;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
        };

      public:
        class Iterator {
            friend class MemoryBreakpointTree;

          public:
            const MemoryBreakpointData &operator*() const { return ptr_->data; }
            const MemoryBreakpointData *operator->() const { return &ptr_->data; }

            Iterator &operator++();

            bool operator==(Iterator other) { return ptr_ == other.ptr_; }

          private:
            const Node *ptr_ = nullptr;
            const MemoryBreakpointTree *tree_ = nullptr;
        };

        void insert(MemoryBreakpointData data);
        void erase(Iterator it);

        void rebuildTree();

        bool isBreakpointTriggered(uint16_t address, uint8_t data, bool is_read) {
            return find(root_.get(), address, data, is_read);
        }

        Iterator begin() const;
        Iterator end() const;

        size_t size() const;

      private:
        static bool find(Node *node, uint16_t address, uint8_t data, bool is_read);
        static Node *getParent(Node *node) { return node ? node->parent : nullptr; }
        static std::unique_ptr<Node> buildTree(std::span<MemoryBreakpointData> elems);
        static void insert(Node *node, MemoryBreakpointData data);

        std::unique_ptr<Node> root_;
        size_t size_ = 0;
    };

    class MemoryBreakpoints : public gb::MemoryObserver {
      public:
        MemoryBreakpoints(std::function<void()> &&callback) : callback_(std::move(callback)) {}

        void addBreakpoint(MemoryBreakpointData data) { breakpoints_.insert(data); }

        void removeBreakpoint(MemoryBreakpointTree::Iterator it) { breakpoints_.erase(it); }

        const MemoryBreakpointTree &getBreakpoints() const { return breakpoints_; }

        void onRead(uint16_t address, uint8_t data) override {
            if (breakpoints_.isBreakpointTriggered(address, data, true)) {
                callback_();
            }
        }
        void onWrite(uint16_t address, uint8_t data) override {
            if (breakpoints_.isBreakpointTriggered(address, data, false)) {
                callback_();
            }
        }

      private:
        std::function<void()> callback_;
        MemoryBreakpointTree breakpoints_;
    };
} // namespace emulator
