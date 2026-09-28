#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <random>
#include <csignal>
#include <atomic>

#define TESTING
#include "problem2.cpp"

std::atomic<bool> stop_flag{false};

void SigintHandler(int) {
    stop_flag = true;
}

int64_t SolveNaive(const std::string& seq) {
    std::vector<char> st;
    for (size_t i = 0; i < seq.size(); ++i) {
        char c = seq[i];
        if (c == '(' || c == '[' || c == '{') {
            st.push_back(c);
        } else {
            if (st.empty()) {
                return static_cast<int64_t>(i);
            }
            char top = st.back();
            if ((c == ')' && top == '(') ||
                (c == ']' && top == '[') ||
                (c == '}' && top == '{')) {
                st.pop_back();
            } else {
                return static_cast<int64_t>(i);
            }
        }
    }
    if (st.empty()) {
        return -1;
    }
    return static_cast<int64_t>(seq.size());
}

std::string GenerateCBS(std::mt19937_64& rng, int max_depth) {
    if (max_depth <= 0) {
        return "";
    }
    std::uniform_int_distribution<int> type_dist(0, 3);
    int type = type_dist(rng);
    if (type == 0) {
        return "";
    }

    std::uniform_int_distribution<int> bracket_type(0, 2);
    int b = bracket_type(rng);
    char open_b = '(';
    char close_b = ')';
    if (b == 1) {
        open_b = '[';
        close_b = ']';
    } else if (b == 2) {
        open_b = '{';
        close_b = '}';
    }

    std::string inner = GenerateCBS(rng, max_depth - 1);
    std::string outer = GenerateCBS(rng, max_depth - 1);
    return open_b + inner + close_b + outer;
}

int main() {
    std::signal(SIGINT, SigintHandler);

    std::mt19937_64 rng(1337);
    const std::string brackets = "()[]{}";
    std::uniform_int_distribution<int> char_dist(0, 5);
    std::uniform_int_distribution<int> len_dist(1, 100);
    std::uniform_int_distribution<int> mode_dist(0, 4);

    uint64_t test_case = 1;

    while (!stop_flag) {
        std::string s;
        int mode = mode_dist(rng);

        if (mode == 0) {
            int len = len_dist(rng);
            s.resize(len);
            for (int i = 0; i < len; ++i) {
                s[i] = brackets[char_dist(rng)];
            }
        } else if (mode == 1) {
            s = GenerateCBS(rng, 6);
            if (s.empty()) {
                s = "()";
            }
        } else if (mode == 2) {
            s = GenerateCBS(rng, 6);
            if (s.empty()) {
                s = "()";
            }
            std::uniform_int_distribution<int> mut_type(0, 2);
            int mt = mut_type(rng);
            if (mt == 0 && !s.empty()) {
                std::uniform_int_distribution<size_t> idx_dist(0, s.size() - 1);
                s[idx_dist(rng)] = brackets[char_dist(rng)];
            } else if (mt == 1 && s.size() > 1) {
                std::uniform_int_distribution<size_t> pref_dist(1, s.size() - 1);
                s = s.substr(0, pref_dist(rng));
            } else {
                s += brackets[char_dist(rng)];
            }
        } else if (mode == 3) {
            int len = len_dist(rng);
            const std::string open_only = "([{";
            std::uniform_int_distribution<int> open_dist(0, 2);
            s.resize(len);
            for (int i = 0; i < len; ++i) {
                s[i] = open_only[open_dist(rng)];
            }
        } else {
            int len = len_dist(rng);
            const std::string close_only = ")]}";
            std::uniform_int_distribution<int> close_dist(0, 2);
            s.resize(len);
            for (int i = 0; i < len; ++i) {
                s[i] = close_only[close_dist(rng)];
            }
        }

        int64_t expected = SolveNaive(s);
        int64_t actual = Process(s);

        if (actual != expected) {
            std::cout << "Mismatch found on test " << test_case << ":\n";
            std::cout << "Input: \"" << s << "\"\n";
            std::cout << "Length: " << s.size() << "\n";
            std::cout << "Expected: " << expected << "\n";
            std::cout << "Actual:   " << actual << "\n";
            return 1;
        }

        if (test_case % 50000 == 0) {
            std::cout << "Passed " << test_case << " tests\n";
        }

        ++test_case;
    }

    std::cout << "Validation stopped by user. Total tests passed: " << test_case - 1 << "\n";
    return 0;
}
