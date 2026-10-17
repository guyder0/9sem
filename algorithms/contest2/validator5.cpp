#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <csignal>
#include <atomic>
#include <cstdint>

// 1. Активируем макрос TESTING и подключаем ваш файл
#define TESTING
#include "problem5.cpp"

// Флаг остановки по сигналу Ctrl+C
std::atomic<bool> g_stop_requested(false);

void SignalHandler(int signum) {
    if (signum == SIGINT) {
        g_stop_requested.store(true);
    }
}

// ==========================================
// БРУТФОРС: перебор всех N! перестановок
// ==========================================
uint32_t SolveBruteForce(const std::vector<Coin>& coins) {
    size_t n = coins.size();
    if (n == 0) return 0;
    if (n == 1) return 0;

    std::vector<size_t> p(n);
    std::iota(p.begin(), p.end(), 0);

    uint32_t min_time = kInf;

    // Перебираем все возможные порядки первого посещения монет
    do {
        uint32_t current_time = 0;
        bool ok = true;

        for (size_t i = 1; i < n; ++i) {
            uint32_t d = Dist(coins[p[i]], coins[p[i - 1]]);

            // Защита от переполнения uint32_t при сложении
            if (current_time > kInf - d) {
                ok = false;
                break;
            }

            current_time += d;
            if (current_time > coins[p[i]].lifetime) {
                ok = false;
                break;
            }
        }

        if (ok) {
            min_time = std::min(min_time, current_time);
        }
    } while (std::next_permutation(p.begin(), p.end()));

    return min_time;
}

// ==========================================
// ГЕНЕРАТОР ТЕСТОВ
// ==========================================
std::vector<Coin> GenerateTestCase(std::mt19937& rng) {
    // N от 1 до 7 (7! = 5040 операций брутфорса — отрабатывает за доли миллисекунды)
    std::uniform_int_distribution<size_t> dist_n(1, 7);
    std::uniform_int_distribution<uint32_t> dist_coord(0, 40);
    std::uniform_int_distribution<uint32_t> dist_life(0, 70);

    size_t n = dist_n(rng);
    std::vector<Coin> coins(n);
    for (size_t i = 0; i < n; ++i) {
        coins[i].coord = dist_coord(rng);
        coins[i].lifetime = dist_life(rng);
    }
    return coins;
}

// ==========================================
// ТОЧКА ВХОДА ТЕСТЕРА
// ==========================================
int main() {
    std::signal(SIGINT, SignalHandler);

    std::random_device rd;
    std::mt19937 rng(rd());

    uint64_t test_count = 0;
    std::cout << "Starting stress test against brute-force...\n";
    std::cout << "Press Ctrl+C to terminate at any time.\n\n";

    while (!g_stop_requested.load()) {
        ++test_count;

        std::vector<Coin> coins = GenerateTestCase(rng);

        // 1. Считаем эталон брутфорсом (ему порядок координат не важен)
        uint32_t expected = SolveBruteForce(coins);

        // 2. Для вашего Solve монеты обязательно должны быть отсортированы (как в вашем main)
        std::vector<Coin> coins_sorted = coins;
        std::sort(coins_sorted.begin(), coins_sorted.end(), [](auto c1, auto c2) {
            return c1.coord < c2.coord;
        });

        // 3. Вызываем вашу функцию Solve
        uint32_t actual = Solve(coins_sorted);

        // 4. Сравниваем результаты
        if (expected != actual) {
            std::cout << "\n=========================================\n";
            std::cout << "[-] FAILED on test #" << test_count << "!\n";
            std::cout << "=========================================\n";
            std::cout << "Input data (N = " << coins.size() << "):\n";
            std::cout << "coord lifetime\n";
            for (const auto& c : coins) {
                std::cout << c.coord << " " << c.lifetime << "\n";
            }
            std::cout << "-----------------------------------------\n";
            std::cout << "Expected (Brute Force): "
                      << (expected == kInf ? "No solution" : std::to_string(expected)) << "\n";
            std::cout << "Got (Solve):            "
                      << (actual == kInf ? "No solution" : std::to_string(actual)) << "\n";
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
