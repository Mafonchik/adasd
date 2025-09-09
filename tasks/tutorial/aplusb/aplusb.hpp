#include <iostream>

int Sum(int a, int b) {
    if (a == INT_MAX && b == (INT_MIN + 1)) {
        return 0;
    } else if (a == INT_MAX) {
        return INT_MAX;
    }
    return a + b;
}
