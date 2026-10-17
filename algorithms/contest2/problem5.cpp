#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

struct Coin {
    uint32_t coord;
    uint32_t lifetime;
};

const uint32_t kInf = -1;

using std::vector;

vector<Coin> Input() {
    size_t num_coins;
    std::cin >> num_coins;
    vector<Coin> coins;
    coins.reserve(num_coins);

    for (size_t i = 0; i < num_coins; i++) {
        Coin coin;
        std::cin >> coin.coord >> coin.lifetime;
        coins.push_back(coin);
    }
    return coins;
}

uint32_t Dist(Coin coin1, Coin coin2) {
    if (coin1.coord > coin2.coord) {
        return coin1.coord - coin2.coord;
    }
    return coin2.coord - coin1.coord;
}

uint32_t CalcCand(uint32_t current_time, Coin from, Coin to) {
    uint32_t cand =
        (current_time == kInf) ? kInf : current_time + Dist(from, to);
    if (cand <= to.lifetime) {
        return cand;
    }
    return kInf;
}

uint32_t Solve(const vector<Coin>& coins) {
    size_t num_coins = coins.size();
    vector<vector<vector<uint32_t>>> dp(
        num_coins,
        vector<vector<uint32_t>>(num_coins, vector<uint32_t>(2, kInf)));

    for (size_t i = 0; i < num_coins; ++i) {
        dp[i][i][0] = 0;
        dp[i][i][1] = 0;
    }
    for (size_t len = 1; len < num_coins; ++len) {
        for (size_t left = 0; left <= num_coins - len; ++left) {
            size_t right = left + len - 1;
            if (left > 0) {
                uint32_t cand1 =
                    CalcCand(dp[left][right][0], coins[left], coins[left - 1]);
                uint32_t cand2 =
                    CalcCand(dp[left][right][1], coins[right], coins[left - 1]);
                dp[left - 1][right][0] = std::min(cand1, cand2);
            }
            if (right < num_coins - 1) {
                uint32_t cand1 =
                    CalcCand(dp[left][right][0], coins[left], coins[right + 1]);
                uint32_t cand2 = CalcCand(dp[left][right][1], coins[right],
                                          coins[right + 1]);
                dp[left][right + 1][1] = std::min(cand1, cand2);
            }
        }
    }
    return std::min(dp[0][num_coins - 1][0], dp[0][num_coins - 1][1]);
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto coins = Input();
    std::sort(coins.begin(), coins.end(),
              [](auto c1, auto c2) { return c1.coord < c2.coord; });
    auto res = Solve(coins);
    if (res != kInf) {
        std::cout << res;
    } else {
        std::cout << "No solution";
    }

    return 0;
}
#endif
