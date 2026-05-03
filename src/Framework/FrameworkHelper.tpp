#include "FrameworkHelper.hpp"

using namespace llvm;

namespace framework {

template <typename Derived, typename LatticeValT>
requires type_traits::isDenseSet<LatticeValT>::value
LatticeValT FrameworkHelper<Derived, LatticeValT>::interserctionOp(const LatticeValT& lhs, const LatticeValT& rhs) const {
        if (lhs.size() == full().size() || rhs.size() == empty().size()) return rhs;
        if (rhs.size() == full().size() || lhs.size() == empty().size()) return lhs;

        LatticeValT result;

        for (const itemType& value : lhs)
            if (rhs.count(value))
                result.insert(value);

        return result;
}

template <typename Derived, typename LatticeValT>
requires type_traits::isDenseSet<LatticeValT>::value
LatticeValT FrameworkHelper<Derived, LatticeValT>::unionOp(const LatticeValT& lhs, const LatticeValT& rhs) const {
    if (lhs.size() == full().size() || rhs.size() == empty().size()) return lhs;
    if (rhs.size() == full().size() || lhs.size() == empty().size()) return rhs;

    LatticeValT result;

    for (const itemType& value : lhs)
        result.insert(value);

    for (const itemType& value : rhs)
        result.insert(value);

    return result;

}

template <typename Derived, typename LatticeValT>
requires type_traits::isDenseSet<LatticeValT>::value
LatticeValT FrameworkHelper<Derived, LatticeValT>::subtractOp(const LatticeValT& lhs, const LatticeValT& rhs) const {
    if (rhs.size() == empty().size()) return lhs;
    if (rhs.size() == full().size() || lhs.size() == empty().size()) return empty();

    LatticeValT result;

    for (const itemType& value : lhs)
        if ( !rhs.count(value))
            result.insert(value);

    return result;

}

}