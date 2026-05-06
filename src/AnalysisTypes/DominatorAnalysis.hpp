#ifndef DOMINATOR_ANALYSIS_HPP
#define DOMINATOR_ANALYSIS_HPP

#include "Framework/BasicBlockAnalysis.hpp"
#include "Framework/FrameworkHelper.hpp"

using namespace llvm;
using namespace framework;

class DominatorAnalysis
    : public BasicBlockAnalysis<DominatorAnalysis, DenseSet<BasicBlock*>, PASS_TYPE::FORWARDS>,
      public FrameworkHelper<DominatorAnalysis, DenseSet<BasicBlock*>> {
protected:
    friend BasicBlockAnalysis<DominatorAnalysis, DenseSet<BasicBlock*>, PASS_TYPE::FORWARDS>;
    friend FrameworkHelper<DominatorAnalysis, DenseSet<BasicBlock*>>;

    LatticeValT top() const { return this->full(); } 
    LatticeValT boundary() const { return this->empty(); }
    LatticeValT meet(const LatticeValT& lhs, const LatticeValT& rhs) const { return this->interserctionOp(lhs, rhs); } 
    LatticeValT transfer(BasicBlock* B, LatticeValT inVal) const;

    LatticeValT narrow(BasicBlock*, LatticeValT outVal, LatticeValT) const { return outVal; }
    LatticeValT widen(BasicBlock*, LatticeValT outVal, LatticeValT, size_t) const { return outVal; }
    LatticeValT getNodePathSensitiveOutput(BasicBlock*, BasicBlock*, LatticeValT parentOutput) { return parentOutput; }

    LatticeValT getDomain(Function* function) { 
        LatticeValT S;
        
        for (BasicBlock& B : *function) 
            S.insert(&B);
        
        return S;
    }
public:
    DominatorAnalysis(): 
        BasicBlockAnalysis<DominatorAnalysis, DenseSet<BasicBlock*>, PASS_TYPE::FORWARDS>(), 
        FrameworkHelper<DominatorAnalysis, DenseSet<BasicBlock*>>()
    { }

    BasicBlock* immediateDominator(Function* function, BasicBlock* B);
};

#endif // !DOMINATOR_ANALYSIS_HPP