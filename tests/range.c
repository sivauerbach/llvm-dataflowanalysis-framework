// range-test.c
#include <stdint.h>

int test_add_range(int a, int b) {
    int c;
    if (a > 3 && a< 10 && b > 5 && b < 15) {
        c = a + b;
        // expected range for c: [10, 23]
    } else if (a > 0 && a < 5 && b > 0 && b < 5) {
        c = a + b;
        // expected range for c: [2, 8]
    } else {
        c = 20;
        // expected range for c: [20, 20]
    }
    return c;
}


int test_shrink_range(int a, int16_t b, int8_t c) {
    int d = a + b;
    d = d - c;

    return d;
}

int test_phi(int a, int b) {
    int c;
    if (a > 5) {
        c = 21;
        // expected range for c: [21, 21]
    } else {
        c = 20;
        // expected range for c: [20, 20]
    }

    // expected range for c: [20, 21]
    return c;
}

int test_widen(int a) {
    while (a < 100) {
        a = a + 1;
    }
    return a;
}