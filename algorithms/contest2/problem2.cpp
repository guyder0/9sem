#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

struct Player {
    uint32_t eff;
    size_t id;
};

struct TeamResult {
    uint64_t sum;
    size_t left;
    size_t right;
};

using std::vector;

vector<Player> Input() {
    size_t size;
    std::cin >> size;
    vector<Player> players;
    players.reserve(size);

    for (size_t i = 0; i < size; i++) {
        uint32_t player;
        std::cin >> player;
        players.push_back({player, i + 1});
    }
    return players;
}

TeamResult Solve(const vector<Player>& players) {
    if (players.size() == 1) {
        return {players[0].eff, 0, 0};
    }
    size_t left = 0;
    size_t right = 1;
    uint64_t current_sum = players[0].eff + players[1].eff;
    uint32_t two_weakest = current_sum;
    uint64_t max_sum = current_sum;
    size_t max_left = 0;
    size_t max_right = 1;

    while (right + 1 < players.size()) {
        if (two_weakest < players[right + 1].eff) {
            current_sum -= players[left].eff;
            left += 1;
            two_weakest = players[left].eff + players[left + 1].eff;
        } else {
            right += 1;
            current_sum += players[right].eff;
            if (current_sum > max_sum) {
                max_sum = current_sum;
                max_left = left;
                max_right = right;
            }
        }
    }
    return {max_sum, max_left, max_right};
}

void Output(vector<Player>* players, uint64_t sum, size_t left, size_t right) {
    std::cout << sum << "\n";
    std::sort(players->begin() + left, players->begin() + right + 1,
              [](auto p1, auto p2) { return p1.id < p2.id; });
    for (size_t pos = left; pos < right; ++pos) {
        std::cout << players->at(pos).id << " ";
    }
    std::cout << players->at(right).id << "\n";
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto players = Input();
    std::sort(players.begin(), players.end(),
              [](auto p1, auto p2) { return p1.eff < p2.eff; });
    auto [sum, left, right] = Solve(players);
    Output(&players, sum, left, right);
    return 0;
}
#endif
