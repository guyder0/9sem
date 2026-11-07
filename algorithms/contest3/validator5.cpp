
#define TESTING
#include "problem5.cpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using std::vector;

int Reference(const vector<Triangle>& triangles) {
    vector<Triangle> sorted = triangles;

    std::sort(sorted.begin(), sorted.end(),
        [](const Triangle& a, const Triangle& b) {
            if (a.sa != b.sa) return a.sa < b.sa;
            if (a.sb != b.sb) return a.sb < b.sb;
            return a.sc < b.sc;
        });

    auto last = std::unique(sorted.begin(), sorted.end());
    return static_cast<int>(last - sorted.begin());
}

void PrintTriangle(const Triangle& t) {
    std::cout << t.sa << " " << t.sb << " " << t.sc;
}

void Check(const vector<Triangle>& input, const std::string& name) {
    vector<Triangle> canonical;
    canonical.reserve(input.size());

    for (const Triangle& t : input) {
        canonical.push_back(MakeCanonical(t));
    }

    int expected = Reference(canonical);
    int actual = HashTable(canonical).GetResult();

    if (actual != expected) {
        std::cout << "\nFAILED: " << name << "\n";
        std::cout << "Expected: " << expected << "\n";
        std::cout << "Actual:   " << actual << "\n";
        std::cout << "Input (" << input.size() << "):\n";

        for (const Triangle& t : input) {
            PrintTriangle(t);
            std::cout << "\n";
        }

        std::exit(1);
    }
}

int main() {
    std::cout << "Running fixed tests...\n";

    Check({{3, 4, 5}}, "Single triangle");

    Check({
        {3, 4, 5}, {3, 4, 5}, {3, 4, 5}
    }, "All identical");

    Check({
        {3, 4, 5}, {6, 8, 10}, {9, 12, 15}
    }, "Similar triangles");

    Check({
        {3, 4, 5}, {4, 5, 3}, {5, 3, 4}
    }, "Permutations");

    Check({
        {3, 4, 5}, {4, 4, 5}, {5, 5, 5}
    }, "All different");

    Check({
        {3, 3, 3}, {7, 7, 7}, {100, 100, 100}
    }, "Equilateral triangles");

    Check({
        {5, 5, 8}, {10, 10, 16}, {5, 8, 5},
        {6, 6, 8}
    }, "Isosceles triangles");

    Check({
        {2, 3, 4}, {4, 6, 8}, {6, 9, 12},
        {3, 4, 5}, {6, 8, 10}, {5, 5, 5}
    }, "Mixed duplicates");

    Check({
        {1, 1, 1}, {1, 1, 2}, {1, 2, 2},
        {2, 2, 3}, {2, 3, 3}, {3, 3, 4}
    }, "Small values");

    Check({
        {999983, 999979, 999961},
        {999979, 999961, 999983},
        {999983, 999983, 999983},
        {999979, 999979, 999979},
        {999961, 999961, 999961}
    }, "Large values");

    std::cout << "All 10 fixed tests passed!\n";

    std::mt19937 rng(std::random_device{}());

    std::uniform_int_distribution<int> size_dist(1, 3000);
    std::uniform_int_distribution<int> side_dist(1, 100000);
    std::uniform_int_distribution<int> small_dist(1, 100);
    std::uniform_int_distribution<int> scale_dist(1, 20);
    std::uniform_int_distribution<int> chance(0, 99);

    uint64_t tests = 0;

    std::cout << "Running random tests (Ctrl+C to stop)...\n";

    while (true) {
        int n = size_dist(rng);

        vector<Triangle> input;
        input.reserve(n);

        for (int i = 0; i < n; ++i) {
            Triangle t;

            int mode = chance(rng);

            if (mode < 35 && !input.empty()) {
                // Exact duplicate
                t = input[rng() % input.size()];
            } else if (mode < 65 && !input.empty()) {
                // Similar triangle (scaled copy)
                t = input[rng() % input.size()];
                int scale = scale_dist(rng);

                if (t.sa <= 100000 / scale &&
                    t.sb <= 100000 / scale &&
                    t.sc <= 100000 / scale) {
                    t.sa *= scale;
                    t.sb *= scale;
                    t.sc *= scale;
                }
            } else if (mode < 85) {
                // Small values: many duplicates
                t = {small_dist(rng), small_dist(rng),
                     small_dist(rng)};
            } else {
                // Random large values
                t = {side_dist(rng), side_dist(rng),
                     side_dist(rng)};
            }

            // Random permutation
            if (chance(rng) < 50) std::swap(t.sa, t.sb);
            if (chance(rng) < 50) std::swap(t.sb, t.sc);

            input.push_back(t);
        }

        Check(input, "Random test #" + std::to_string(tests + 1));

        ++tests;

        if (tests % 100 == 0) {
            std::cout << "\rPassed: " << tests << std::flush;
        }
    }
}
