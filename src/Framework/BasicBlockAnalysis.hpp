#ifndef BASICBLOCK_ANALYSIS_HPP
#define BASICBLOCK_ANALYSIS_HPP

#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/CFG.h>
#include <type_traits>

#include "DataflowAnalysis.hpp"

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
template <typename Derived, typename LatticeValT, 
            PASS_TYPE PassType>
class BasicBlockAnalysis: public DataflowAnalysis<BasicBlockAnalysis<Derived, LatticeValT, PassType>, BasicBlock*, LatticeValT, PassType> {
private:
    using BaseT = DataflowAnalysis<BasicBlockAnalysis<Derived, LatticeValT, PassType>, BasicBlock*, LatticeValT, PassType>;
    friend BaseT;

    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

protected:  //implementing from DataflowAnalysis

    auto getNodePredecessors(BasicBlock* B, Function*) { return predecessors(B); }
    auto getNodeSuccessors(BasicBlock* B, Function*) { return successors(B); }

    bool isEntryNode(BasicBlock* B, Function* F) { return &F->getEntryBlock() == B; }
    bool isExitNode(BasicBlock* B, Function*) { return succ_begin(B) == succ_end(B); }

    auto getIter(Function* F) { 
        std::vector<BasicBlock *> iter;

        for(BasicBlock& B: *F) {
            iter.push_back(&B);
        }
        
        return iter; 
    }

protected:
    // Interface: (To be implemented by derived class)
    LatticeValT top() const { return derived().top(); } 
    LatticeValT boundary() const { return derived().boundary(); }
    LatticeValT meet(const LatticeValT& lhs, const LatticeValT& rhs) const { return derived().meet(lhs, rhs); }
    LatticeValT transfer(BasicBlock* node, LatticeValT inVal) const { return derived().transfer(node, inVal); }

    // Overridable (have default implementation)
    LatticeValT narrow(LatticeValT outVal, LatticeValT oldOutVal) const 
        { return derived().narrow(outVal, oldOutVal); }
    LatticeValT widen(LatticeValT outVal, LatticeValT oldOutVal, size_t visits) const 
        { return derived().widen(outVal, oldOutVal, visits); }
    LatticeValT getNodePathSensitiveOutput(BasicBlock* node, BasicBlock* parent, LatticeValT parentOutput) 
        { return derived().getNodePathSensitiveOutput(node, parent, parentOutput); }

public:
    void run(Function* function) { this->runImpl(function); }
};    

}

#endif // !BASICBLOCK_ANALYSIS_HPP
