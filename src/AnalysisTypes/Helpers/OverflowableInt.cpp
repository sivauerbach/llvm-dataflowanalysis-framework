#include "OverflowableInt.hpp"

#include<limits>

#include "Utils/ArithmeticUtils.hpp"

bool operator==(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    if (lhs.overflowType != rhs.overflowType) {
        return false;
    }

    if (OVERFLOW::NONE != lhs.overflowType) {
        return true;
    } 
    
    return lhs.value == rhs.value;
}

bool operator<(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    // UNDER is smallest
    if (lhs.overflowType == OVERFLOW::UNDER)
        return rhs.overflowType != OVERFLOW::UNDER;

    if (rhs.overflowType == OVERFLOW::UNDER)
        return false;

    // OVER is largest
    if (rhs.overflowType == OVERFLOW::OVER)
        return lhs.overflowType != OVERFLOW::OVER;

    if (lhs.overflowType == OVERFLOW::OVER)
        return false;

    // Both finite
    return lhs.value < rhs.value;
}

bool operator!=(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    return !(lhs == rhs);
}

bool operator>(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    return rhs < lhs;
}

bool operator<=(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    return !(lhs < rhs);
}

OverflowableInt operator+(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    if (lhs.overflowType == OVERFLOW::UNDEFIEND || rhs.overflowType == OVERFLOW::UNDEFIEND) {
        return OVERFLOW::UNDEFIEND;
    }

    // Propagate infinities
    if ((lhs.overflowType == OVERFLOW::OVER && rhs.overflowType != OVERFLOW::UNDER) ||
            (rhs.overflowType == OVERFLOW::OVER && lhs.overflowType != OVERFLOW::UNDER))
        return OVERFLOW::OVER;

    if ((lhs.overflowType == OVERFLOW::UNDER && rhs.overflowType != OVERFLOW::OVER)  ||
            (rhs.overflowType == OVERFLOW::UNDER && lhs.overflowType != OVERFLOW::OVER))
        return OVERFLOW::UNDER;

    // We are left with one of (including the oppisite pair):
    //      NONE, NONE
    //      UNDER, OVER => UDEF
    if (lhs.overflowType != OVERFLOW::NONE && rhs.overflowType != OVERFLOW::NONE)
        return OVERFLOW::UNDEFIEND;

    if (willAddOverflow(lhs.value, rhs.value)) {
        return (lhs.value >= 0) ? OVERFLOW::OVER : OVERFLOW::UNDER;
    }

    return (lhs.value + rhs.value);
}

OverflowableInt operator-(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    // Note, we treat OVERFLOW::OVER as + inf not as MAX(64), so the only problem is when,
    //  rhs has exactly the value MIN(64).
    if (rhs.overflowType != OVERFLOW::NONE || std::numeric_limits<decltype(rhs.value)>::min() != rhs.value) {
        auto overflowType = rhs.overflowType;
        if (OVERFLOW::OVER == rhs.overflowType) overflowType = OVERFLOW::UNDER;
        else if (OVERFLOW::UNDER == rhs.overflowType) overflowType = OVERFLOW::OVER;
        
        return lhs + OverflowableInt(-rhs.value, overflowType);
    }

    switch (lhs.overflowType) {
        case OVERFLOW::OVER:
            return OVERFLOW::OVER;

        case OVERFLOW::UNDER:
            return OVERFLOW::UNDER;

        case OVERFLOW::UNDEFIEND:
            return OVERFLOW::UNDEFIEND;

        case OVERFLOW::NONE:
            if (0 >= lhs.value) return OVERFLOW::UNDER;
            else return (lhs.value - rhs.value);
    }
}

OverflowableInt operator*(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    // Finite zero dominates
    if ((lhs.overflowType == OVERFLOW::NONE && lhs.value == 0) ||
            (rhs.overflowType == OVERFLOW::NONE && rhs.value == 0))
        return 0;

    if (lhs.overflowType == OVERFLOW::UNDEFIEND || rhs.overflowType == OVERFLOW::UNDEFIEND) {
        return OVERFLOW::UNDEFIEND;
    }

    // Handle infinities
    if (lhs.overflowType != OVERFLOW::NONE || rhs.overflowType != OVERFLOW::NONE) {
        bool negative = false;

        if (lhs.overflowType == OVERFLOW::UNDER)
            negative = !negative;

        if (rhs.overflowType == OVERFLOW::UNDER)
            negative = !negative;

        if (lhs.overflowType == OVERFLOW::NONE && lhs.value < 0)
            negative = !negative;

        if (rhs.overflowType == OVERFLOW::NONE && rhs.value < 0)
            negative = !negative;

        return negative ? OVERFLOW::UNDER : OVERFLOW::OVER;
    }

    if (willMulOverflow(lhs.value, rhs.value)) {
        bool negative = ((lhs.value < 0) != (rhs.value < 0));
        return negative ? OVERFLOW::UNDER : OVERFLOW::OVER;
    }

    return (lhs.value * rhs.value);
}

OverflowableInt operator/(const OverflowableInt& lhs, const OverflowableInt& rhs) {
    if (lhs.overflowType == OVERFLOW::UNDEFIEND || rhs.overflowType == OVERFLOW::UNDEFIEND) {
        return OVERFLOW::UNDEFIEND;
    }
    
    // Division by zero
    if (rhs.overflowType == OVERFLOW::NONE && rhs.value == 0) {
        return OVERFLOW::UNDEFIEND;
    }

    // Finite / infinity = 0
    if (rhs.overflowType != OVERFLOW::NONE && lhs.overflowType == OVERFLOW::NONE)
        return 0;

    // Infinity / finite
    if (lhs.overflowType != OVERFLOW::NONE &&
        rhs.overflowType == OVERFLOW::NONE) {

        bool negative =
            ((lhs.overflowType == OVERFLOW::UNDER) != (rhs.value < 0));

        return negative ? OVERFLOW::UNDER : OVERFLOW::OVER;
    }

    // Infinity / infinity -> undefined
    if (lhs.overflowType != OVERFLOW::NONE &&
        rhs.overflowType != OVERFLOW::NONE) {
            return OVERFLOW::UNDEFIEND;
    }

    if (willAddOverflow(lhs.value, rhs.value)) {
        bool negative = ((lhs.value < 0) != (rhs.value < 0));
        return negative ? OVERFLOW::UNDER : OVERFLOW::OVER;
    }

    return static_cast<decltype(OverflowableInt::value)>(lhs.value / rhs.value);
}