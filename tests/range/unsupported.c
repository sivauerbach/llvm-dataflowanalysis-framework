int testLoopWithInternalBranch() {
    int x = 0;

    for (int i = 0; i < 3; i++) {
        x = x + 2;

        if (x == 7) {   // DEAD (x is less then 6)
            return 1;
        }
    }

    if (x != 8) {
        return 1;       // DEAD
    }

    return 0;
}

int testConditionalLoopArithmetic() {
    int x = 0;

    for (int i = 0; i < 5; i++) {
        if (i < 3) {
            x = x + 2;
        } else {
            x = x + 1;
        }
    }

    // x = 2+2+2+1+1 = 8

    if (x == 9) return 1;   // DEAD
    if (x == 8) return 0;

    return 0;
}

int testNestedLoopInteraction() {
    int x = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            x = x + 1;
        }
    }

    // x = 6

    if (x < 6) return 1;   // DEAD
    if (x > 6) return 1;   // DEAD

    return 0;
}