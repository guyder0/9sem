#define TESTING
#include "problem3.cpp"

#include <cstdint>
#include <cstdlib>
#include <random>
#include <string>
#include <utility>

struct TestCase {
    int n;
    std::vector<int> requests;
    std::string name;
};

void ResetSolution() {
    queries.clear();
    heap.clear();
    memory.clear();
}

std::vector<int> RunSolution(const TestCase& test) {
    ResetSolution();
    InitMemory(test.n);
    std::vector<int> answers;
    for (int request : test.requests) {
        int result = ProcessQuery(request);
        if (request > 0) answers.push_back(result);
    }
    return answers;
}

std::vector<int> RunReference(const TestCase& test) {
    std::vector<unsigned char> occupied(test.n, 0);
    // The i-th request owns [start, start + length), or has no allocation.
    std::vector<std::pair<int, int>> allocations(test.requests.size(), {-1, 0});
    std::vector<int> answers;

    for (size_t i = 0; i < test.requests.size(); ++i) {
        int request = test.requests[i];
        if (request > 0) {
            int best_start = -1, best_length = 0;
            for (int j = 0; j < test.n;) {
                if (occupied[j]) {
                    ++j;
                    continue;
                }
                int start = j;
                while (j < test.n && !occupied[j]) ++j;
                int length = j - start;
                if (length > best_length) {
                    best_length = length;
                    best_start = start;
                }
            }
            if (best_length < request) {
                answers.push_back(-1);
            } else {
                answers.push_back(best_start + 1);
                allocations[i] = {best_start, request};
                for (int j = best_start; j < best_start + request; ++j)
                    occupied[j] = 1;
            }
        } else {
            size_t index = static_cast<size_t>(-request - 1);
            auto [start, length] = allocations[index];
            if (start != -1) {
                for (int j = start; j < start + length; ++j)
                    occupied[j] = 0;
            }
        }
    }
    return answers;
}

void PrintTest(const TestCase& test) {
    std::cout << test.n << ' ' << test.requests.size() << '\n';
    for (int request : test.requests) std::cout << request << '\n';
}

void PrintAnswers(const std::vector<int>& answers) {
    for (int x : answers) std::cout << x << '\n';
}

void Check(const TestCase& test, uint64_t number, uint64_t seed) {
    auto expected = RunReference(test);
    auto actual = RunSolution(test);
    if (expected == actual) return;

    std::cout << "FAIL: " << test.name << " (test " << number << ", seed " << seed << ")\n";
    std::cout << "--- Input ---\n";
    PrintTest(test);
    std::cout << "--- Expected ---\n";
    PrintAnswers(expected);
    std::cout << "--- Actual ---\n";
    PrintAnswers(actual);
    std::cout.flush();
    std::exit(1);
}

std::vector<TestCase> FixedTests() {
    return {
        {6, {2, 3, -1, 3, 3, -5, 2, 2}, "statement example"},
        {1, {1, 1, -1, 1, -2, -4}, "single cell"},
        {10, {10, 1, -1, 10}, "full allocation"},
        {10, {3, 4, 3, -2, -1, -3, 10}, "merge both sides"},
        {12, {2, 2, 2, 2, 2, 2, -2, -4, -6, 2, 2, 2}, "equal free blocks"},
        {8, {4, 5, -2, -1, 8}, "failed allocation and ignored free"},
        {15, {3, 3, 3, 3, 3, -1, -3, -5, 2, 2, 2}, "leftmost tie"},
        {9, {2, 3, 4, -2, -1, 5, -3, 9}, "fragmentation and recovery"},
        {20, {5, 5, 5, 5, -2, -3, 10, -1, -4, -7, 20}, "repeated merging"},
        {7, {8, 7, -1, -2, 1, -5, 7}, "oversized and failed frees"},
    };
}

TestCase RandomTest(std::mt19937_64& rng, uint64_t number) {
    auto randint = [&](int low, int high) {
        return std::uniform_int_distribution<int>(low, high)(rng);
    };
    int n = randint(1, 100);
    int m = randint(1, 300);
    TestCase test{n, {}, "random #" + std::to_string(number)};
    test.requests.reserve(m);
    std::vector<int> releasable;

    for (int i = 1; i <= m; ++i) {
        bool release = !releasable.empty() && randint(0, 99) < 42;
        if (release) {
            int index = randint(0, static_cast<int>(releasable.size()) - 1);
            test.requests.push_back(-releasable[index]);
            releasable[index] = releasable.back();
            releasable.pop_back();
        } else {
            int k = randint(1, n);
            if (randint(0, 99) < 35) k = randint(1, std::min(n, 8));
            test.requests.push_back(k);
            releasable.push_back(i);
        }
    }
    return test;
}

int main(int argc, char** argv) {
    uint64_t seed = std::random_device{}();
    if (argc > 1) seed = std::stoull(argv[1]);
    std::mt19937_64 rng(seed);
    std::cout << "Seed: " << seed << '\n';

    auto fixed = FixedTests();
    for (size_t i = 0; i < fixed.size(); ++i)
        Check(fixed[i], i + 1, seed);
    std::cout << "All " << fixed.size() << " fixed tests passed.\n";

    for (uint64_t i = 1;; ++i) {
        Check(RandomTest(rng, i), i, seed);
        if (i % 10000 == 0) {
            std::cout << "Passed " << i << " random tests\n";
            std::cout.flush();
        }
    }
}
