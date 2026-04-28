#ifndef DATAFLOW_ANALYSIS_HPP
#define DATAFLOW_ANALYSIS_HPP

#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/CFG.h>
#include <type_traits>

#include "PassTypes.hpp"

using namespace llvm;

namespace framework {

// ============================================================
//  Generic fixed-point dataflow engine
//
//  Derived classes must provide:
//    using LatticeVal = ...;          // type of per-block lattice element
//    LatticeVal boundary();           // value for entry (fwd) or exit (bwd) block
//    LatticeVal top();                // "uninitialized" / top-of-lattice value
//    LatticeVal meet(LatticeVal, LatticeVal);  // meet (or join) operator
//    LatticeVal transfer(BasicBlock*, LatticeVal in); // transfer function
// ============================================================
template <typename Derived, typename NodeT, typename LatticeVal, 
            PASS_TYPE PassType, 
            typename IteratorType = std::vector<NodeT>>
class DataflowAnalysis {
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

    template <typename ... Args>
    void initializeBlocks(Args ... args);

    template <typename ... Args>
    auto getNodePrev(NodeT node, Args ... args) requires type_traits::isForword<PassType>::value { return derived().getNodePredecessors(node, args ...); }
    
    template <typename ... Args>
    auto getNodePrev(NodeT node, Args ... args) requires type_traits::isBackwards<PassType>::value { return derived().getNodeSuccessors(node, args ...); }

protected:
    using LatticeValT = LatticeVal;

    DenseMap<NodeT, LatticeVal> in;
    DenseMap<NodeT , LatticeVal> out;

    template <typename ... Args>
    bool isEdgeNode(NodeT node, Args ... args) requires type_traits::isForword<PassType>::value { return derived().isEntryNode(node, args ...); }
    
    template <typename ... Args>
    bool isEdgeNode(NodeT node, Args ... args) requires type_traits::isBackwards<PassType>::value { return derived().isExitNode(node, args ...); }
    
    LatticeVal& getNodeOutput(NodeT node) requires type_traits::isForword<PassType>::value { return out[node]; }
    LatticeVal& getNodeOutput(NodeT node) requires type_traits::isBackwards<PassType>::value { return in[node]; }

    LatticeVal& getNodeInput(NodeT node) requires type_traits::isForword<PassType>::value { return in[node]; }
    LatticeVal& getNodeInput(NodeT node) requires type_traits::isBackwards<PassType>::value { return out[node]; }

    template <typename ... Args>
    void runImpl(Args ... args);

protected:
    // Interface:
    template <typename ... Args>
    auto getNodePredecessors(NodeT node, Args ... args) { return derived().getNodePredecessors(node, args ...); };
    template <typename ... Args>
    auto getNodeSuccessors(NodeT node, Args ... args) { return derived().getNodeSuccessors(node, args ...); };

    template <typename ... Args>
    bool isEntryNode(NodeT node, Args ... args) { return derived().isEntryNode(node, args ...); }
    template <typename ... Args>
    bool isExitNode(NodeT node, Args ... args) { return derived().isExitNode(node, args ...); }

    template <typename ... Args>
    auto getIter(Args ... args) { return derived().getIter(args ...); }

    LatticeValT top() const { return derived().top(); } 
    LatticeValT boundary() const { return derived().boundary(); }
    LatticeValT meet(const LatticeValT& lhs, const LatticeValT& rhs) const { return derived().meet(lhs, rhs); }
    LatticeValT transfer(NodeT node, LatticeValT inVal) const { return derived().transfer(node, inVal); };
    
public:
    DataflowAnalysis(): in(), out() { };
};    

}

#include "DataflowAnalysis.tpp"

#endif // !DATAFLOW_ANALYSIS_HPP
