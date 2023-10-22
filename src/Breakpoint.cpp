#include "Breakpoint.h"
#include <algorithm>
#include <cstdint>
#include <memory>

namespace emulator {

    std::unique_ptr<MemoryBreakpointTree::Node>
    MemoryBreakpointTree::buildTree(std::span<MemoryBreakpointData> elems) {
        if (elems.empty()) {
            return nullptr;
        }
        size_t middle = elems.size() / 2;
        auto result = std::make_unique<Node>(
            Node{.data = elems[middle], .max_address = elems[middle].max_address});
        result->left = buildTree(elems.first(middle));
        if (middle + 1 < elems.size()) {
            result->right = buildTree(elems.subspan(middle + 1));
        }

        if (result->left) {
            result->max_address = std::max(result->max_address, result->left->max_address);
            result->left->parent = result.get();
        }
        if (result->right) {
            result->max_address = std::max(result->max_address, result->right->max_address);
            result->right->parent = result.get();
        }

        return result;
    }

    void MemoryBreakpointTree::insert(Node *node, MemoryBreakpointData data) {
        node->max_address = std::max(node->max_address, data.max_address);
        if (data < node->data) {
            if (!node->left) {
                node->left =
                    std::make_unique<Node>(Node{.data = data, .max_address = data.max_address});
            } else {
                insert(node->left.get(), data);
            }
        } else {
            if (!node->right) {
                node->right =
                    std::make_unique<Node>(Node{.data = data, .max_address = data.max_address});
            } else {
                insert(node->right.get(), data);
            }
        }
    }

    void MemoryBreakpointTree::rebuildTree() {
        std::vector<MemoryBreakpointData> node_data;
        node_data.reserve(size_);
        for (auto data : *this) {
            node_data.push_back(data);
        }

        root_ = buildTree(node_data);
    }

    void MemoryBreakpointTree::insert(MemoryBreakpointData data) {
        if (!root_) {
            root_ = std::make_unique<Node>(Node{.data = data, .max_address = data.max_address});
            return;
        }
    }

    void MemoryBreakpointTree::erase(Iterator it) {
        if (it.tree_ != this) {
            return;
        }
    }

    MemoryBreakpointTree::Iterator &MemoryBreakpointTree::Iterator::operator++() {
        if (!ptr_) {
            return *this;
        }

        if (ptr_->right) {
            ptr_ = ptr_->right.get();
            for (auto next = ptr_->left.get(); next; next = ptr_->left.get()) {
                ptr_ = next;
            }
            return *this;
        }

        for (; ptr_; ptr_ = ptr_->parent) {
            if (ptr_->parent && ptr_->parent->left.get() == ptr_) {
                ptr_ = ptr_->parent;
                break;
            }
        }

        return *this;
    }
} // namespace emulator
