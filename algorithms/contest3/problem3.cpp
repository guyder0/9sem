#include <algorithm>
#include <iostream>
#include <list>
#include <utility>
#include <vector>

using std::list;
using std::vector;

struct MemoryBlock {
    int addr;
    int size;
    size_t heap_pos;
    bool free;
};

using Memory = list<MemoryBlock>;
using BlockReference = Memory::iterator;

vector<BlockReference> queries;
vector<BlockReference> heap;
Memory memory;

//  -- private heap functions

bool Better(BlockReference b1, BlockReference b2) {
    return b1->size > b2->size || (b1->size == b2->size && b1->addr < b2->addr);
}

void SwapHeap(size_t pos1, size_t pos2) {
    heap[pos1]->heap_pos = pos2;
    heap[pos2]->heap_pos = pos1;
    std::swap(heap[pos1], heap[pos2]);
}

void SiftUp(size_t pos) {
    while (pos > 0 && Better(heap[pos], heap[(pos - 1) / 2])) {
        size_t swap_pos = (pos - 1) / 2;
        SwapHeap(pos, swap_pos);
        pos = swap_pos;
    }
}

void SiftDown(size_t pos) {
    while (
        (2 * pos + 1 < heap.size() && Better(heap[2 * pos + 1], heap[pos])) ||
        (2 * pos + 2 < heap.size() && Better(heap[2 * pos + 2], heap[pos]))) {
        size_t swap_pos = 2 * pos + 1;
        size_t cand_pos = 2 * pos + 2;
        if (cand_pos < heap.size() && Better(heap[cand_pos], heap[swap_pos])) {
            swap_pos = cand_pos;
        }
        SwapHeap(pos, swap_pos);
        pos = swap_pos;
    }
}

//  -- heap interface

void HeapAdd(BlockReference elem) {
    heap.push_back(elem);
    elem->heap_pos = heap.size() - 1;
    SiftUp(elem->heap_pos);
}

void HeapDelete(size_t pos) {
    SwapHeap(pos, heap.size() - 1);
    heap.pop_back();
    if (pos == heap.size()) {
        return;
    }
    if (pos > 0 && Better(heap[pos], heap[(pos - 1) / 2])) {
        SiftUp(pos);
    } else {
        SiftDown(pos);
    }
}

void HeapChange(size_t pos, int delta) {
    heap[pos]->size += delta;
    if (delta > 0) {
        SiftUp(pos);
    } else {
        SiftDown(pos);
    }
}

void InitMemory(int memory_size) {
    memory.push_back({0, memory_size, 0, true});
    heap.push_back(memory.begin());
}

//  -- task functions

int ProcessQuery(int query) {
    if (query > 0) {
        if (heap.empty() || heap[0]->size < query) {
            queries.push_back(memory.end());
            return -1;
        }
        auto free_block = heap[0];
        auto block =
            memory.insert(free_block, {free_block->addr, query, 0, false});
        free_block->addr += query;
        queries.push_back(block);
        if (free_block->size == query) {
            HeapDelete(0);
            memory.erase(free_block);
        } else {
            HeapChange(0, -query);
        }
        return block->addr + 1;
    }

    auto block = queries[-query - 1];
    if (block == memory.end()) {
        queries.push_back(memory.end());
        return 0;
    }

    bool first_block = block == memory.begin();
    auto next_block = std::next(block);
    bool last_block = next_block == memory.end();
    auto prev_block = first_block ? memory.end() : std::prev(block);
    // prev free next busy
    if (!first_block && prev_block->free && (last_block || !next_block->free)) {
        HeapChange(prev_block->heap_pos, block->size);
        memory.erase(block);
    }
    // prev busy next free
    else if ((first_block || !prev_block->free) && !last_block &&
             next_block->free) {
        next_block->addr -= block->size;
        HeapChange(next_block->heap_pos, block->size);
        memory.erase(block);
    }
    // prev busy next busy
    else if ((first_block || !prev_block->free) &&
             (last_block || !next_block->free)) {
        block->free = true;
        HeapAdd(block);
    } else {  // next and prev block are free
        HeapChange(prev_block->heap_pos, block->size + next_block->size);
        HeapDelete(next_block->heap_pos);
        memory.erase(block);
        memory.erase(next_block);
    }
    queries.push_back(memory.end());
    return 0;
}

vector<int> Process() {
    int memory_size;
    size_t num_queries;
    std::cin >> memory_size >> num_queries;
    InitMemory(memory_size);

    vector<int> results;
    results.reserve(num_queries);
    for (size_t i = 0; i < num_queries; ++i) {
        int query;
        std::cin >> query;
        int result = ProcessQuery(query);
        results.push_back(result);
    }

    return results;
}

void Output(const vector<int>& results) {
    for (size_t i = 0; i < results.size(); ++i) {
        if (results[i] == 0) {
            continue;
        }
        std::cout << results[i] << "\n";
    }
}

//  -- main

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto results = Process();
    Output(results);

    return 0;
}
#endif
