
#define TESTING
#include "problem4.cpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <deque>
#include <random>
#include <string>
#include <vector>

using std::vector;

struct TestCase {
    int k;
    vector<int> numbers;
    std::string operations;
};

void PrintTest(const TestCase& test) {
    std::cerr << test.numbers.size() << ' '
              << test.operations.size() << ' '
              << test.k << '\n';

    for (int x : test.numbers) {
        std::cerr << x << ' ';
    }
    std::cerr << '\n' << test.operations << '\n';
}

void ValidateTest(const TestCase& test) {
    int n = static_cast<int>(test.numbers.size());
    int m = static_cast<int>(test.operations.size());

    assert(1 <= n && n <= 100000);
    assert(1 <= test.k && test.k <= 100000);
    assert(m <= 2 * n - 2);

    for (int x : test.numbers) {
        assert(0 <= x && x <= 1000000000);
    }

    int l = 0;
    int r = 0;

    for (char op : test.operations) {
        if (op == 'R') {
            ++r;
        } else {
            assert(op == 'L');
            ++l;
        }

        assert(l <= r);
        assert(r < n);
    }
}

int Expected(const std::deque<int>& values, int k) {
    if (values.size() < static_cast<size_t>(k)) {
        return -1;
    }

    vector<int> sorted(values.begin(), values.end());
    std::sort(sorted.begin(), sorted.end());
    return sorted[k - 1];
}

void RunTest(const TestCase& test, size_t test_id) {
    ValidateTest(test);

    KthElementQueue actual(test.k);
    std::deque<int> reference;

    actual.Push(test.numbers[0]);
    reference.push_back(test.numbers[0]);

    size_t next_number = 1;

    for (size_t i = 0; i < test.operations.size(); ++i) {
        char op = test.operations[i];

        if (op == 'R') {
            int value = test.numbers[next_number++];
            actual.Push(value);
            reference.push_back(value);
        } else {
            actual.Pop();
            reference.pop_front();
        }

        int expected = Expected(reference, test.k);
        int got = actual.Get();

        if (expected != got) {
            std::cerr << "\nFAILED test #" << test_id
                      << ", operation #" << i + 1 << "\n\n";

            PrintTest(test);

            std::cerr << "\nFailed prefix: "
                      << test.operations.substr(0, i + 1) << '\n';

            std::cerr << "Expected: " << expected << '\n';
            std::cerr << "Got:      " << got << '\n';

            std::exit(1);
        }
    }
}

int main() {
    const vector<TestCase> fixed_tests = {
        {1, {5}, ""},
        {1, {5, 3, 7, 2}, "RRLRL"},
        {2, {5, 3, 7, 2, 9}, "RRLRRLL"},
        {3, {1, 2, 3, 4, 5}, "RRRRLLLL"},
        {3, {5, 4, 3, 2, 1}, "RRRRLLLL"},
        {2, {7, 7, 7, 7, 7}, "RRLRRLL"},
        {1, {5, 10, 0, 20, 3}, "RRLRRLL"},
        {4, {8, 1, 6, 3, 9, 2}, "RRRRRLLLL"},
        {2, {10, 20, 30, 40, 50}, "RLRLRLRL"},
        {3, {0, 0, 1, 1, 2, 2, 0}, "RRRRLRLRLL"}
    };

    for (size_t i = 0; i < fixed_tests.size(); ++i) {
        RunTest(fixed_tests[i], i + 1);
    }

    std::cerr << "10 fixed tests passed\n";

    std::mt19937_64 rng(std::random_device{}());

    auto RandomInt = [&](int lo, int hi) {
        return std::uniform_int_distribution<int>(lo, hi)(rng);
    };

    for (size_t test_id = 1;; ++test_id) {
        TestCase test;

        int n = RandomInt(1, 200);
        test.k = RandomInt(1, 250);

        int value_range;
        switch (RandomInt(0, 4)) {
            case 0: value_range = 0; break;
            case 1: value_range = 3; break;
            case 2: value_range = 100; break;
            case 3: value_range = 1000000; break;
            default: value_range = 1000000000; break;
        }

        for (int i = 0; i < n; ++i) {
            test.numbers.push_back(RandomInt(0, value_range));
        }

        int l = 0;
        int r = 0;

        int max_operations = RandomInt(0, 2 * n - 2);

        for (int i = 0; i < max_operations; ++i) {
            bool can_r = r + 1 < n;
            bool can_l = l < r;

            if (!can_r && !can_l) {
                break;
            }

            bool push;

            if (!can_l) {
                push = true;
            } else if (!can_r) {
                push = false;
            } else {
                push = RandomInt(0, 1) == 0;
            }

            if (push) {
                ++r;
                test.operations += 'R';
            } else {
                ++l;
                test.operations += 'L';
            }
        }

        RunTest(test, test_id + fixed_tests.size());

        if (test_id % 10000 == 0) {
            std::cerr << "Passed " << test_id
                      << " random tests\n";
        }
    }
}
