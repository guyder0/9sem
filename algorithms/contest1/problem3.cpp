#include <cstdint>
#include <iostream>
#include <vector>

using std::vector;

void Input(vector<vector<int32_t>>* seqs_a, vector<vector<int32_t>>* seqs_b);
void ScanSequences(vector<vector<int32_t>>* seqs, size_t seq_count,
                   size_t seq_len);
void Output(const vector<size_t>& seq);

vector<size_t> Process(const vector<vector<int32_t>>& seqs_a,
                       const vector<vector<int32_t>>& seqs_b);
size_t ProcessRequest(const vector<vector<int32_t>>& seqs_a,
                      const vector<vector<int32_t>>& seqs_b, size_t req_i,
                      size_t req_j);

void Input(vector<vector<int32_t>>* seqs_a, vector<vector<int32_t>>* seqs_b) {
    size_t seq_a_count;
    size_t seq_b_count;
    size_t seq_len;
    std::cin >> seq_a_count >> seq_b_count >> seq_len;
    ScanSequences(seqs_a, seq_a_count, seq_len);
    ScanSequences(seqs_b, seq_b_count, seq_len);
}

void ScanSequences(vector<vector<int32_t>>* seqs, size_t seq_count,
                   size_t seq_len) {
    seqs->reserve(seq_count);
    for (size_t i = 0; i < seq_count; ++i) {
        vector<int32_t> seq;
        seq.reserve(seq_len);
        for (size_t j = 0; j < seq_len; ++j) {
            int32_t elem;
            std::cin >> elem;
            seq.push_back(elem);
        }
        seqs->push_back(seq);
    }
}

void Output(const vector<size_t>& seq) {
    for (auto elem : seq) {
        std::cout << elem << "\n";
    }
}

vector<size_t> Process(const vector<vector<int32_t>>& seqs_a,
                       const vector<vector<int32_t>>& seqs_b) {
    int32_t num_request;
    vector<size_t> answers;
    std::cin >> num_request;
    answers.reserve(num_request);

    for (int32_t i = 0; i < num_request; ++i) {
        size_t req_i;
        size_t req_j;
        std::cin >> req_i >> req_j;
        answers.push_back(ProcessRequest(seqs_a, seqs_b, req_i, req_j));
    }
    return answers;
}

size_t ProcessRequest(const vector<vector<int32_t>>& seqs_a,
                      const vector<vector<int32_t>>& seqs_b, size_t req_i,
                      size_t req_j) {
    const vector<int32_t>& seq_a = seqs_a[req_i - 1];
    const vector<int32_t>& seq_b = seqs_b[req_j - 1];
    size_t left = 0;
    size_t right = seq_a.size();
    size_t mid;

    while (left != right) {
        mid = left + (right - left) / 2;
        if (seq_a[mid] - seq_b[mid] < 0) {
            left = mid + 1;
        } else if (seq_a[mid] - seq_b[mid] > 0) {
            right = mid;
        } else {
            return mid + 1;
        }
    }

    // краевые случаи
    if (left == seq_a.size()) {
        return left;
    }
    if (left == 0) {
        return 1;
    }

    if (seq_b[left - 1] > seq_a[left]) {
        return left + 1;
    }
    return left;
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    vector<vector<int32_t>> seqs_a;
    vector<vector<int32_t>> seqs_b;
    Input(&seqs_a, &seqs_b);
    vector<size_t> answers = Process(seqs_a, seqs_b);
    Output(answers);
    return 0;
}
#endif
