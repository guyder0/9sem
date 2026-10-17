#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <csignal>
#include <atomic>
#include <cstdint>

// 1. Активируем макрос TESTING и подключаем ваше решение
#define TESTING
#include "problem2.cpp"

// Флаг для перехвата сигнала Ctrl+C
std::atomic<bool> g_stop_requested(false);

void SignalHandler(int signum) {
    if (signum == SIGINT) {
        g_stop_requested.store(true);
    }
}

// ==========================================
// ЭТАЛОН: Честный перебор всех 2^N подмножеств
// ==========================================
uint64_t SolveBruteForce(const std::vector<Player>& players) {
    size_t n = players.size();
    if (n == 0) return 0;
    if (n == 1) return players[0].eff;

    uint64_t best_sum = 0;

    // Перебираем все непустые подмножества игроков
    uint32_t total_masks = (1u << n);
    for (uint32_t mask = 1; mask < total_masks; ++mask) {
        std::vector<uint32_t> subset;
        uint64_t current_sum = 0;

        for (size_t i = 0; i < n; ++i) {
            if (mask & (1u << i)) {
                subset.push_back(players[i].eff);
                current_sum += players[i].eff;
            }
        }

        // Проверка сплоченности:
        // Если игроков >= 3, максимальный <= сумме двух минимальных
        bool is_cohesive = true;
        if (subset.size() >= 3) {
            std::sort(subset.begin(), subset.end());
            uint64_t two_weakest = static_cast<uint64_t>(subset[0]) + subset[1];
            if (subset.back() > two_weakest) {
                is_cohesive = false;
            }
        }

        if (is_cohesive && current_sum > best_sum) {
            best_sum = current_sum;
        }
    }

    return best_sum;
}

// ==========================================
// ГЕНЕРАТОР ТЕСТОВ
// ==========================================
std::vector<Player> GenerateTestCase(std::mt19937& rng) {
    // N от 1 до 14 (2^14 = 16384 масок, брутфорс отрабатывает мгновенно)
    std::uniform_int_distribution<size_t> dist_n(1, 14);
    std::uniform_int_distribution<uint32_t> dist_eff(1, 100);

    size_t n = dist_n(rng);
    std::vector<Player> players;
    players.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        players.push_back({ dist_eff(rng), i + 1 });
    }
    return players;
}

// ==========================================
// MAIN ТЕСТЕРА
// ==========================================
int main() {
    std::signal(SIGINT, SignalHandler);

    std::random_device rd;
    std::mt19937 rng(rd());

    uint64_t test_count = 0;
    std::cout << "Starting stress-testing against 2^N brute-force...\n";
    std::cout << "Press Ctrl+C to stop at any time.\n\n";

    while (!g_stop_requested.load()) {
        ++test_count;

        std::vector<Player> test_players = GenerateTestCase(rng);

        // 1. Считаем эталон брутфорсом
        uint64_t expected_sum = SolveBruteForce(test_players);

        // 2. Подготавливаем данные для вашего Solve (сортируем, как в вашем main)
        std::vector<Player> sorted_players = test_players;
        std::sort(sorted_players.begin(), sorted_players.end(), [](auto p1, auto p2) {
            return p1.eff < p2.eff;
        });

        // 3. Вызываем вашу функцию Solve
        auto [actual_sum, actual_left, actual_right] = Solve(sorted_players);

        // 4. Дополнительно проверяем валидность возвращенного диапазона
        bool range_valid = true;
        if (actual_left > actual_right || actual_right >= sorted_players.size()) {
            range_valid = false;
        } else {
            uint64_t real_range_sum = 0;
            for (size_t i = actual_left; i <= actual_right; ++i) {
                real_range_sum += sorted_players[i].eff;
            }
            if (real_range_sum != actual_sum) {
                range_valid = false;
            }
        }

        // 5. Проверяем расхождения
        if (expected_sum != actual_sum || !range_valid) {
            std::cout << "\n=========================================\n";
            std::cout << "[-] FAILED on test #" << test_count << "!\n";
            std::cout << "=========================================\n";
            std::cout << "Input data:\n";
            std::cout << test_players.size() << "\n";
            for (size_t i = 0; i < test_players.size(); ++i) {
                std::cout << test_players[i].eff << (i + 1 == test_players.size() ? "" : " ");
            }
            std::cout << "\n-----------------------------------------\n";
            std::cout << "Expected max sum (Brute Force): " << expected_sum << "\n";
            std::cout << "Got max sum (Solve):            " << actual_sum << "\n";
            std::cout << "Got interval [left, right]:     [" << actual_left << ", " << actual_right << "]\n";
            if (!range_valid) {
                std::cout << "(!) Warning: returned [left, right] interval is invalid or its sum doesn't match max_sum!\n";
            }
            std::cout << "=========================================\n";
            return 1;
        }

        // Вывод прогресса каждые 5 000 тестов
        if (test_count % 5000 == 0) {
            std::cout << "\rPassed " << test_count << " tests..." << std::flush;
        }
    }

    std::cout << "\n\nTesting stopped by user (Ctrl+C).\n";
    std::cout << "Total passed tests: " << test_count << "\n";

    return 0;
}
