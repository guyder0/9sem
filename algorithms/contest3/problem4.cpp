#include <algorithm>
#include <cstdint>
#include <iostream>
#include <queue>
#include <vector>

using std::queue;
using std::vector;

typedef bool (*Comparator)(int x1, int x2);
typedef struct {
    int value;
    size_t heap_pos;
    bool less_heap;
} QueueElem;

class Heap {
private:
    Comparator comp_;
    vector<QueueElem*> heap_;

    void SwapHeap(size_t pos1, size_t pos2) {
        heap_[pos1]->heap_pos = pos2;
        heap_[pos2]->heap_pos = pos1;
        std::swap(heap_[pos1], heap_[pos2]);
    }

    void SiftUp(size_t pos) {
        while (pos > 0 &&
               !comp_(heap_[pos]->value, heap_[(pos - 1) / 2]->value)) {
            size_t swap_pos = (pos - 1) / 2;
            SwapHeap(pos, swap_pos);
            pos = swap_pos;
        }
    }

    void SiftDown(size_t pos) {
        while ((2 * pos + 1 < heap_.size() &&
                !comp_(heap_[2 * pos + 1]->value, heap_[pos]->value)) ||
               (2 * pos + 2 < heap_.size() &&
                !comp_(heap_[2 * pos + 2]->value, heap_[pos]->value))) {
            size_t swap_pos = 2 * pos + 1;
            size_t cand_pos = 2 * pos + 2;
            if (cand_pos < heap_.size() &&
                !comp_(heap_[cand_pos]->value, heap_[swap_pos]->value)) {
                swap_pos = cand_pos;
            }
            SwapHeap(pos, swap_pos);
            pos = swap_pos;
        }
    }

public:
    Heap(Comparator comp) : comp_(comp) {}

    size_t Size() { return heap_.size(); }

    void Add(QueueElem* elem) {
        heap_.push_back(elem);
        elem->heap_pos = heap_.size() - 1;
        SiftUp(elem->heap_pos);
    }

    QueueElem* Get() { return heap_[0]; }

    void Delete(size_t pos) {
        SwapHeap(pos, heap_.size() - 1);
        heap_.pop_back();
        if (pos == heap_.size()) {
            return;
        }
        if (pos > 0 && !comp_(heap_[pos]->value, heap_[(pos - 1) / 2]->value)) {
            SiftUp(pos);
        } else {
            SiftDown(pos);
        }
    }
};

class KthElementQueue {
private:
    size_t k_th_;
    queue<QueueElem> queue_;
    Heap less_k_;
    Heap more_k_;

public:
    KthElementQueue(int k_th)
        : k_th_(k_th),
          less_k_(Heap([](int x1, int x2) { return x1 < x2; })),
          more_k_(Heap([](int x1, int x2) { return x1 > x2; })) {}

    void Push(int elem) {
        if (less_k_.Size() < k_th_) {
            queue_.push({elem, 0, true});
            less_k_.Add(&queue_.back());
        } else if (less_k_.Get()->value > elem) {
            queue_.push({elem, 0, true});
            auto* swap = less_k_.Get();
            less_k_.Delete(0);
            less_k_.Add(&queue_.back());
            swap->less_heap = false;
            more_k_.Add(swap);
        } else {
            queue_.push({elem, 0, false});
            more_k_.Add(&queue_.back());
        }
    }

    void Pop() {
        auto& popped = queue_.front();
        if (popped.less_heap) {
            less_k_.Delete(popped.heap_pos);
            if (more_k_.Size() > 0) {
                auto* swap = more_k_.Get();
                more_k_.Delete(0);
                swap->less_heap = true;
                less_k_.Add(swap);
            }
        } else {
            more_k_.Delete(popped.heap_pos);
        }
        queue_.pop();
    }

    int Get() {
        if (less_k_.Size() < k_th_) {
            return -1;
        }
        return less_k_.Get()->value;
    }
};

vector<int> InputNumbers(size_t len) {
    vector<int> result(len);
    for (auto& el : result) {
        std::cin >> el;
    }
    return result;
}

vector<char> InputQueries(size_t len) {
    vector<char> result(len);
    for (auto& el : result) {
        std::cin >> el;
    }
    return result;
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t num_numbers;
    size_t num_queries;
    size_t k_order;
    std::cin >> num_numbers >> num_queries >> k_order;
    auto numbers = InputNumbers(num_numbers);
    auto queries = InputQueries(num_queries);

    KthElementQueue kth_element_queue(k_order);
    kth_element_queue.Push(numbers[0]);
    size_t cur_push = 1;

    for (char query : queries) {
        if (query == 'R') {
            kth_element_queue.Push(numbers[cur_push++]);
        } else {
            kth_element_queue.Pop();
        }
        std::cout << kth_element_queue.Get() << "\n";
    }

    return 0;
}
#endif
