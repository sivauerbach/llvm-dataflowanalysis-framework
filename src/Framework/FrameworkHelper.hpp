#ifndef FRAMEOWRK_HELPER_HPP
#define FRAMEOWRK_HELPER_HPP

#include <llvm/ADT/DenseSet.h>
#include <type_traits>

#include "PassTypes.hpp"

using namespace llvm;

namespace framework {
namespace type_traits {
template <typename LatticeValT>
struct isDenseSet: public std::false_type {};

template <typename LatticeValT>
struct isDenseSet<DenseSet<LatticeValT>>: public std::true_type {
    using innerType = LatticeValT;
};
}

template <typename Derived, typename LatticeValT>
requires type_traits::isDenseSet<LatticeValT>::value
struct FrameworkHelper {
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

    LatticeValT domain;

protected:
    using itemType = type_traits::isDenseSet<LatticeValT>::innerType;

    LatticeValT empty() const { return LatticeValT{ }; }
    
    LatticeValT full() { return domain; }
    const LatticeValT& full() const { return domain; }

    LatticeValT unionOp(const LatticeValT& lhs, const LatticeValT& rhs) const;
    LatticeValT interserctionOp(const LatticeValT& lhs, const LatticeValT& rhs) const;
    LatticeValT subtractOp(const LatticeValT& lhs, const LatticeValT& rhs) const;

protected:
// Interface:
    template <typename ... Args>
    LatticeValT getDomain(Args ... args) { return derived().getDomain(args ...); } 

public:
    FrameworkHelper(): domain() { };

    template <typename ... Args>
    void init(Args ... args) { domain = getDomain(args ...); } 
};

}

#include "FrameworkHelper.tpp"

#endif // !FRAMEOWRK_HELPER_HPP