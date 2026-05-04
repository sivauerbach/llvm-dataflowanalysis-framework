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
//    using LatticeValT = ...;          // type of per-block lattice element
//    LatticeValT boundary();           // value for entry (fwd) or exit (bwd) block
//    LatticeValT top();                // "uninitialized" / top-of-lattice value
//    LatticeValT meet(LatticeValT, LatticeValT);  // meet (or join) operator
//    LatticeValT transfer(BasicBlock*, LatticeValT in); // transfer function
// ============================================================
template <typename Derived, typename NodeT, typename LatticeValTemplate, 
            PASS_TYPE PassType, 
            typename IteratorType = std::vector<NodeT>>
class DataflowAnalysis {
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

    enum class PHASE {
        WIDENING,
        NARROWING 
    };

    template <typename ... Args>
    void initializeBlocks(Args ... args);

    template <typename ... Args>
    auto getNodePrev(NodeT node, Args ... args) requires type_traits::isForword<PassType>::value { return derived().getNodePredecessors(node, args ...); }
    
    template <typename ... Args>
    auto getNodePrev(NodeT node, Args ... args) requires type_traits::isBackwards<PassType>::value { return derived().getNodeSuccessors(node, args ...); }

    template <typename ... Args>
    auto getNodeNext(NodeT node, Args ... args) requires type_traits::isForword<PassType>::value { return derived().getNodeSuccessors(node, args ...); }
    
    template <typename ... Args>
    auto getNodeNext(NodeT node, Args ... args) requires type_traits::isBackwards<PassType>::value { return derived().getNodePredecessors(node, args ...); }

    template <typename ... Args>
    void runPhase(PHASE phase, Args ... args);

protected:
    using LatticeValT = LatticeValTemplate;

    DenseMap<NodeT, LatticeValT> in;
    DenseMap<NodeT , LatticeValT> out;

    template <typename ... Args>
    bool isEdgeNode(NodeT node, Args ... args) requires type_traits::isForword<PassType>::value { return derived().isEntryNode(node, args ...); }
    
    template <typename ... Args>
    bool isEdgeNode(NodeT node, Args ... args) requires type_traits::isBackwards<PassType>::value { return derived().isExitNode(node, args ...); }

protected:
    LatticeValT& getNodeOutput(NodeT node) requires type_traits::isForword<PassType>::value { return out[node]; }
    LatticeValT& getNodeOutput(NodeT node) requires type_traits::isBackwards<PassType>::value { return in[node]; }

    LatticeValT& getNodeInput(NodeT node) requires type_traits::isForword<PassType>::value { return in[node]; }
    LatticeValT& getNodeInput(NodeT node) requires type_traits::isBackwards<PassType>::value { return out[node]; }

    template <typename ... Args>
    void runImpl(Args ... args);

protected:
    // Interface: (To be implemented by derived class)
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
    LatticeValT transfer(NodeT node, LatticeValT inVal) const { return derived().transfer(node, inVal); }
    
    LatticeValT narrow(LatticeValT outVal, LatticeValT oldOutVal) const
        { return derived().narrow(outVal, oldOutVal); }
    LatticeValT widen(LatticeValT outVal, LatticeValT oldOutVal, size_t visits) const 
        { return derived().widen(outVal, oldOutVal, visits); }
    LatticeValT getNodePathSensitiveOutput(NodeT node, NodeT parent, LatticeValT parentOutput) 
        { return derived().getNodePathSensitiveOutput(node, parent, parentOutput); }
 
public:
    DataflowAnalysis(): in(), out() { };
};    

}

#include "DataflowAnalysis.tpp"

#endif // !DATAFLOW_ANALYSIS_HPP
