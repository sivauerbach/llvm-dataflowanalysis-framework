#ifndef KILL_GEN_HELPER_HPP
#define KILL_GEN_HELPER_HPP

#include "FrameworkHelper.hpp"
#include <type_traits>

namespace framework {

template <typename Derived, typename NodeT, typename LatticeVal>
requires std::is_pointer_v<NodeT>
class KillGenHelper: public FrameworkHelper<KillGenHelper<Derived, NodeT, LatticeVal>, LatticeVal> {
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }; 

    using BaseT = FrameworkHelper<KillGenHelper<Derived, NodeT, LatticeVal>, LatticeVal>;
    friend BaseT;

    using itemType = BaseT::itemType;

protected:
    template <typename ... Args>
    LatticeVal killGenTransfer(NodeT node, LatticeVal& inVal, Args ... args) const { 
        auto kill = this->unionOp(this->constKill(node, args ...), this->depKill(node, inVal, args ...)),
                gen = this->unionOp(this->constGen(node, args ...), this->depGen(node, inVal, args ...));

        return this->unionOp(this->subtractOp(inVal, kill), gen);
    }

protected:
// Interface:
    template <typename ... Args>
    LatticeVal depGen(NodeT node, LatticeVal& inVal, Args ... args) const { return derived().depGen(node, inVal, args ...); } 
    template <typename ... Args>
    LatticeVal constGen(NodeT node, Args ... args) const { return derived().constGen(node, args ...);  }
    
    template <typename ... Args>
    LatticeVal depKill(NodeT node, LatticeVal& inVal, Args ... args) const { return derived().depKill(node, inVal, args ...); }
    template <typename ... Args>
    LatticeVal constKill(NodeT node, Args ... args) const { return derived().constKill(node, args ...); };

    template <typename ... Args>
    LatticeVal getUniverse(Args ... args) { return derived().getUniverse(args ...); } 
};

}

#endif // ! KILL_GEN_HELPER_HPP
