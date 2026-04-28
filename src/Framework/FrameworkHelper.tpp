#include "FrameworkHelper.hpp"

using namespace llvm;

namespace framework {

template <typename Derived, typename LatticeVal>
requires type_traits::isDenseSet<LatticeVal>::value
LatticeVal FrameworkHelper<Derived, LatticeVal>::interserctionOp(const LatticeVal& lhs, const LatticeVal& rhs) const {
        if (lhs.size() == full().size() || rhs.size() == empty().size()) return rhs;
        if (rhs.size() == full().size() || lhs.size() == empty().size()) return lhs;

        LatticeVal result;

        for (const itemType& value : lhs)
            if (rhs.count(value))
                result.insert(value);

        return result;
}

template <typename Derived, typename LatticeVal>
requires type_traits::isDenseSet<LatticeVal>::value
LatticeVal FrameworkHelper<Derived, LatticeVal>::unionOp(const LatticeVal& lhs, const LatticeVal& rhs) const {
    if (lhs.size() == full().size() || rhs.size() == empty().size()) return lhs;
    if (rhs.size() == full().size() || lhs.size() == empty().size()) return rhs;

    LatticeVal result;

    for (const itemType& value : lhs)
        result.insert(value);

    for (const itemType& value : rhs)
        result.insert(value);

    return result;

}

template <typename Derived, typename LatticeVal>
requires type_traits::isDenseSet<LatticeVal>::value
LatticeVal FrameworkHelper<Derived, LatticeVal>::subtractOp(const LatticeVal& lhs, const LatticeVal& rhs) const {
    if (rhs.size() == empty().size()) return lhs;
    if (rhs.size() == full().size() || lhs.size() == empty().size()) return empty();

    LatticeVal result;

    for (const itemType& value : lhs)
        if ( !rhs.count(value))
            result.insert(value);

    return result;

}

}