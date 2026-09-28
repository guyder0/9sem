#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <algorithm>
#include <sstream>
#include <csignal>
#include <atomic>

#define TESTING
#include "problem3.cpp"

std::atomic<bool> stop_flag{false};

void SigintHandler(int) {
    stop_flag = true;
}

int32_t GetOptimalValue(const std::vector<int32_t>& a, const std::vector<int32_t>& b) {
    int32_t min_max_val = std::max(a[0], b[0]);
    for (size_t t = 1; t < a.size(); ++t) {
        int32_t cur = std::max(a[t], b[t]);
        if (cur < min_max_val) {
            min_max_val = cur;
        }
    }
    return min_max_val;
}

int main() {
    std::signal(SIGINT, SigintHandler);

    std::mt19937_64 rng(1337);
    std::uniform_int_distribution<size_t> n_dist(1, 15);
    std::uniform_int_distribution<size_t> m_dist(1, 15);
    std::uniform_int_distribution<size_t> l_dist(1, 30);
    std::uniform_int_distribution<size_t> q_dist(1, 20);
    std::uniform_int_distribution<int> val_mode(0, 2);
    std::uniform_int_distribution<int32_t> small_val(0, 20);
    std::uniform_int_distribution<int32_t> med_val(0, 1000);
    std::uniform_int_distribution<int32_t> large_val(0, 99999);

    uint64_t test_case = 1;

    while (!stop_flag) {
        size_t n = n_dist(rng);
        size_t m = m_dist(rng);
        size_t l = l_dist(rng);
        size_t q = std::min(q_dist(rng), n * m);

        int mode = val_mode(rng);

        std::vector<std::vector<int32_t>> A(n, std::vector<int32_t>(l));
        for (size_t i = 0; i < n; ++i) {
            for (size_t k = 0; k < l; ++k) {
                if (mode == 0) {
                    A[i][k] = small_val(rng);
                } else if (mode == 1) {
                    A[i][k] = med_val(rng);
                } else {
                    A[i][k] = large_val(rng);
                }
            }
            std::sort(A[i].begin(), A[i].end());
        }

        std::vector<std::vector<int32_t>> B(m, std::vector<int32_t>(l));
        for (size_t j = 0; j < m; ++j) {
            for (size_t k = 0; k < l; ++k) {
                if (mode == 0) {
                    B[j][k] = small_val(rng);
                } else if (mode == 1) {
                    B[j][k] = med_val(rng);
                } else {
                    B[j][k] = large_val(rng);
                }
            }
            std::sort(B[j].begin(), B[j].end(), std::greater<int32_t>());
        }

        std::uniform_int_distribution<size_t> i_dist(1, n);
        std::uniform_int_distribution<size_t> j_dist(1, m);

        std::vector<std::pair<size_t, size_t>> queries(q);
        std::stringstream input_ss;
        input_ss << q << "\n";
        for (size_t idx = 0; idx < q; ++idx) {
            queries[idx] = {i_dist(rng), j_dist(rng)};
            input_ss << queries[idx].first << " " << queries[idx].second << "\n";
        }

        auto* orig_cin_buf = std::cin.rdbuf(input_ss.rdbuf());
        std::vector<size_t> actual = Process(A, B);
        std::cin.rdbuf(orig_cin_buf);

        if (actual.size() != q) {
            std::cout << "Mismatch in output size on test " << test_case << ":\n";
            std::cout << "Expected size: " << q << ", Actual size: " << actual.size() << "\n";
            return 1;
        }

        for (size_t q_idx = 0; q_idx < q; ++q_idx) {
            size_t ans_k = actual[q_idx];
            size_t cur_i = queries[q_idx].first;
            size_t cur_j = queries[q_idx].second;

            if (ans_k < 1 || ans_k > l) {
                std::cout << "Invalid index range on test " << test_case << ", query " << (q_idx + 1) << ":\n";
                std::cout << "Returned k = " << ans_k << ", but valid range is [1, " << l << "]\n";
                return 1;
            }

            int32_t actual_val = std::max(A[cur_i - 1][ans_k - 1], B[cur_j - 1][ans_k - 1]);
            int32_t optimal_val = GetOptimalValue(A[cur_i - 1], B[cur_j - 1]);

            if (actual_val != optimal_val) {
                std::cout << "Suboptimal answer on test " << test_case << ", query " << (q_idx + 1) << ":\n";
                std::cout << "n = " << n << ", m = " << m << ", l = " << l << "\n";
                std::cout << "Query (" << cur_i << ", " << cur_j << ")\n";
                std::cout << "A[" << cur_i << "]: ";
                for (size_t t = 0; t < l; ++t) {
                    std::cout << A[cur_i - 1][t] << (t + 1 == l ? "" : " ");
                }
                std::cout << "\nB[" << cur_j << "]: ";
                for (size_t t = 0; t < l; ++t) {
                    std::cout << B[cur_j - 1][t] << (t + 1 == l ? "" : " ");
                }
                std::cout << "\n";
                std::cout << "Returned k = " << ans_k << " with max(A[k], B[k]) = " << actual_val << "\n";
                std::cout << "Optimal max(A[k], B[k]) = " << optimal_val << "\n";
                return 1;
            }
        }

        if (test_case % 5000 == 0) {
            std::cout << "Passed " << test_case << " tests\n";
        }

        ++test_case;
    }

    std::cout << "Validation stopped by user. Total tests passed: " << test_case - 1 << "\n";
    return 0;
}
