#ifndef KILL_GEN_HELPER_HPP
#define KILL_GEN_HELPER_HPP

#include "FrameworkHelper.hpp"
#include <type_traits>

namespace framework {

template <typename Derived, typename NodeT, typename LatticeValT>
requires std::is_pointer_v<NodeT>
class KillGenHelper: public FrameworkHelper<KillGenHelper<Derived, NodeT, LatticeValT>, LatticeValT> {
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }; 

    using BaseT = FrameworkHelper<KillGenHelper<Derived, NodeT, LatticeValT>, LatticeValT>;
    friend BaseT;

    using itemType = BaseT::itemType;

protected:
    template <typename ... Args>
    LatticeValT killGenTransfer(NodeT node, LatticeValT& inVal, Args ... args) const { 
        auto kill = this->unionOp(this->constKill(node, args ...), this->depKill(node, inVal, args ...)),
                gen = this->unionOp(this->constGen(node, args ...), this->depGen(node, inVal, args ...));

        return this->unionOp(this->subtractOp(inVal, kill), gen);
    }

protected:
// Interface:
    template <typename ... Args>
    LatticeValT depGen(NodeT node, LatticeValT& inVal, Args ... args) const { return derived().depGen(node, inVal, args ...); } 
    template <typename ... Args>
    LatticeValT constGen(NodeT node, Args ... args) const { return derived().constGen(node, args ...);  }
    
    template <typename ... Args>
    LatticeValT depKill(NodeT node, LatticeValT& inVal, Args ... args) const { return derived().depKill(node, inVal, args ...); }
    template <typename ... Args>
    LatticeValT constKill(NodeT node, Args ... args) const { return derived().constKill(node, args ...); };

    template <typename ... Args>
    LatticeValT getUniverse(Args ... args) { return derived().getUniverse(args ...); } 
};

}

#endif // ! KILL_GEN_HELPER_HPP
