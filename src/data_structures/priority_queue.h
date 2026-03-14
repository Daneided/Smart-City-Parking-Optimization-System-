// Priority queue for search ranking and pathfinding
// Wraps heap with decrease-key support needed by A* and spot ranking

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <algorithm>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// Min-heap priority queue with decrease-key support.
// Supports updating priority of existing items without duplicates.
// Uses lazy deletion with a removed sentinel.
class PriorityQueue {
public:
    PriorityQueue() : _counter(0), _size(0) {}

    int size() const { return _size; }
    bool empty() const { return _size == 0; }
    bool contains(const std::string& item) const {
        return _entry_map.find(item) != _entry_map.end();
    }

    // Add item or update its priority if already present.
    void push(const std::string& item, double priority) {}

    // Remove and return item with lowest priority.
    std::optional<std::string> pop() { return std::nullopt; }

    // Remove and return (item, priority) with lowest priority.
    std::optional<std::pair<std::string, double>> pop_with_priority() { return std::nullopt; }

    // Return item with lowest priority without removing it.
    std::optional<std::string> peek() { return std::nullopt; }

    // Return the lowest priority value without removing.
    std::optional<double> peek_priority() { return std::nullopt; }

    // Get current priority of an item, or nullopt if not present.
    std::optional<double> get_priority(const std::string& item) { return std::nullopt; }

    // Decrease priority of an existing item.
    // Returns false if item not found or new priority is not lower.
    bool decrease_key(const std::string& item, double new_priority) { return false; }

private:
    struct Entry {
        double priority;
        int counter;
        std::string item;
        bool removed;

        bool operator>(const Entry& other) const {
            if (priority != other.priority) return priority > other.priority;
            return counter > other.counter;
        }
    };

    std::vector<Entry> _heap;
    std::unordered_map<std::string, int> _entry_map;  // item -> index in _heap
    int _counter;
    int _size;

    void _sift_up(int idx) {
        while (idx > 0) {
            int parent = (idx - 1) / 2;
            if (_heap[idx].priority < _heap[parent].priority ||
                (_heap[idx].priority == _heap[parent].priority &&
                 _heap[idx].counter < _heap[parent].counter)) {
                _swap(idx, parent);
                idx = parent;
            } else {
                break;
            }
        }
    }

    void _sift_down(int idx) {
        int n = static_cast<int>(_heap.size());
        while (true) {
            int smallest = idx;
            int left = 2 * idx + 1;
            int right = 2 * idx + 2;
            if (left < n && !(_heap[left] > _heap[smallest]) && (left != smallest)) {
                if (_heap[left].priority < _heap[smallest].priority ||
                    (_heap[left].priority == _heap[smallest].priority &&
                     _heap[left].counter < _heap[smallest].counter)) {
                    smallest = left;
                }
            }
            if (right < n) {
                if (_heap[right].priority < _heap[smallest].priority ||
                    (_heap[right].priority == _heap[smallest].priority &&
                     _heap[right].counter < _heap[smallest].counter)) {
                    smallest = right;
                }
            }
            if (smallest == idx) break;
            _swap(idx, smallest);
            idx = smallest;
        }
    }

    void _swap(int i, int j) {
        std::swap(_heap[i], _heap[j]);
        if (!_heap[i].removed) _entry_map[_heap[i].item] = i;
        if (!_heap[j].removed) _entry_map[_heap[j].item] = j;
    }
};

#endif // PRIORITY_QUEUE_H
