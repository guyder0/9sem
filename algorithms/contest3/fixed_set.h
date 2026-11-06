#include <cstdint>
#include <functional>
#include <random>
#include <stack>
#include <tuple>
#include <vector>

using HashFunc = std::function<int(int)>;
using std::stack;
using std::tuple;
using std::vector;

typedef struct {
    int to;
    int weight;
} Edge;

const int kPrime = (1U << 31) - 1;  // наибольшее простое число в int

HashFunc HashFamily(int pa, int pb, int module) {
    return [=](int obj) {
        if (obj < 0) {
            obj += kPrime;
        }
        uint64_t key = static_cast<uint64_t>(obj);
        return (key * pa + pb) % kPrime % module;
    };
}

bool DfsCycles(int vert, const vector<vector<Edge>>& adj_matrix,
               vector<bool>* visited) {
    stack<tuple<int, int>> vert_stack;
    vert_stack.push({vert, -1});
    while (!vert_stack.empty()) {
        auto [vert, parent] = vert_stack.top();
        vert_stack.pop();
        if (visited->at(vert)) {
            return true;
        }
        visited->at(vert) = true;
        bool parent_seen = false;
        for (auto visit_vert : adj_matrix[vert]) {
            if (visit_vert.to == parent) {
                if (parent_seen) {
                    return true;
                }
                parent_seen = true;
                continue;
            }
            vert_stack.push({visit_vert.to, vert});
        }
    }
    return false;
}

bool HasCycles(const vector<vector<Edge>>& adj_matrix) {
    vector<bool> visited(adj_matrix.size(), false);
    for (size_t i = 0; i < adj_matrix.size(); i++) {
        if (visited[i]) {
            continue;
        }
        if (DfsCycles(static_cast<int>(i), adj_matrix, &visited)) {
            return true;
        }
    }
    return false;
}

void DfsTable(int vert, const vector<vector<Edge>>& adj_matrix,
              vector<int>* table, int mod) {
    stack<tuple<Edge, int>> vert_stack;
    vert_stack.push({{vert, 0}, 0});
    while (!vert_stack.empty()) {
        auto [edge, parent] = vert_stack.top();
        vert_stack.pop();
        table->at(edge.to) = (edge.weight + mod - parent) % mod;
        for (auto visit_vert : adj_matrix[edge.to]) {
            if (table->at(visit_vert.to) != -1) {
                continue;
            }
            vert_stack.push({visit_vert, table->at(edge.to)});
        }
    }
}

vector<int> FillTable(const vector<vector<Edge>>& adj_matrix, int mod) {
    vector<int> table(adj_matrix.size(), -1);
    for (size_t i = 0; i < adj_matrix.size(); i++) {
        if (table[i] == -1) {
            DfsTable(i, adj_matrix, &table, mod);
        }
    }
    return table;
}

class FixedSet {
private:
    static const int kGraphSize = 5;
    vector<int> saved_numbers_;
    HashFunc hashfun1_, hashfun2_;
    vector<int> table_;

public:
    FixedSet() {};
    void Initialize(const vector<int>& numbers);
    bool Contains(int number) const;
};

void FixedSet::Initialize(const vector<int>& numbers) {
    saved_numbers_ = numbers;
    if (saved_numbers_.empty()) {
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution a_distr(1, kPrime - 1);
    std::uniform_int_distribution b_distr(0, kPrime - 1);
    vector<vector<Edge>> adj_matrix(numbers.size() * kGraphSize);

    table_.reserve(numbers.size());

    while (true) {
        for (auto& adj : adj_matrix) {
            adj.clear();
        }

        hashfun1_ =
            HashFamily(a_distr(gen), b_distr(gen), numbers.size() * kGraphSize);
        hashfun2_ =
            HashFamily(a_distr(gen), b_distr(gen), numbers.size() * kGraphSize);
        for (size_t i = 0; i < numbers.size(); ++i) {
            auto number = numbers[i];
            auto hash1 = hashfun1_(number);
            auto hash2 = hashfun2_(number);
            adj_matrix[hash1].push_back({hash2, static_cast<int>(i)});
            adj_matrix[hash2].push_back({hash1, static_cast<int>(i)});
        }
        if (!HasCycles(adj_matrix)) {
            break;
        }
    }
    table_ = FillTable(adj_matrix, saved_numbers_.size());
}

bool FixedSet::Contains(int number) const {
    if (saved_numbers_.empty()) {
        return false;
    }
    int position = (table_[hashfun1_(number)] + table_[hashfun2_(number)]) %
                   saved_numbers_.size();
    return number == saved_numbers_[position];
}
