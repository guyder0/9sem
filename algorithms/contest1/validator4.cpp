#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include <cstdint>

#define TESTING
#include "problem4.cpp"

double SolveNaive(const std::vector<Point>& pts, size_t k) {
    if (k == 0) {
        return 0.0;
    }

    double low = 0.0;
    double high = 0.0;
    for (const auto& p : pts) {
        high = std::max(high, std::sqrt((double)p.x * p.x + p.y2));
    }
    high = (high + 1000.0) * 2.0;

    for (int iter = 0; iter < 80; ++iter) {
        double mid = low + (high - low) / 2.0;
        double mid2 = mid * mid;

        std::vector<std::pair<double, int>> events;
        events.reserve(pts.size() * 2);

        for (const auto& p : pts) {
            if ((double)p.y2 > mid2) {
                continue;
            }
            double d = std::sqrt(std::max(0.0, mid2 - p.y2));
            events.emplace_back(p.x - d, -1);
            events.emplace_back(p.x + d, 1);
        }

        std::sort(events.begin(), events.end(), [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
            if (a.first != b.first) {
                return a.first < b.first;
            }
            return a.second < b.second;
        });

        size_t cur = 0;
        bool ok = false;
        for (const auto& ev : events) {
            if (ev.second == -1) {
                cur++;
                if (cur >= k) {
                    ok = true;
                    break;
                }
            } else {
                cur--;
            }
        }

        if (ok) {
            high = mid;
        } else {
            low = mid;
        }
    }

    return (low + high) / 2.0;
}

int main() {
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    size_t test_id = 0;

    while (true) {
        test_id++;

        size_t n;
        if (test_id % 3 == 1) {
            n = std::uniform_int_distribution<size_t>(1, 10)(rng);
        } else if (test_id % 3 == 2) {
            n = std::uniform_int_distribution<size_t>(11, 100)(rng);
        } else {
            n = std::uniform_int_distribution<size_t>(101, 1000)(rng);
        }

        size_t k = std::uniform_int_distribution<size_t>(1, n)(rng);
        std::uniform_int_distribution<int32_t> dist_coord(-1000, 1000);

        std::vector<Point> pts(n);
        for (size_t i = 0; i < n; ++i) {
            int32_t x = dist_coord(rng);
            int32_t y = dist_coord(rng);
            pts[i].x = x;
            pts[i].y2 = y * y;
        }

        double expected = SolveNaive(pts, k);
        double actual = Process(pts, k);

        if (std::isnan(actual) || std::abs(expected - actual) > 1e-3) {
            std::cout << std::fixed << std::setprecision(6);
            std::cout << "FAIL on test #" << test_id << "\n";
            std::cout << "Expected: " << expected << "\n";
            std::cout << "Got: " << actual << "\n";
            std::cout << "Difference: " << std::abs(expected - actual) << "\n";
            std::cout << "Input:\n";
            std::cout << n << " " << k << "\n";
            for (size_t i = 0; i < n; ++i) {
                int32_t y = (int32_t)std::round(std::sqrt(pts[i].y2));
                std::cout << pts[i].x << " " << y << "\n";
            }
            return 1;
        }

        if (test_id % 500 == 0) {
            std::cout << "Passed " << test_id << " tests...\n";
        }
    }

    return 0;
}
