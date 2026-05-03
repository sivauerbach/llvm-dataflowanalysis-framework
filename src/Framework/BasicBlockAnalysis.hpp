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
//    using LatticeVal = ...;          // type of per-block lattice element
//    LatticeVal boundary();           // value for entry (fwd) or exit (bwd) block
//    LatticeVal top();                // "uninitialized" / top-of-lattice value
//    LatticeVal meet(LatticeVal, LatticeVal);  // meet (or join) operator
//    LatticeVal transfer(BasicBlock*, LatticeVal in); // transfer function
// ============================================================
template <typename Derived, typename LatticeVal, 
            PASS_TYPE PassType>
class BasicBlockAnalysis: public DataflowAnalysis<BasicBlockAnalysis<Derived, LatticeVal, PassType>, BasicBlock*, LatticeVal, PassType> {
private:
    using BaseT = DataflowAnalysis<BasicBlockAnalysis<Derived, LatticeVal, PassType>, BasicBlock*, LatticeVal, PassType>;
    friend BaseT;

    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

protected:  //implementing from DataflowAnalysis
    using LatticeValT = BaseT::LatticeValT;

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
    LatticeValT transfer(BasicBlock* node, LatticeValT inVal) const { return derived().transfer(node, inVal); };

    LatticeVal getNodePathSensitiveOutput(BasicBlock* node, BasicBlock* parent, LatticeVal parentOutput, Function* function) 
        { return derived().getNodePathSensitiveOutput(node, parent, parentOutput, function); }

public:
    void run(Function* function) { this->runImpl(function); }
};    

}

#endif // !BASICBLOCK_ANALYSIS_HPP
