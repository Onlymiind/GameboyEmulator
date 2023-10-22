#include "Breakpoint.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

namespace emulator {

    bool MemoryBreakpointTree::find(Node *node, uint16_t address, uint8_t data, bool is_read) {
        if (!node) {
            return false;
        }

        if (address < node->data.min_address) {
            return find(node->left.get(), address, data, is_read);
        } else if (address > node->max_address) {
            return false;
        }

        bool found =
            node->data.isInRange(address) && (!node->data.value || *node->data.value == data);
        if (found) {
            if (is_read) {
                found = node->data.break_on_read;
            } else {
                found = node->data.break_on_write;
            }
        }

        if (found) {
            return found;
        }

        found = find(node->left.get(), address, data, is_read);
        if (found) {
            return found;
        }

        found = find(node->right.get(), address, data, is_read);
        return found;
    }

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
        if (node->data == data) {
            return;
        }
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
        node->max_address = std::max(node->max_address, data.max_address);
    }

    std::unique_ptr<MemoryBreakpointTree::Node>
    MemoryBreakpointTree::replace(MemoryBreakpointTree &tree, Node *old_node,
                                  std::unique_ptr<Node> &&new_node) {
        if (!old_node->parent) {
            tree.root_.swap(new_node);
        } else if (old_node == old_node->parent->left.get()) {
            old_node->parent->left.swap(new_node);
        } else {
            old_node->parent->right.swap(new_node);
        }
        if (new_node) {
            new_node->parent = old_node->parent;
        }

        return new_node;
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
            ++size_;
            return;
        }

        insert(root_.get(), data);

        ++size_;
    }

    void MemoryBreakpointTree::erase(Iterator it) {
        if (it.tree_ != this || !it.ptr_) {
            return;
        }

        if (!it.ptr_->left) {
            replace(*this, it.ptr_, std::move(it.ptr_->right));
        } else if (!it.ptr_->right) {
            replace(*this, it.ptr_, std::move(it.ptr_->left));
        } else {
            Node *successor_ptr = it.ptr_->right.get();
            for (; successor_ptr->left; successor_ptr = successor_ptr->left.get())
                ;
            std::unique_ptr<Node> successor;
            if (successor_ptr == it.ptr_->right.get()) {
                successor = std::move(it.ptr_->right);
            } else {
                successor = replace(*this, successor_ptr, std::move(successor->right));
                successor->right = std::move(it.ptr_->right);
                successor->right->parent = successor.get();
            }
            successor->left = std::move(it.ptr_->left);
            successor->left->parent = successor.get();
            replace(*this, it.ptr_, std::move(successor));
        }

        --size_;
    }

    MemoryBreakpointTree::Iterator MemoryBreakpointTree::begin() const {
        Iterator result{root_.get(), this};
        if (!root_) {
            return result;
        }
        for (Node *ptr = root_.get(); ptr->left; ptr = ptr->left.get()) {
            result.ptr_ = ptr->left.get();
        }
        return result;
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
