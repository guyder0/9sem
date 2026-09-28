#include <cstdint>
#include <iostream>
#include <vector>

using std::vector;

vector<int64_t> Input();
void Output(const vector<int64_t>& seq);

vector<int64_t> Process(const vector<int64_t>& seq);
void CalculateDP(const vector<int64_t>& seq, vector<int64_t>* inc,
                 vector<int64_t>* dec);
vector<int64_t> CalculateSubseq(const vector<int64_t>& seq,
                                const vector<int64_t>& inc,
                                const vector<int64_t>& dec);

vector<int64_t> Input() {
    size_t length;
    vector<int64_t> seq;

    std::cin >> length;
    seq.reserve(length);
    for (size_t i = 0; i < length; i++) {
        int64_t elem;
        std::cin >> elem;
        seq.push_back(elem);
    }
    return seq;
}

void Output(const vector<int64_t>& seq) {
    size_t pos = 0;
    for (; pos + 1 < seq.size(); ++pos) {
        std::cout << seq[pos] << " ";
    }
    std::cout << seq[pos];
}

vector<int64_t> Process(const vector<int64_t>& seq) {
    size_t len = seq.size();
    vector<int64_t> inc(len, 1);
    vector<int64_t> dec(len, 1);

    CalculateDP(seq, &inc, &dec);
    vector<int64_t> subseq = CalculateSubseq(seq, inc, dec);
    return subseq;
}

void CalculateDP(const vector<int64_t>& seq, vector<int64_t>* inc,
                 vector<int64_t>* dec) {
    size_t len = seq.size();
    for (size_t i = len - 1; i-- > 0;) {
        for (size_t j = i + 1; j < len; ++j) {
            if (seq[i] < seq[j] && inc->at(i) <= dec->at(j) + 1) {
                inc->at(i) = dec->at(j) + 1;
            }
            if (seq[i] > seq[j] && dec->at(i) <= inc->at(j) + 1) {
                dec->at(i) = inc->at(j) + 1;
            }
        }
    }
}

vector<int64_t> CalculateSubseq(const vector<int64_t>& seq,
                                const vector<int64_t>& inc,
                                const vector<int64_t>& dec) {
    vector<int64_t> subseq;
    bool increasing;
    size_t start_i = 0;
    for (size_t i = 0; i < seq.size(); ++i) {
        if (seq[i] > seq[0]) {
            increasing = true;
            start_i = i;
            break;
        }
        if (seq[i] < seq[0]) {
            increasing = false;
            start_i = i;
            break;
        }
    }
    subseq.push_back(seq[0]);
    int64_t last = seq[0];

    size_t potential = std::max(inc[0], dec[0]) - 1;
    size_t calc_potential;
    for (size_t i = start_i; i < seq.size(); ++i) {
        calc_potential = increasing ? dec[i] : inc[i];
        if (calc_potential == potential) {
            if ((increasing && seq[i] <= last) ||
                (!increasing && seq[i] >= last)) {
                continue;
            }
            subseq.push_back(seq[i]);
            last = seq[i];
            increasing = !increasing;
            --potential;
        }
    }
    return subseq;
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const auto& seq = Input();
    const auto& subseq = Process(seq);
    Output(subseq);
    return 0;
}
#endif
