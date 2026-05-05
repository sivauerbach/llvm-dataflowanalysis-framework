#include "SignedRange.hpp"

#include <array>

#include "Utils/ArithmeticUtils.hpp"
#include "AnalysisTypes/Helpers/OverflowableInt.hpp"

int64_t SignedRange::min(size_t sizeInBits) {
    if (sizeInBits >= 64) 
        return std::numeric_limits<int64_t>::min();

    return -(1ULL << (sizeInBits - 1));
}

int64_t SignedRange::max(size_t sizeInBits) {
    if (sizeInBits >= 64) 
        return std::numeric_limits<int64_t>::max();

    return (1ULL << (sizeInBits - 1)) - 1;
}

SignedRange SignedRange::meet(SignedRange r1, SignedRange r2) {
    if (r1.isEmpty()) return r2;
    if (r2.isEmpty()) return r1;

    int64_t newLower = std::min(r1.lower, r2.lower);
    int64_t newUpper = std::max(r1.upper, r2.upper);
    return SignedRange(newLower, newUpper);
}

SignedRange SignedRange::widen(SignedRange newRange, SignedRange oldRange, size_t sizeInBits) {
    if (newRange.isEmpty()) return oldRange;
    if (oldRange.isEmpty()) return newRange;

    return SignedRange(newRange.lower < oldRange.lower ? min(sizeInBits) : oldRange.lower,
                        newRange.upper > oldRange.upper ? max(sizeInBits) : oldRange.upper);
}

SignedRange SignedRange::narrow(SignedRange newRange, SignedRange oldRange, size_t sizeInBits) {
    if (newRange.isEmpty() || oldRange.isEmpty()) return SignedRange(true);

    return SignedRange(oldRange.lower == min(sizeInBits) ? newRange.lower : oldRange.lower,
                        oldRange.upper == max(sizeInBits) ? newRange.upper: oldRange.upper);
}

SignedRange SignedRange::addRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits) {
    int64_t newLower =0, newUpper = 0;

    if (! noSignedWrap) {
        // For starters, we assume only NSW, NUW operations will be used, so we can just return the full range for the result.
    
        return SignedRange(min(sizeInBits), max(sizeInBits));
    }

    if (!willAddOverflow(r1.lower, r2.lower)) {
        if (r1.lower + r2.lower > max(sizeInBits)) {
            return SignedRange(true);
        } else {
            newLower = std::max(r1.lower + r2.lower, min(sizeInBits));
        }
    } else {
        if (r1.lower < 0) {
            newLower = min(sizeInBits);
        } else {
            return SignedRange(true);
        }
    }           
    
    if (!willAddOverflow(r1.upper, r2.upper)) {
        if (r1.upper + r2.upper < min(sizeInBits)) {
            return SignedRange(true);
        } else {
            newUpper = std::min(r1.upper + r2.upper, max(sizeInBits));
        }
    } else {
        if (r1.upper > 0) {
            newUpper = max(sizeInBits);
        } else {
            return SignedRange(true);
        }
    }

    return SignedRange(newLower, newUpper);  
}

SignedRange SignedRange::subRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits) {
    if (! noSignedWrap) {
        // For starters, we assume only NSW, NUW operations will be used, so we can just return the full range for the result.
    
        return SignedRange(min(sizeInBits), max(sizeInBits));
    }
    
    if (min(64) != r2.lower) {
        // Here we wont overflow on -r2.lower 
        return SignedRange::addRanges(r1, SignedRange(-r2.upper, -r2.lower), noSignedWrap, sizeInBits);
    }

    int64_t newLower = 0, newUpper = 0;
    // As r2.lower == min(64), the only case when willSubOverflow is when r1.upper is non-negative, then
    //  the upperbound must be bigger then -min(64)>max(64).
    if (willSubOverflow(r1.upper, r2.lower)) {
        newUpper = max(64);
    } else {
        newUpper = r1.upper - r2.lower;
    }

    if (! willSubOverflow(r1.lower, r2.upper)) {
        if (r1.lower - r2.upper < min(sizeInBits)) {
            return SignedRange(true);
        } else {
            newLower = std::max(r1.lower - r2.upper, min(sizeInBits));
        }
    } else {
        if (r1.lower < 0) {
            newLower = min(sizeInBits);
        } else {
            return SignedRange(true);
        }
    }

    return SignedRange(newLower, newUpper);
}



SignedRange SignedRange::mulRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits) {
    if (! noSignedWrap) {
        // For starters, we assume only NSW, NUW operations will be used, so we can just return the full range for the result.
    
        return SignedRange(min(sizeInBits), max(sizeInBits));
    }

    OverflowableInt upper1 = r1.upper, upper2 = r2.upper, lower1 = r1.lower, lower2 = r2.lower;
    std::array<OverflowableInt, 4> vals{
        upper1 * upper2,
        upper1 * lower2,
        lower1 * upper2,
        lower1 * lower2
    };

    auto lo = std::min_element(vals.begin(), vals.end());
    auto hi = std::max_element(vals.begin(), vals.end());

    if (lo->isPositiveInf() || hi->isNegativeInf()) return SignedRange(true);
 
    return SignedRange(std::max(lo->getValue(), min(sizeInBits)), 
                       std::min(hi->getValue(), max(sizeInBits)));  
}

SignedRange SignedRange::signDivRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits) {
    if (! noSignedWrap) {
        // For starters, we assume only NSW, NUW operations will be used, so we can just return the full range for the result.
    
        // For now we'll just assume there isa nsw on division, as we are working over intergers, 
        //        there should not be an overflow problem.
    }

    OverflowableInt upper1 = r1.upper, upper2 = r2.upper, lower1 = r1.lower, lower2 = r2.lower;
    if (0 == upper2) upper2 = -1;
    if (0 == lower2) lower2 = 1;
        
    std::array<OverflowableInt, 8> vals{        
        upper1 / upper2,
        upper1 / lower2,
        lower1 / upper2,
        lower1 / lower2,
        // The following are redundant is 0 not in [lower2, upper2]  
        upper1 / ((lower2 < 0 && upper2 > 0) ? 1 : upper2),
        upper1 / ((lower2 < 0 && upper2 > 0) ? -1 : lower2),
        lower1 / ((lower2 < 0 && upper2 > 0) ? 1 : upper2),
        lower1 / ((lower2 < 0 && upper2 > 0) ? -1 : lower2)
    };

    auto lo = std::min_element(vals.begin(), vals.end());
    auto hi = std::max_element(vals.begin(), vals.end());

    if (lo->isPositiveInf() || hi->isNegativeInf()) return SignedRange(true);

    return SignedRange(std::max(lo->getValue(), min(sizeInBits)), 
                        std::min(hi->getValue(), max(sizeInBits)));
}

SignedRange SignedRange::singedWrapCast(SignedRange range, size_t sizeInBits) {
    if (range.isEmpty() || sizeInBits == 0) return SignedRange(true);
    if (sizeInBits >= 64) return range; // no change for 64 bit or larger types

    if (range.lower >> sizeInBits == range.upper >> sizeInBits) 
        return SignedRange(range.lower & ((1ULL << sizeInBits) - 1) , range.upper & ((1ULL << sizeInBits) - 1));
    else
        return full(sizeInBits);
}

SignedRange& SignedRange::operator=(const SignedRange& other) {
    if (this != &other) {
        lower = other.lower;
        upper = other.upper;
        empty = other.empty;
    }
    return *this;
}

SignedRange& SignedRange::operator=(SignedRange&& other) noexcept {
    if (this != &other) {
        lower = std::move(other.lower);
        upper = std::move(other.upper);
        empty = std::move(other.empty);
    }
    return *this;
}

std::string SignedRange::getRangeString() const {
    if (empty) return "empty";

    std::string lowerStr = std::to_string(lower), upperStr = std::to_string(upper);

    size_t closeRange = 15;
    auto checkCloseToIntergerMax = [this, closeRange](int64_t lim, size_t power, std::string& old) -> void {
        if (lim < 0 && (lim <= this->min(power) + closeRange) && (lim >= this->min(power))) {
            
            old = "Min(" + std::to_string(power) + " bit)";
            
            if (this->min(power) != lim) {
                old += " + " + std::to_string(lim - this->min(power));
            }
        } else if(lim > 0 && (lim >= this->max(power) - closeRange) && (lim <= this->max(power))) {
            old = "Max(" + std::to_string(power) + " bit)";
            
            if (this->max(power) != lim) {
                old += " - " + std::to_string(this->max(power) - lim);
            }
        }
    };


    for (size_t i = 8; i <= 64; i += 8) {
        checkCloseToIntergerMax(lower, i, lowerStr);
        checkCloseToIntergerMax(upper, i, upperStr);
    }

    return "[" + lowerStr + ", " + upperStr + "]";
}