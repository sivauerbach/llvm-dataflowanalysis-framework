#ifndef OVERFLOWABLE_INT_HPP
#define OVERFLOWABLE_INT_HPP

#include<limits>
#include <cstdint>
#include <string>

enum class OVERFLOW {
    NONE,
    OVER,
    UNDER,
    UNDEFIEND
};
class OverflowableInt {
    int64_t value;
    OVERFLOW overflowType;

public:
    friend bool operator==(const OverflowableInt&, const OverflowableInt&);
    friend bool operator!=(const OverflowableInt&, const OverflowableInt&);
    friend bool operator>=(const OverflowableInt&, const OverflowableInt&);
    friend bool operator<=(const OverflowableInt&, const OverflowableInt&);
    friend bool operator>(const OverflowableInt&, const OverflowableInt&);
    friend bool operator<(const OverflowableInt&, const OverflowableInt&);
    
    friend OverflowableInt operator+(const OverflowableInt&, const OverflowableInt&);
    friend OverflowableInt operator-(const OverflowableInt&, const OverflowableInt&);
    friend OverflowableInt operator*(const OverflowableInt&, const OverflowableInt&);
    friend OverflowableInt operator/(const OverflowableInt&, const OverflowableInt&);

public:
    // Implicit castable
    OverflowableInt(int64_t _value): value(_value), overflowType(OVERFLOW::NONE) { }
    OverflowableInt(OVERFLOW _overflowType): value(0), overflowType(_overflowType) { }

    OverflowableInt(int64_t _value, OVERFLOW _overflowType): value(_value), overflowType(_overflowType) { }

    bool isNegativeInf() { return OVERFLOW::UNDER == overflowType; }
    bool isPositiveInf() { return OVERFLOW::OVER == overflowType; }
    bool isWellDefined() { return OVERFLOW::UNDEFIEND != overflowType; }

    auto getValue() { 
        if (OVERFLOW::UNDER == overflowType) return std::numeric_limits<decltype(value)>::min();
        else if (OVERFLOW::OVER == overflowType) return std::numeric_limits<decltype(value)>::max();
        else return value;
    }

    operator std::string() const {
        switch (overflowType) {
            case OVERFLOW::UNDER: return "-inf";
            case OVERFLOW::OVER: return "inf";
            case OVERFLOW::UNDEFIEND: return "UNDEFIEND";
            case OVERFLOW::NONE: return std::to_string(value);
        }
    }
};

#endif // ! OVERFLOWABLE_INT_HPP