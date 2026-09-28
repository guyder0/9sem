#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

struct Point {
    int32_t x;
    int32_t y2;
};

using std::vector;

constexpr double kEps = 1e-4;
constexpr int kPrecision = 6;

void Input(vector<Point>* points, size_t* cover);
double Process(const vector<Point>& points, size_t cover);
size_t MaxCoveredPoints(const vector<Point>& points, double r2);

void Input(vector<Point>* points, size_t* cover) {
    size_t num_points;
    size_t cover_value;
    std::cin >> num_points >> cover_value;
    *cover = cover_value;

    int32_t px;
    int32_t py;
    for (size_t i = 0; i < num_points; ++i) {
        std::cin >> px >> py;
        points->push_back(Point{px, py * py});
    }
}

double Process(const vector<Point>& points, size_t cover) {
    double min = -1;
    double max = -1;
    double pred = 1;
    do {
        size_t can_cover = MaxCoveredPoints(points, pred * pred);
        if (can_cover < cover) {
            min = pred;
        } else {
            max = pred;
        }

        if (max == -1) {
            pred *= 2;
        } else if (min == -1) {
            pred *= 1. / 2;
        } else {
            pred = (min + max) / 2;
        }
    } while (min == -1 or max == -1 or max - min > kEps);
    return max;
}

size_t MaxCoveredPoints(const vector<Point>& points, double r2) {
    static vector<double> l_bounds;
    static vector<double> r_bounds;
    l_bounds.clear();
    r_bounds.clear();

    for (auto point : points) {
        if (r2 > point.y2) {
            double bias = std::sqrt(r2 - point.y2);
            l_bounds.push_back(point.x - bias);
            r_bounds.push_back(point.x + bias);
        }
    }
    std::sort(l_bounds.begin(), l_bounds.end());
    std::sort(r_bounds.begin(), r_bounds.end());

    size_t li = 0;
    size_t ri = 0;
    size_t max_covered = 0;
    size_t cur_covered = 0;
    while (li < l_bounds.size()) {  // правые точно не закончатся раньше
        if (l_bounds[li] <= r_bounds[ri]) {
            cur_covered += 1;
            ++li;
        } else {
            cur_covered -= 1;
            ++ri;
        }
        if (cur_covered > max_covered) {
            max_covered = cur_covered;
        }
    }
    return max_covered;
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    vector<Point> points;
    size_t cover;
    Input(&points, &cover);
    double result = Process(points, cover);
    std::cout << std::fixed << std::setprecision(kPrecision) << result;

    return 0;
}
#endif
