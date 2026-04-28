#ifndef FRAMEOWRK_HELPER_HPP
#define FRAMEOWRK_HELPER_HPP

#include <llvm/ADT/DenseSet.h>
#include <type_traits>

#include "PassTypes.hpp"

using namespace llvm;

namespace framework {
namespace type_traits {
template <typename LatticeVal>
struct isDenseSet: public std::false_type {};

template <typename LatticeVal>
struct isDenseSet<DenseSet<LatticeVal>>: public std::true_type {
    using innerType = LatticeVal;
};
}

template <typename Derived, typename LatticeVal>
requires type_traits::isDenseSet<LatticeVal>::value
struct FrameworkHelper {
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

    LatticeVal universe;

protected:
    using itemType = type_traits::isDenseSet<LatticeVal>::innerType;

    LatticeVal empty() const { return LatticeVal{ }; }
    
    LatticeVal full() { return universe; }
    const LatticeVal& full() const { return universe; }

    LatticeVal unionOp(const LatticeVal& lhs, const LatticeVal& rhs) const;
    LatticeVal interserctionOp(const LatticeVal& lhs, const LatticeVal& rhs) const;
    LatticeVal subtractOp(const LatticeVal& lhs, const LatticeVal& rhs) const;

protected:
// Interface:
    template <typename ... Args>
    LatticeVal getUniverse(Args ... args) { return derived().getUniverse(args ...); } 

public:
    FrameworkHelper(): universe() { };

    template <typename ... Args>
    void init(Args ... args) { universe = getUniverse(args ...); } 
};

}

#include "FrameworkHelper.tpp"

#endif // !FRAMEOWRK_HELPER_HPP