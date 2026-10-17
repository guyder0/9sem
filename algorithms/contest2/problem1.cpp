#include <deque>
#include <iostream>
#include <queue>
#include <vector>

using std::deque;
using std::queue;
using std::vector;

vector<int> Input();
void Output(const vector<int>& seq);
vector<int> Process(const vector<int>& seq, size_t num_reqs);

vector<int> Input() {
    size_t length;
    vector<int> seq;

    std::cin >> length;
    seq.reserve(length);
    for (size_t i = 0; i < length; i++) {
        int elem;
        std::cin >> elem;
        seq.push_back(elem);
    }
    return seq;
}

void Output(const vector<int>& seq) {
    size_t pos = 0;
    for (; pos + 1 < seq.size(); ++pos) {
        std::cout << seq[pos] << " ";
    }
    std::cout << seq[pos];
}

vector<int> Process(const vector<int>& seq, size_t num_reqs) {
    size_t rp = 0;  // lp не требуется
    char req;
    queue<int> max_queue;
    deque<int> max_deque;
    vector<int> result;

    max_queue.push(seq[0]);
    max_deque.push_back(seq[0]);

    for (size_t i = 0; i < num_reqs; i++) {
        std::cin >> req;
        if (req == 'L') {
            int popped = max_queue.front();
            max_queue.pop();
            if (popped == max_deque.front()) {
                max_deque.pop_front();
            }
        } else {
            rp += 1;
            int pushed = seq[rp];
            max_queue.push(pushed);
            while (!max_deque.empty() && pushed > max_deque.back()) {
                max_deque.pop_back();
            }
            max_deque.push_back(pushed);
        }
        result.push_back(max_deque.front());
    }

    return result;
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const auto& seq = Input();
    size_t num_reqs;
    std::cin >> num_reqs;
    if (num_reqs > 0) {
        const auto& res = Process(seq, num_reqs);
        Output(res);
    }
    return 0;
}
#endif
