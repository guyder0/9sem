#include <iostream>
#include <queue>
#include <vector>

using std::queue;
using std::vector;

queue<vector<int>> Input();
void Output(const vector<int>& seq);
vector<int> Merge(const vector<int>& seq1, const vector<int>& seq2);
vector<int> Process(queue<vector<int>>* seqs);

queue<vector<int>> Input() {
    size_t num_seq;
    size_t seq_len;
    queue<vector<int>> seqs;

    std::cin >> num_seq >> seq_len;
    for (size_t i = 0; i < num_seq; i++) {
        vector<int> seq;
        seq.reserve(seq_len);
        for (size_t j = 0; j < seq_len; j++) {
            int elem;
            std::cin >> elem;
            seq.push_back(elem);
        }
        seqs.push(std::move(seq));
    }
    return seqs;
}

void Output(const vector<int>& seq) {
    size_t pos = 0;
    for (; pos + 1 < seq.size(); ++pos) {
        std::cout << seq[pos] << " ";
    }
    std::cout << seq[pos];
}

vector<int> Merge(const vector<int>& seq1, const vector<int>& seq2) {
    vector<int> result;
    size_t s1 = seq1.size();
    size_t s2 = seq2.size();
    size_t i1 = 0;
    size_t i2 = 0;
    result.reserve(s1 + s2);

    while (i1 < s1 && i2 < s2) {
        if (seq1[i1] < seq2[i2]) {
            result.push_back(seq1[i1]);
            ++i1;
        } else {
            result.push_back(seq2[i2]);
            ++i2;
        }
    }
    while (i1 < s1) {
        result.push_back(seq1[i1]);
        ++i1;
    }
    while (i2 < s2) {
        result.push_back(seq2[i2]);
        ++i2;
    }

    return result;
}

vector<int> Process(queue<vector<int>>* seqs) {
    while (seqs->size() > 1) {
        vector<int> seq1 = std::move(seqs->front());
        seqs->pop();
        vector<int> seq2 = std::move(seqs->front());
        seqs->pop();
        seqs->push(Merge(seq1, seq2));
    }
    return seqs->front();
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto seqs = Input();
    auto res = Process(&seqs);
    Output(res);
    return 0;
}
#endif
