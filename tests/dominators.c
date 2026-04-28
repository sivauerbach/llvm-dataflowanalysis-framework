// dominators.c

#include <stdio.h>

// 1. Simple if-else diamond
int diamond(int x) {
    int result;
    if (x > 0) {
        result = x * 2;
    } else {
        result = -x;
    }
    return result;
}

// 2. Nested if with early return
int nested(int x, int y) {
    if (x < 0) {
        return -1;
    }
    if (y < 0) {
        return -2;
    }
    return x + y;
}

// 3. Loop with break and continue
int loop_break(int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) continue;
        if (i > 10)     break;
        sum += i;
    }
    return sum;
}

// 4. Nested loops
int nested_loops(int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += i * j;
        }
    }
    return sum;
}

// 5. Switch statement
int switch_func(int x) {
    int result = 0;
    switch (x) {
        case 1:  result = 10; break;
        case 2:  result = 20; break;
        case 3:  result = 30; break;
        default: result = -1; break;
    }
    return result;
}

int main() {
    printf("%d\n", diamond(5));
    printf("%d\n", nested(3, 4));
    printf("%d\n", loop_break(15));
    printf("%d\n", nested_loops(4));
    printf("%d\n", switch_func(2));
    return 0;
}