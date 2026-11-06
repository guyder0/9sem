#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#include <list>

using std::vector;
using std::list;

struct MemoryBlock {
    int addr;
    int size;
    size_t heap_pos;
    bool free;
};

using BlockIterator = list<MemoryBlock>::iterator;

vector<BlockIterator> queries;
vector<BlockIterator> heap;
list<MemoryBlock>

uint32_t ProcessQuery(uint32_t query) {

}

void InitMemory(uint32_t memory_size) {

}

vector<int> Process() {
    uint32_t memory_size;
    size_t num_queries;
    std::cin >> memory_size >> num_queries;
    InitMemory(memory_size);

    vector<int> results(num_queries);
    for (size_t i = 0; i < num_queries; ++i) {
        uint32_t query;
        std::cin >> query;
        uint32_t result = ProcessQuery(query);
        results.push_back(result);
    }
}

void Output(const vector<int>& results) {
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << results[i] << "\n";
    }
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    vector<int> results = Process();
    Output(results);

    return 0;
}
#endif
