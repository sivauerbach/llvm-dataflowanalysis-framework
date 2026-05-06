#include <stdio.h>

volatile int sink = 0;

/* -------------------------------------------------
 * 1. Nested condition with dependent arithmetic
 * ------------------------------------------------- */
int testNestedDependentBranches() {
    int a = 5;
    int b = a + 3;      // 8
    int c = b - 2;      // 6

    if (a < 10) {
        if (b == 7) {    // DEAD (b = 8)
            return 1;
        }
        if (c == 6) {
            return 0;
        }
    }
    return 0;
}

/* -------------------------------------------------
 * 2. Multi-variable impossible constraint
 * ------------------------------------------------- */
int testMultiVariableImpossibleRelation() {
    int x = 10;
    int y = x - 3;      // 7
    int z = y + 2;      // 9

    if (z < x && z > 15) {   // DEAD (z = 9)
        return 1;
    }
    return 0;
}

/* -------------------------------------------------
 * 3. Loop with bounded induction variable
 * ------------------------------------------------- */
int testLoopBoundedRange() {
    int sum = 0;

    for (int i = 0; i < 5; i++) {
        sum = sum + i;
    }

    // sum = 0+1+2+3+4 = 10

    if (sum != 10) {
        return 1;   // DEAD
    }

    if (sum < 0) {
        return 1;   // DEAD
    }

    return 0;
}

/* -------------------------------------------------
 * 4. Loop + branch inside loop (range accumulation)
 * ------------------------------------------------- */
int testLoopWithInternalBranch() {
    int i;
    for (i = 0; i < 3; i+=2) {
        if (i== 7) {   // DEAD (x is less then 6)
            return 1;
        }
    }

    if (i != 8) {
        return 1;       // DEAD
    }

    return 0;
}

/* -------------------------------------------------
 * 5. Cross-variable constraint elimination
 * ------------------------------------------------- */
int testCrossVariableConstraint() {
    int a = 3;
    int b = a * 2;     // 6
    int c = b + a;     // 9

    if (c == b + 4) {  // 9 == 10 -> DEAD
        return 1;
    }

    if (c == 9 && b == 6) {
        return 0;
    }

    return 0;
}

/* -------------------------------------------------
 * 6. Nested branches with unreachable inner block
 * ------------------------------------------------- */
int testNestedImpossibleInnerBranch() {
    int x = 4;
    int y = x + 2;     // 6

    if (x < 10) {
        if (y < 5) {   // DEAD
            if (x == 4) {
                return 1;
            }
        }

        if (y == 6) {
            if (x == 5) { // DEAD
                return 1;
            }
        }
    }

    return 0;
}

/* -------------------------------------------------
 * 7. Arithmetic chain with contradiction
 * ------------------------------------------------- */
int testArithmeticContradiction() {
    int a = 2;
    int b = a + 3;   // 5
    int c = b * 2;   // 10
    int d = c - 1;   // 9

    if (d < b && d > 20) { // DEAD
        return 1;
    }

    return 0;
}

/* -------------------------------------------------
 * 8. Loop with decreasing variable (bounded)
 * ------------------------------------------------- */
int testDecreasingLoopRange() {
    int x = 20;

    while (x > 10) {
        x = x - 3;
    }

    // possible x values: 20,17,14,11,8

    if (x > 20) return 1;   // DEAD
    if (x < 0) return 1;    // DEAD (given step constraints)

    return 0;
}

/* -------------------------------------------------
 * 9. Mixed multiplication + addition constraints
 * ------------------------------------------------- */
int testMixedConstraints() {
    int a = 3;
    int b = 4;
    int c = a * b;     // 12
    int d = c + a;     // 15

    if (d == 14) return 1;  // DEAD

    if (d == 15 && c == 12) {
        return 0;
    }

    return 0;
}

/* -------------------------------------------------
 * 10. Loop invariant propagation test
 * ------------------------------------------------- */
int testLoopInvariantPropagation() {
    int x = 5;

    for (int i = 0; i < 3; i++) {
        int y = x + 2;   // always 7
        if (y == 8) {     // DEAD
            return 1;
        }
    }

    return 0;
}

/* -------------------------------------------------
 * 1. Deep nested branches with arithmetic propagation
 * ------------------------------------------------- */
int testDeepNestedBranches() {
    int a = 2;
    int b = a + 3;     // 5
    int c = b * 2;     // 10
    int d = c - 1;     // 9

    if (a < 10) {
        if (b > 4) {
            if (c == 10) {
                if (d == 8) {   // DEAD
                    return 1;
                }
            }
        }
    }

    return 0;
}

/* -------------------------------------------------
 * 2. Multi-step reassignment narrowing
 * ------------------------------------------------- */
int testReassignmentNarrowing() {
    int x = 5;
    x = x + 3;   // 8
    x = x - 2;   // 6
    x = x * 2;   // 12

    if (x == 11) return 1;  // DEAD
    if (x == 12) return 0;

    return 0;
}

/* -------------------------------------------------
 * 3. Loop producing exact affine result
 * ------------------------------------------------- */
int testExactLoopAccumulation() {
    int sum = 0;

    for (int i = 1; i <= 4; i++) {
        sum = sum + i;
    }

    // sum = 10

    if (sum < 10) return 1;  // DEAD
    if (sum > 10) return 1;  // DEAD
    if (sum != 10) return 1; // DEAD

    return 0;
}

/* -------------------------------------------------
 * 5. Cross-variable arithmetic chain (no var-var compare)
 * ------------------------------------------------- */
int testCrossVariableChain() {
    int a = 3;
    int b = a + 4;     // 7
    int c = b + 1;     // 8
    int d = c * 2;     // 16

    if (d == 15) return 1;  // DEAD
    if (d == 16) return 0;

    return 0;
}

/* -------------------------------------------------
 * 6. Nested impossible branches after propagation
 * ------------------------------------------------- */
int testImpossibleNestedRange() {
    int x = 4;
    int y = x + 2;     // 6
    int z = y - 1;     // 5

    if (x < 10) {
        if (y == 6) {
            if (z == 6) {   // DEAD
                return 1;
            }
        }
    }

    return 0;
}

/* -------------------------------------------------
 * 7. Loop with decreasing monotonic constraint
 * ------------------------------------------------- */
int testDecreasingLoop() {
    int x = 30;

    while (x > 10) {
        x = x - 4;
    }

    // possible: 30,26,22,18,14,10,6

    if (x > 30) return 1;   // DEAD
    if (x == 31) return 1;  // DEAD
    if (x == 6) return 0;

    return 0;
}

/* -------------------------------------------------
 * 8. Arithmetic explosion then narrowing comparison
 * ------------------------------------------------- */
int testArithmeticExplosion() {
    int a = 2;
    int b = a + 2;     // 4
    int c = b + 2;     // 6
    int d = c + 2;     // 8
    int e = d * 2;     // 16

    if (e < 15) return 1;   // DEAD
    if (e > 16) return 1;   // DEAD
    if (e == 16) return 0;

    return 0;
}

/* -------------------------------------------------
 * 9. Branch inside loop creates redundant path
 * ------------------------------------------------- */
int testLoopBranchDeadPath() {
    int x = 1;

    for (int i = 0; i < 3; i++) {
        x = x + 3;

        if (x == 2) {   // DEAD (x = 4,7,10)
            return 1;
        }
    }

    if (x != 10) return 1;  // DEAD

    return 0;
}

/* -------------------------------------------------
 * 10. Mixed arithmetic + reassignment + narrowing
 * ------------------------------------------------- */
int testMixedReassignmentChain() {
    int x = 10;
    x = x - 3;   // 7
    x = x * 2;   // 14
    x = x + 1;   // 15

    if (x == 14) return 1;  // DEAD
    if (x == 15) return 0;

    return 0;
}


/* -------------------------------------------------
 * 12. Chain leading to exact constant trap
 * ------------------------------------------------- */
int testExactConstantTrap() {
    int a = 1;
    int b = a + 1;   // 2
    int c = b + 1;   // 3
    int d = c + 1;   // 4
    int e = d + 1;   // 5

    if (e == 6) return 1;  // DEAD
    if (e == 5) return 0;

    return 0;
}

/* -------------------------------------------------
 * MAIN DRIVER
 * ------------------------------------------------- */
int main() {
    sink += testNestedDependentBranches();
    sink += testMultiVariableImpossibleRelation();
    sink += testLoopBoundedRange();
    sink += testLoopWithInternalBranch();
    sink += testCrossVariableConstraint();
    sink += testNestedImpossibleInnerBranch();
    sink += testArithmeticContradiction();
    sink += testDecreasingLoopRange();
    sink += testMixedConstraints();
    sink += testLoopInvariantPropagation();
    sink += testDeepNestedBranches();
    sink += testReassignmentNarrowing();
    sink += testExactLoopAccumulation();
    sink += testCrossVariableChain();
    sink += testImpossibleNestedRange();
    sink += testDecreasingLoop();
    sink += testArithmeticExplosion();
    sink += testLoopBranchDeadPath();
    sink += testMixedReassignmentChain();
    sink += testExactConstantTrap();

    printf("%d\n", sink);
    return 0;
}