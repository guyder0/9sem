#define TESTING
#include "problem2.cpp"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <random>
#include <string>
#include <utility>

using Edge = std::pair<int, int>;
using Edges = vector<Edge>;

struct Test {
    std::string name;
    int n;
    Edges a, b;
};

vector<Vert> Build(int n, const Edges& edges) {
    vector<Vert> v(n + 1);
    for (auto [a, b] : edges) {
        v[a].adj.push_back(b);
        v[b].adj.push_back(a);
        ++v[a].degree;
        ++v[b].degree;
    }
    return v;
}

// Independent canonical encoding for an unrooted tree (AHU).
std::string Encode(int v, int parent, const vector<Vert>& tree) {
    vector<std::string> children;
    for (int u : tree[v].adj)
        if (u != parent) children.push_back(Encode(u, v, tree));
    std::sort(children.begin(), children.end());
    std::string s = "(";
    for (const auto& child : children) s += child;
    return s + ")";
}

vector<int> Centers(const vector<Vert>& tree) {
    int n = static_cast<int>(tree.size()) - 1;
    if (n == 1) return {1};
    vector<int> degree(n + 1);
    queue<int> q;
    for (int i = 1; i <= n; ++i) {
        degree[i] = static_cast<int>(tree[i].adj.size());
        if (degree[i] <= 1) q.push(i);
    }
    int remaining = n;
    while (remaining > 2) {
        int count = static_cast<int>(q.size());
        remaining -= count;
        while (count--) {
            int v = q.front(); q.pop();
            for (int u : tree[v].adj)
                if (--degree[u] == 1) q.push(u);
        }
    }
    vector<int> result;
    while (!q.empty()) { result.push_back(q.front()); q.pop(); }
    return result;
}

std::string Canonical(const vector<Vert>& tree) {
    std::string best;
    bool first = true;
    for (int center : Centers(tree)) {
        auto s = Encode(center, 0, tree);
        if (first || s < best) best = std::move(s);
        first = false;
    }
    return best;
}

bool ValidMapping(const vector<int>& mapping, const vector<Vert>& a,
                  const vector<Vert>& b) {
    int n = static_cast<int>(a.size()) - 1;
    if (static_cast<int>(mapping.size()) != n + 1 || mapping[0] == -1)
        return false;
    vector<bool> seen(n + 1);
    for (int i = 1; i <= n; ++i) {
        if (mapping[i] < 1 || mapping[i] > n || seen[mapping[i]]) return false;
        seen[mapping[i]] = true;
    }
    for (int i = 1; i <= n; ++i) {
        vector<int> mapped;
        for (int u : a[i].adj) mapped.push_back(mapping[u]);
        auto expected = b[mapping[i]].adj;
        std::sort(mapped.begin(), mapped.end());
        std::sort(expected.begin(), expected.end());
        if (mapped != expected) return false;
    }
    return true;
}

vector<int> Run(const vector<Vert>& a, const vector<Vert>& b) {
    int n = static_cast<int>(a.size()) - 1;
    if (n == 1) return {0, 1};
    if (n == 2) return {0, 1, 2};
    auto x = a, y = b;
    current_max_hash = 1;
    auto cx = Process(&x);
    current_max_hash = 1;
    auto cy = Process(&y);
    return MakeIsomorphism(cx, cy, x, y);
}

void Dump(const Test& t, const vector<int>& answer, bool expected) {
    std::cerr << "\nFAILED: " << t.name << "\n"
              << "n=" << t.n << ", expected=" << (expected ? "isomorphic" : "not isomorphic") << "\n"
              << "First tree:\n";
    for (auto [u, v] : t.a) std::cerr << u << ' ' << v << '\n';
    std::cerr << "Second tree:\n";
    for (auto [u, v] : t.b) std::cerr << u << ' ' << v << '\n';
    std::cerr << "result:";
    for (int x : answer) std::cerr << ' ' << x;
    std::cerr << "\n\nReproduction input for problem2.cpp:\n" << t.n << '\n';
    for (auto [u, v] : t.a) std::cerr << u << ' ' << v << '\n';
    for (auto [u, v] : t.b) std::cerr << u << ' ' << v << '\n';
}

void Check(const Test& t) {
    auto a = Build(t.n, t.a), b = Build(t.n, t.b);
    bool expected = Canonical(a) == Canonical(b);
    auto answer = Run(a, b);
    bool says_no = !answer.empty() && answer[0] == -1;
    bool ok = expected ? ValidMapping(answer, a, b) : says_no;
    if (!ok) {
        Dump(t, answer, expected);
        std::exit(1);
    }
}

Edges Path(int n) {
    Edges e;
    for (int i = 2; i <= n; ++i) e.push_back({i - 1, i});
    return e;
}
Edges Star(int n) {
    Edges e;
    for (int i = 2; i <= n; ++i) e.push_back({1, i});
    return e;
}
Edges RandomTree(int n, std::mt19937_64& rng) {
    Edges e;
    for (int i = 2; i <= n; ++i)
        e.push_back({std::uniform_int_distribution<int>(1, i - 1)(rng), i});
    return e;
}
Edges Relabel(Edges edges, int n, std::mt19937_64& rng) {
    vector<int> p(n + 1);
    std::iota(p.begin(), p.end(), 0);
    std::shuffle(p.begin() + 1, p.end(), rng);
    for (auto& [u, v] : edges) { u = p[u]; v = p[v]; }
    std::shuffle(edges.begin(), edges.end(), rng);
    for (auto& [u, v] : edges)
        if (std::uniform_int_distribution<int>(0, 1)(rng)) std::swap(u, v);
    return edges;
}

int main(int argc, char** argv) {
    std::uint64_t seed = argc > 1 ? std::stoull(argv[1]) : std::random_device{}();
    std::mt19937_64 rng(seed);
    std::cerr << "seed=" << seed << '\n';

    const vector<Test> manual = {
        {"single vertex", 1, {}, {}},
        {"two vertices", 2, {{1,2}}, {{2,1}}},
        {"three vertex path", 3, Path(3), {{2,1},{2,3}}},
        {"four vertex path", 4, Path(4), {{1,3},{3,2},{2,4}}},
        {"five vertex path", 5, Path(5), {{3,1},{1,5},{5,2},{2,4}}},
        {"star with five vertices", 5, Star(5), {{3,1},{3,2},{3,4},{3,5}}},
        {"path vs star", 5, Path(5), Star(5)},
        {"balanced binary tree", 7, {{1,2},{1,3},{2,4},{2,5},{3,6},{3,7}},
            {{4,2},{4,6},{2,1},{2,3},{6,5},{6,7}}},
        {"different degree sequences", 6, Path(6), Star(6)},
        {"same degree sequence, different shape", 8,
            {{1,2},{2,3},{3,4},{4,5},{2,6},{3,7},{4,8}},
            {{1,2},{2,3},{3,4},{4,5},{2,6},{2,7},{4,8}}}
    };
    for (const auto& t : manual) {
        Check(t);
        std::cerr << "[OK] " << t.name << '\n';
    }
    std::cerr << "Manual tests passed. Starting infinite randomized tests...\n";
    for (std::uint64_t iteration = 1;; ++iteration) {
        // Mix small cases (frequent corner cases) and larger cases.
        int n = iteration % 5 == 0
            ? std::uniform_int_distribution<int>(2, 200)(rng)
            : std::uniform_int_distribution<int>(1, 30)(rng);
        Edges a;
        switch (iteration % 5) {
            case 0: a = Path(n); break;
            case 1: a = Star(n); break;
            default: a = RandomTree(n, rng); break;
        }
        Edges b;
        if (iteration % 2 == 0) b = Relabel(a, n, rng);
        else {
            b = RandomTree(n, rng);
            b = Relabel(b, n, rng);
        }
        a = Relabel(a, n, rng);
        Check({"random #" + std::to_string(iteration), n, a, b});
        if (iteration % 1000 == 0)
            std::cerr << "[OK] " << iteration << " random tests\n";
    }
}
