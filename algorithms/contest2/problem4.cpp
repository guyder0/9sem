#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

unsigned int cur = 0;
unsigned int a_global;
unsigned int b_global;

const unsigned int kGenconstant = 8;

unsigned int NextRand24() {
    cur = cur * a_global + b_global;
    return cur >> kGenconstant;
}

unsigned int NextRand32() {
    unsigned int a_local = NextRand24();
    unsigned int b_local = NextRand24();
    return (a_local << kGenconstant) ^ b_local;
}

using std::vector;

unsigned int FindMedian(const vector<unsigned int>& seq) {
    unsigned int low = 0;
    unsigned int high = -1;
    unsigned int mid;
    size_t len = seq.size();
    size_t target = (len + 1) / 2;
    while (low < high) {
        mid = low + (high - low) / 2;
        size_t count = 0;
        for (auto x : seq) {
            count += (size_t)(x <= mid);
        }
        if (count < target) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

uint64_t CalculateDistance(const vector<unsigned int>& seq, unsigned int med) {
    uint64_t result = 0;
    for (auto x : seq) {
        if (x < med) {
            result += med - x;
        } else {
            result += x - med;
        }
    }
    return result;
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    size_t len;

    std::cin >> len >> a_global >> b_global;
    vector<unsigned int> seq(len);
    std::generate(seq.begin(), seq.end(), NextRand32);
    auto med = FindMedian(seq);
    auto dist = CalculateDistance(seq, med);
    std::cout << dist;

    return 0;
}
#endif
