#include "fixed_set.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

namespace {
constexpr int64_t P = 2147483647LL;
constexpr uint64_t DEFAULT_SEED = 0x5EED12345678ULL;
std::mt19937_64 rng;
uint64_t cases = 0, checks = 0;
uint64_t seed = DEFAULT_SEED;

int R(int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}
int64_t Residue(int x) {
    int64_t r = static_cast<int64_t>(x) % P;
    return r < 0 ? r + P : r;
}

[[noreturn]] void Fail(const std::string& name, const std::vector<int>& values,
                       int query, bool expected, bool actual) {
    std::cerr << "\nFAIL case=" << cases << " name=" << name << " seed=" << seed
              << " query=" << query << " expected=" << expected
              << " actual=" << actual << "\n";
    std::cerr << "values (" << values.size() << "): ";
    for (int x : values) std::cerr << x << ' ';
    std::cerr << "\n";
    std::exit(1);
}

// Do not insert two distinct values with the same residue mod P:
// the original hash family cannot separate them and Initialize can hang.
std::vector<int> Unique(size_t n, int lo, int hi) {
    std::unordered_set<int64_t> residues;
    std::vector<int> v;
    v.reserve(n);
    size_t attempts = 0;
    while (v.size() < n) {
        if (++attempts > n * 100 + 10000) {
            std::cerr << "Generator exhausted range\n";
            std::exit(2);
        }
        int x = R(lo, hi);
        if (residues.insert(Residue(x)).second)
            v.push_back(x);
    }
    return v;
}

void Verify(const std::string& name, const std::vector<int>& values,
            const std::vector<int>& queries, FixedSet& set) {
    ++cases;
    std::unordered_set<int> reference(values.begin(), values.end());
    set.Initialize(values);
    for (int x : values) {
        bool actual = set.Contains(x);
        ++checks;
        if (!actual) Fail(name, values, x, true, actual);
    }
    for (int x : queries) {
        bool expected = reference.count(x) != 0;
        bool actual = set.Contains(x);
        ++checks;
        if (expected != actual) Fail(name, values, x, expected, actual);
    }
}

void RunHardcoded(FixedSet& set) {
    std::cerr << "Hardcoded tests...\n";
    Verify("empty", {}, {0, -1, 1, std::numeric_limits<int>::min(),
                           std::numeric_limits<int>::max()}, set);
    Verify("single zero", {0}, {-2, -1, 0, 1, 2}, set);
    Verify("single negative", {-1}, {-2, -1, 0, 1, 2}, set);
    Verify("mixed", {-3, 5, 0, 3, 7, 1}, {-4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8}, set);
    Verify("extremes", {-999999999, 999999999, -1000000000, 1000000000},
           {-1000000000, -999999999, -1, 0, 1, 999999999, 1000000000}, set);
    Verify("reinit empty", {}, {0, -3, 5}, set);
    Verify("reinit after empty", {-12, 27}, {-12, 27, 0, -13, 28}, set);

    // Exhaustively check all subsets of [-5,5].
    for (int mask = 0; mask < (1 << 11); ++mask) {
        std::vector<int> v, q;
        for (int j = -5; j <= 5; ++j) {
            q.push_back(j);
            if (mask & (1 << (j + 5))) v.push_back(j);
        }
        Verify("all subsets [-5,5]", v, q, set);
    }
    std::cerr << "Hardcoded/exhaustive passed: " << cases << " cases\n";
}

void RunRandom(FixedSet& set) {
    for (;;) {
        int mode = R(0, 9);
        size_t n;
        int lo, hi;
        if (mode <= 2) { n = static_cast<size_t>(R(0, 25)); lo = -100; hi = 100; }
        else if (mode <= 5) { n = static_cast<size_t>(R(0, 300)); lo = -100000; hi = 100000; }
        else if (mode <= 7) { n = static_cast<size_t>(R(0, 2000)); lo = -1000000000; hi = 1000000000; }
        else if (mode == 8) { n = static_cast<size_t>(R(2000, 10000)); lo = -1000000000; hi = 1000000000; }
        else { n = static_cast<size_t>(R(0, 1000)); lo = -1000000000; hi = 1000000000; }

        auto values = Unique(n, lo, hi);
        if (R(0, 1)) std::sort(values.begin(), values.end());
        else std::shuffle(values.begin(), values.end(), rng);

        std::vector<int> queries;
        queries.reserve(n + 1000);
        for (int x : values) queries.push_back(x);
        for (int i = 0; i < 250; ++i) queries.push_back(R(lo, hi));
        for (int i = 0; i < 250; ++i)
            queries.push_back(R(-1000000000, 1000000000));
        for (int i = 0; i < 100; ++i) queries.push_back(R(-100, 100));
        queries.insert(queries.end(), {0, -1, 1, -1000000000, 1000000000, 999999999, -999999999});

        Verify("random mode " + std::to_string(mode), values, queries, set);
        if (cases % 100 == 0) {
            std::cerr << "PASS cases=" << cases << " checks=" << checks
                      << " seed=" << seed << " (Ctrl+C to stop)\n";
        }
    }
}
} // namespace

int main(int argc, char** argv) {
    if (argc > 1) {
        try { seed = std::stoull(argv[1], nullptr, 0); }
        catch (...) { std::cerr << "Usage: " << argv[0] << " [seed]\n"; return 2; }
    }
    rng.seed(seed);
    std::cerr << "Seed: " << seed << "\n";
    FixedSet set;
    RunHardcoded(set);
    RunRandom(set);
}
