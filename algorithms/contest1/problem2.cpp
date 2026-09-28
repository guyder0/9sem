#include <cstdint>
#include <iostream>
#include <stack>
#include <string>

using std::stack;
using std::string;

string Input();
void Output(int64_t code);

int64_t Process(const string& seq);
bool IsOpenbrace(char brace);
bool CompBraces(char brace1, char brace2);

string Input() {
    string seq;
    std::cin >> seq;
    return seq;
}

void Output(int64_t code) {
    if (code == -1) {
        std::cout << "CORRECT";
    } else {
        std::cout << code;
    }
}

int64_t Process(const string& seq) {
    stack<char> unclosed;
    int64_t correct_steps = 0;

    for (char brace : seq) {
        if (IsOpenbrace(brace)) {
            unclosed.push(brace);
        } else {
            if (unclosed.empty() || !CompBraces(unclosed.top(), brace)) {
                return correct_steps;
            }
            unclosed.pop();
        }
        ++correct_steps;
    }

    if (!unclosed.empty()) {
        return correct_steps;
    }
    return -1;
}

bool IsOpenbrace(char brace) {
    return brace == '(' || brace == '[' || brace == '{';
}

bool CompBraces(char brace1, char brace2) {
    return (brace1 == '(' && brace2 == ')') ||
           (brace1 == '[' && brace2 == ']') || (brace1 == '{' && brace2 == '}');
}

#ifndef TESTING
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const string& seq = Input();
    int64_t code = Process(seq);
    Output(code);
    return 0;
}
#endif
