#include <iostream>
#include <random>
#include <algorithm>
#include <cassert>

// Отключаем main() из вашего решения и подключаем сам код
#define TESTING
#include "problem3.cpp"

// Генератор случайных чисел
std::mt19937 rng(1337);

int RandomInt(int from, int to) {
    std::uniform_int_distribution<int> dist(from, to);
    return dist(rng);
}

// Эталонное решение: просто объединить всё в один вектор и вызвать std::sort
vector<int> BruteForceSort(const vector<vector<int>>& all_seqs) {
    vector<int> expected;
    for (const auto& seq : all_seqs) {
        for (int x : seq) {
            expected.push_back(x);
        }
    }
    std::sort(expected.begin(), expected.end());
    return expected;
}

int main() {
    std::cout << "Starting stress testing for k-way merge...\n";
    std::cout << "Press Ctrl+C to stop.\n\n";

    uint64_t test_id = 1;

    while (true) {
        // 1. Генерируем параметры: количество последовательностей и их длину
        size_t num_seq = RandomInt(1, 20);   // от 1 до 20 массивов
        size_t seq_len = RandomInt(1, 30);   // от 1 до 30 элементов в каждом

        vector<vector<int>> raw_data(num_seq, vector<int>(seq_len));
        queue<vector<int>> input_queue;

        // 2. Генерируем отсортированные последовательности
        for (size_t i = 0; i < num_seq; ++i) {
            for (size_t j = 0; j < seq_len; ++j) {
                raw_data[i][j] = RandomInt(-100, 100);
            }
            // Каждая последовательность на входе ДОЛЖНА быть отсортирована
            std::sort(raw_data[i].begin(), raw_data[i].end());

            // Копию кладём в очередь для тестируемой функции
            input_queue.push(raw_data[i]);
        }

        // 3. Получаем эталонный ответ
        vector<int> expected = BruteForceSort(raw_data);

        // 4. Запускаем ваше решение
        vector<int> actual = Process(&input_queue);

        // 5. Проверяем совпадение
        if (actual != expected) {
            std::cerr << "\n========================================\n";
            std::cerr << "[-] FAILED on test #" << test_id << "!\n";
            std::cerr << "num_seq = " << num_seq << ", seq_len = " << seq_len << "\n";
            std::cerr << "Input sequences:\n";
            for (size_t i = 0; i < num_seq; ++i) {
                std::cerr << "  seq[" << i << "]: ";
                for (int x : raw_data[i]) std::cerr << x << " ";
                std::cerr << "\n";
            }

            std::cerr << "\nExpected (" << expected.size() << " elems):\n";
            for (int x : expected) std::cerr << x << " ";
            std::cerr << "\n\nGot (" << actual.size() << " elems):\n";
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
