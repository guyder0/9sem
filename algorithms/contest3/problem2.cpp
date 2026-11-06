#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>

using std::queue;
using std::vector;

int current_max_hash;

struct Vert {
    vector<int> adj;
    int hash = -1;
    int degree = 0;
};

struct VertHash {
    int id;
    vector<int> hash_childs;
};

vector<int> ProcessLine(vector<Vert>* verts, const vector<int>& visit) {
    vector<int> prep_visit;
    vector<VertHash> hash_vectors;

    // step 1: form hash_vectors vector for all undeleted leafs
    // that vector contains vectors of hash of childs
    // unique determine subtree
    for (int ivert : visit) {
        Vert& vert = verts->at(ivert);
        vector<int> hash_childs;

        for (int ichild : vert.adj) {
            Vert& child = verts->at(ichild);

            if (child.hash != -1) {
                hash_childs.push_back(child.hash);
                continue;
            }
            // there hash == -1 => unvisited
            if (--(child.degree) == 1) {
                prep_visit.push_back(ichild);
            }
        }
        std::sort(hash_childs.begin(), hash_childs.end());
        hash_vectors.push_back({ivert, hash_childs});
        std::sort(vert.adj.begin(), vert.adj.end(), [verts](int x1, int x2) {
            return verts->at(x1).hash < verts->at(x2).hash;
        });
    }

    // step 2: based on hash_vectors assing unique hash
    std::sort(hash_vectors.begin(), hash_vectors.end(),
              [](const auto& x1, const auto& x2) {
                  return x1.hash_childs < x2.hash_childs;
              });
    int hash = ++current_max_hash;
    for (size_t i = 0; i < hash_vectors.size(); ++i) {
        if (i > 0 &&
            hash_vectors[i].hash_childs != hash_vectors[i - 1].hash_childs) {
            hash = ++current_max_hash;
        }

        verts->at(hash_vectors[i].id).hash = hash;
    }

    return prep_visit;
}

vector<int> Process(vector<Vert>* verts) {
    vector<int> visit;
    for (size_t i = 0; i < verts->size(); ++i) {
        Vert& vert = verts->at(i);
        if (vert.adj.size() == 1) {
            vert.hash = 1;
            if (--(verts->at(vert.adj[0]).degree) == 1) {
                visit.push_back(vert.adj[0]);
            }
        }
    }

    while (true) {
        auto new_visit = ProcessLine(verts, visit);
        if (new_visit.empty()) {
            break;
        }
        visit = std::move(new_visit);
    }
    std::sort(visit.begin(), visit.end(), [verts](auto x1, auto x2) {
        return verts->at(x1).hash < verts->at(x2).hash;
    });
    return visit;
}

vector<int> MakeIsomorphism(const vector<int>& centroids_y,
                            const vector<int>& centroids_x,
                            const vector<Vert>& verts_y,
                            const vector<Vert>& verts_x) {
    vector<int> result(verts_y.size(), 0);
    queue<int> y_visit;
    queue<int> x_visit;

    size_t process_verts = 0;

    if (centroids_y.size() != centroids_x.size()) {
        result[0] = -1;
        return result;
    }

    for (auto vert : centroids_y) {
        y_visit.push(vert);
    }
    for (auto vert : centroids_x) {
        x_visit.push(vert);
    }

    while (!y_visit.empty() && !x_visit.empty()) {
        int ivy = y_visit.front();
        Vert vy = verts_y[ivy];
        y_visit.pop();
        int ivx = x_visit.front();
        Vert vx = verts_x[ivx];
        x_visit.pop();

        if (vy.hash != vx.hash || y_visit.size() != x_visit.size() ||
            vy.adj.size() != vx.adj.size()) {
            result[0] = -1;
            return result;
        }

        result[ivy] = ivx;
        size_t skip_first = process_verts <= centroids_y.size() ? 0 : 1;
        for (size_t i = skip_first; i < vy.adj.size(); ++i) {
            y_visit.push(vy.adj[i]);
            x_visit.push(vx.adj[i]);
        }
        process_verts++;
    }

    return result;
}

void Output(const vector<int>& isomorphism) {
    for (size_t i = 1; i < isomorphism.size(); ++i) {
        std::cout << isomorphism[i] << "\n";
    }
}

vector<Vert> Input(size_t len) {
    vector<Vert> verts(len + 1);
    for (size_t i = 0; i < len - 1; ++i) {
        int v1;
        int v2;
        std::cin >> v1 >> v2;
        verts[v1].adj.push_back(v2);
        verts[v1].degree++;
        verts[v2].adj.push_back(v1);
        verts[v2].degree++;
    }
    return verts;
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t len;
    std::cin >> len;
    auto kingdom_y = Input(len);
    auto kingdom_x = Input(len);

    if (len == 1) {
        std::cout << 1 << "\n";
        return 0;
    }
    if (len == 2) {
        std::cout << 1 << "\n" << 2 << "\n";
        return 0;
    }

    current_max_hash = 1;
    auto centroids_y = Process(&kingdom_y);
    current_max_hash = 1;
    auto centroids_x = Process(&kingdom_x);

    auto isomorphism =
        MakeIsomorphism(centroids_y, centroids_x, kingdom_y, kingdom_x);
    if (isomorphism[0] == -1) {
        std::cout << -1 << "\n";
    } else {
        Output(isomorphism);
    }

    return 0;
}
#endif
