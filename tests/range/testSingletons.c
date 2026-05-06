#include <stdio.h>

volatile int sink = 0;

/* -------------------------------------------------
 * 1. Classic convergence loop (must collapse to 5)
 * ------------------------------------------------- */
int testLoopConvergesToSingleValue() {
    int x = 1;

    while (x < 5) {
        x = x + 1;
    }

    if (x != 5) return 1;  // DEAD

    return 0;
}

/* -------------------------------------------------
 * 2. Conditional skewed convergence
 * ------------------------------------------------- */
int testConditionalConvergence() {
    int x = 0;

    while (x < 10) {
        if (x < 3) {
            x = x + 2;
        } else {
            x = x + 1;
        }
    }

    // only possible fixpoint: 10

    if (x != 10) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 3. Nested loop convergence (outer forces single value)
 * ------------------------------------------------- */
int testNestedLoopSingleton() {
    int x = 0;

    for (int i = 0; i < 3; i++) {
        x = x - 1;
        while (x < 1) {
            x = x + 1;
        }
    }

    // regardless of path, x stabilizes at 6

    if (x != 1) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 4. Branch-driven convergence
 * ------------------------------------------------- */
int testBranchDrivenConvergence() {
    int x = 2;

    while (x < 8) {
        if (x < 5) {
            x = x + 2;
        } else {
            x = x + 1;
        }
    }

    if (x != 8) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 5. Oscillation resolved by bounds
 * ------------------------------------------------- */
int testOscillationToFixedPoint() {
    int x = 1;

    while (x < 10) {
        if (x < 4) {
            x = x + 3;
        } else {
            x = x + 1;
        }
    }

    if (x != 10) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 6. Multi-variable dependency but only one collapses
 * ------------------------------------------------- */
int testDependentCollapseSingleVar() {
    int x = 0;
    int y = 5;

    while (x < y) {
        x = x + 1;
        y = 5; // constant invariant
    }

    if (x != 5) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 7. Nested conditional tightening range
 * ------------------------------------------------- */
int testNestedConditionalTightening() {
    int x = 0;

    while (x < 10) {
        if (x < 5) {
            if (x < 2) {
                x = x + 2;
            } else {
                x = x + 3;
            }
        } else {
            x = x + 5;
        }
    }

    if (x >= 15) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 8. Loop with guarded acceleration
 * ------------------------------------------------- */
int testGuardedAcceleration() {
    int x = 1;

    while (x < 8) {
        if (x < 4) {
            x = x * 2;
        } else {
            x = x + 2;
        }
    }

    if (x != 16) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 9. Converging decreasing loop
 * ------------------------------------------------- */
int testDecreasingConvergence() {
    int x = 20;

    while (x > 7) {
        if (x > 10) {
            x = x - 3;
        } else {
            x = x - 1;
        }
    }

    if (x != 7) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 10. Multi-phase convergence
 * ------------------------------------------------- */
int testMultiPhaseConvergence() {
    int x = 0;

    while (x < 12) {
        if (x < 4) {
            x = x + 2;
        } else if (x < 8) {
            x = x + 3;
        } else {
            x = x + 1;
        }
    }

    if (x >= 12) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 11. Loop invariant forcing singleton result
 * ------------------------------------------------- */
int testInvariantForcesSingleton() {
    int x = 3;

    while (x < 9) {
        int tmp = x + 2;
        x = tmp - 1;
    }

    if (x != 8) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 12. Branch inside loop with constrained growth
 * ------------------------------------------------- */
int testBranchConstrainedGrowth() {
    int x = 0;

    while (x < 6) {
        if (x < 3) {
            x = x + 1;
        } else {
            x = x + 2;
        }
    }

    if (x >= 6) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 13. Hidden convergence via repeated tightening
 * ------------------------------------------------- */
int testHiddenConvergence() {
    int x = 1;

    while (x < 7) {
        if (x < 2) {
            x = x + 3;
        } else if (x < 5) {
            x = x + 2;
        } else {
            x = x + 1;
        }
    }

    if (x != 7) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 14. Interleaved conditional stabilization
 * ------------------------------------------------- */
int testInterleavedStabilization() {
    int x = 2;

    while (x < 12) {
        if (x < 6) {
            x = x + 2;
        }

        if (x >= 6) {
            x = x + 1;
        }
    }

    if (x >= 12) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * 15. Nested loop forcing final single value
 * ------------------------------------------------- */
int testNestedLoopFinalCollapse() {
    int x = 0;

    for (int i = 0; i < 3; i++) {
        int j = 0;

        while (j < 2) {
            x = x + 2;
            j = j + 1;
        }
    }

    if (x != 12) return 1; // DEAD
    return 0;
}

/* -------------------------------------------------
 * MAIN
 * ------------------------------------------------- */
int main() {
    sink += testLoopConvergesToSingleValue();
    sink += testConditionalConvergence();
    sink += testNestedLoopSingleton();
    sink += testBranchDrivenConvergence();
    sink += testOscillationToFixedPoint();
    sink += testDependentCollapseSingleVar();
    sink += testNestedConditionalTightening();
    sink += testGuardedAcceleration();
    sink += testDecreasingConvergence();
    sink += testMultiPhaseConvergence();
    sink += testInvariantForcesSingleton();
    sink += testBranchConstrainedGrowth();
    sink += testHiddenConvergence();
    sink += testInterleavedStabilization();
    sink += testNestedLoopFinalCollapse();

    printf("%d\n", sink);
    return 0;
}