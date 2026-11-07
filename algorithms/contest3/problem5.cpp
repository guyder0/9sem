#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using std::vector;

struct Triangle {
    int sa;
    int sb;
    int sc;
    bool operator==(const Triangle& other) const {
        return this->sa == other.sa && this->sb == other.sb &&
               this->sc == other.sc;
    }
    bool operator!=(const Triangle& other) const { return !(*this == other); }
};

using HashFunc = std::function<size_t(Triangle)>;

constexpr int kHashtableSize = 3;
constexpr int kBucketSize = 3;
constexpr int kPrime = (1U << 31) - 1;

class HashTable {
private:
    HashFunc hash_;
    vector<vector<Triangle>> table_;
    int result_ = 0;

    HashFunc HashFactory(uint32_t pa, uint32_t pb, uint32_t pc, uint32_t pd) {
        auto table_size =
            table_.size();  // чтобы table_, а следовательно и this не
                            // capture-ился в lambda функции
        return [=](auto triangle) {
            uint64_t k1 = static_cast<uint64_t>(triangle.sa);
            uint64_t k2 = static_cast<uint64_t>(triangle.sb);
            uint64_t k3 = static_cast<uint64_t>(triangle.sc);
            return (k1 * pa + k2 * pb + k3 * pc + pd) % kPrime % table_size;
        };
    }

    static bool HasTriangle(vector<Triangle> bucket, Triangle triangle) {
        return std::ranges::any_of(
            bucket, [triangle](auto item) { return item == triangle; });
    }

    void TryHash(const vector<Triangle>& triangles) {
        uint64_t square_size = 0;
        for (auto& bucket : table_) {
            bucket.clear();
        }

        for (auto triangle : triangles) {
            if (square_size >= kBucketSize * triangles.size()) {
                result_ = 0;
                return;
            }

            size_t triangle_hash = hash_(triangle);
            auto& bucket = table_[triangle_hash];
            if (!HasTriangle(bucket, triangle)) {
                result_++;
                square_size += 2 * bucket.size() + 1;
                bucket.push_back(triangle);
            }
        }
    }

public:
    HashTable(vector<Triangle> triangles) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution distr1(1, kPrime - 1);
        std::uniform_int_distribution distr0(0, kPrime - 1);

        table_ = vector<vector<Triangle>>(triangles.size() * kHashtableSize);

        while (result_ == 0) {
            hash_ =
                HashFactory(distr1(gen), distr1(gen), distr1(gen), distr0(gen));
            TryHash(triangles);
        }
    }

    int GetResult() const { return result_; }
};

Triangle MakeCanonical(Triangle tr) {
    Triangle result_tr;
    result_tr.sa = std::min({tr.sa, tr.sb, tr.sc});
    result_tr.sc = std::max({tr.sa, tr.sb, tr.sc});
    result_tr.sb = tr.sa + tr.sb + tr.sc - result_tr.sa - result_tr.sc;

    int gcd = std::gcd(std::gcd(tr.sa, tr.sb), tr.sc);
    result_tr.sa /= gcd;
    result_tr.sb /= gcd;
    result_tr.sc /= gcd;
    return result_tr;
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t num_triangles;
    std::cin >> num_triangles;
    vector<Triangle> triangles(num_triangles);

    for (size_t i = 0; i < num_triangles; ++i) {
        int sa;
        int sb;
        int sc;
        std::cin >> sa >> sb >> sc;
        triangles[i] = MakeCanonical({sa, sb, sc});
    }

    HashTable hash_table(triangles);
    std::cout << hash_table.GetResult();

    return 0;
}
#endif
