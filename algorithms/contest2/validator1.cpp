#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include <cassert>

// Отключаем main() из вашего решения и подключаем сам код
#define TESTING
#include "problem1.cpp"

// Генератор случайных чисел
std::mt19937 rng(1337); // для воспроизводимости можно фиксировать seed или использовать std::random_device{}()

int RandomInt(int from, int to) {
    std::uniform_int_distribution<int> dist(from, to);
    return dist(rng);
}

// Наивное (эталонное) решение за O(N * Q)
vector<int> BruteForce(const vector<int>& seq, const std::string& queries) {
    size_t lp = 0;
    size_t rp = 0;
    vector<int> result;

    for (char req : queries) {
        if (req == 'L') {
            lp++;
        } else {
            rp++;
        }
        // Ищем честный максимум на отрезке [lp, rp]
        int current_max = seq[lp];
        for (size_t i = lp + 1; i <= rp; ++i) {
            current_max = std::max(current_max, seq[i]);
        }
        result.push_back(current_max);
    }
    return result;
}

int main() {
    std::cout << "Starting stress testing... Press Ctrl+C to stop.\n";

    uint64_t test_id = 1;

    while (true) {
        // 1. Генерируем размер массива и сам массив
        size_t n = RandomInt(2, 30); // небольшие размеры для быстрой локализации багов
        vector<int> seq(n);
        for (size_t i = 0; i < n; ++i) {
            seq[i] = RandomInt(-50, 50);
        }

        // 2. Генерируем корректную цепочку запросов L и R
        // Гарантируем: lp <= rp и rp < n (окно не пустое и не выходит за границы)
        size_t cur_lp = 0;
        size_t cur_rp = 0;
        std::string queries = "";
        size_t target_reqs = RandomInt(1, 50);

        for (size_t q = 0; q < target_reqs; ++q) {
            std::vector<char> available_moves;
            if (cur_lp < cur_rp) {
                available_moves.push_back('L'); // можно сдвинуть L, не схлопнув окно
            }
            if (cur_rp + 1 < n) {
                available_moves.push_back('R'); // можно расширить R вправо
            }

            if (available_moves.empty()) {
                break; // дальше двигаться некуда
            }

            char chosen = available_moves[RandomInt(0, available_moves.size() - 1)];
            if (chosen == 'L') {
                cur_lp++;
            } else {
                cur_rp++;
            }
            queries += chosen;
        }

        if (queries.empty()) {
            continue;
        }

        size_t num_reqs = queries.size();

        // 3. Считаем эталонный ответ
        vector<int> expected = BruteForce(seq, queries);

        // 4. Подменяем std::cin строковым потоком и запускаем ваше решение
        std::stringstream fake_cin;
        for (char c : queries) {
            fake_cin << c << ' ';
        }

        auto* original_cin_buf = std::cin.rdbuf(fake_cin.rdbuf());
        vector<int> actual = Process(seq, num_reqs);
        std::cin.rdbuf(original_cin_buf); // обязательно возвращаем cin назад

        // 5. Сравниваем результаты
        if (actual != expected) {
            std::cerr << "\n========================================\n";
            std::cerr << "[-] FAILED on test #" << test_id << "!\n";
            std::cerr << "Array size: " << n << "\n";
            std::cerr << "Array: ";
            for (int x : seq) std::cerr << x << " ";
            std::cerr << "\nQueries (" << num_reqs << "): " << queries << "\n";

            std::cerr << "\nExpected: ";
            for (int x : expected) std::cerr << x << " ";

            std::cerr << "\nGot:      ";
            for (int x : actual) std::cerr << x << " ";
            std::cerr << "\n========================================\n";
            return 1;
        }

        // Лог прогресса
        if (test_id % 10000 == 0) {
            std::cout << "[+] " << test_id << " tests passed successfully!\n";
        }
        test_id++;
    }

    return 0;
}
