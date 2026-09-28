#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <csignal>
#include <atomic>

#define TESTING
#include "problem1.cpp"

std::atomic<bool> stop_flag{false};

void SigintHandler(int) {
    stop_flag = true;
}

void Search(int pos, std::vector<int>& cur, const std::vector<int64_t>& a, std::vector<int>& best) {
    if (cur.size() + (a.size() - pos) < best.size()) {
        return;
    }
    if (pos == static_cast<int>(a.size())) {
        if (cur.empty()) {
            return;
        }
        if (cur.size() > best.size()) {
            best = cur;
        } else if (cur.size() == best.size() && cur < best) {
            best = cur;
        }
        return;
    }

    bool can_add = false;
    size_t sz = cur.size();
    if (sz == 0) {
        can_add = true;
    } else if (sz == 1) {
        can_add = (a[cur[0]] != a[pos]);
    } else {
        int64_t x = a[cur[sz - 2]];
        int64_t y = a[cur[sz - 1]];
        int64_t z = a[pos];
        can_add = ((x < y && y > z) || (x > y && y < z));
    }

    if (can_add) {
        cur.push_back(pos);
        Search(pos + 1, cur, a, best);
        cur.pop_back();
    }

    Search(pos + 1, cur, a, best);
}

std::vector<int64_t> SolveNaive(const std::vector<int64_t>& a) {
    if (a.empty()) {
        return {};
    }
    std::vector<int> cur;
    std::vector<int> best;
    Search(0, cur, a, best);

    std::vector<int64_t> result;
    result.reserve(best.size());
    for (int idx : best) {
        result.push_back(a[idx]);
    }
    return result;
}

int main() {
    std::signal(SIGINT, SigintHandler);

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int> len_dist(1, 15);
    std::uniform_int_distribution<int> type_dist(0, 2);
    std::uniform_int_distribution<int64_t> small_val_dist(-5, 5);
    std::uniform_int_distribution<int64_t> med_val_dist(-100, 100);
    std::uniform_int_distribution<int64_t> large_val_dist(-1000000000LL, 1000000000LL);

    uint64_t test_case = 1;

    while (!stop_flag) {
        int n = len_dist(rng);
        int val_type = type_dist(rng);

        std::vector<int64_t> a(n);
        for (int i = 0; i < n; ++i) {
            if (val_type == 0) {
                a[i] = small_val_dist(rng);
            } else if (val_type == 1) {
                a[i] = med_val_dist(rng);
            } else {
                a[i] = large_val_dist(rng);
            }
        }

        std::vector<int64_t> expected = SolveNaive(a);
        std::vector<int64_t> actual = Process(a);

        if (actual != expected) {
            std::cout << "Mismatch found on test " << test_case << ":\n";
            std::cout << n << "\n";
            for (int i = 0; i < n; ++i) {
                std::cout << a[i] << (i + 1 == n ? "" : " ");
            }
            std::cout << "\n";

            std::cout << "Expected (size " << expected.size() << "):\n";
            for (size_t i = 0; i < expected.size(); ++i) {
                std::cout << expected[i] << (i + 1 == expected.size() ? "" : " ");
            }
            std::cout << "\n";

            std::cout << "Actual (size " << actual.size() << "):\n";
            for (size_t i = 0; i < actual.size(); ++i) {
                std::cout << actual[i] << (i + 1 == actual.size() ? "" : " ");
            }
            std::cout << "\n";
            return 1;
        }

        if (test_case % 1000 == 0) {
            std::cout << "Passed " << test_case << " tests\n";
        }

        ++test_case;
    }

    std::cout << "Validation stopped by user. Total tests passed: " << test_case - 1 << "\n";
    return 0;
}
