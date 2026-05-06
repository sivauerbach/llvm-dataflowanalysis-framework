// faint-test.c

void foo(int y) {}

// 1. SIMPLE FAINT: x is computed but never used
int simple_faint(int a, int b) {
    int x = a + b;   // faint: x is never used
    int y = a * 2;   // not faint: y is returned
    return y;
}

// 2. CHAIN FAINT: z depends only on faint variables, so z is also faint
int chain_faint(int a, int b) {
    int x = a + b;    // faint: only used by z
    int z = x * 2;    // faint: never used
    int y = a - b;    // not faint: returned
    return y;
}

// 3. MIXED: some branches produce faint vars, others don't
int branch_faint(int a, int b) {
    int x;
    if (a > 0) {
        x = a + 1;    // not faint: x is returned
    } else {
        x = b + 1;    // not faint: x is returned
    }
    int dead = a * b; // faint: never used
    return x;
}

// 4. LOOP FAINT: accumulator computed but never used outside loop
int loop_faint(int n) {
    int dead_sum = 0;
    int i;
    for (i = 0; i < n; i++) {
        dead_sum += i;   // faint: dead_sum never used after loop
    }
    return n * 2;        // not faint
}

// 5. CALL KILLS FAINTNESS: call result is conservative non-faint,
//    but a variable used only to compute a dead value is still faint
int call_faint(int a, int b) {
    int x = a + b;          // faint: only used by dead
    int dead = x * 3;       // faint: never used
    int y = a - b;          // not faint: passed to call
    foo(y);      // call: conservative, never faint
    return 0;
}

// 6. TRANSITIVELY NOT FAINT: all vars feed into a return or call
int no_faint(int a, int b) {
    int x = a + b;    // not faint: used by z
    int y = a * b;    // not faint: used by z
    int z = x + y;    // not faint: returned
    return z;
}

// 7. FAINT IN ONE BRANCH ONLY
int partial_branch_faint(int a, int b, int c) {
    int x = a + b;     // not faint: used in else branch and returned
    int y = b * c;     // not faint: used in if branch
    int dead = a * c;  // faint: never used
    if (a > 0) {
        x = y + 1;
    }
    return x;
}
