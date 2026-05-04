#ifndef FAINT_ANALYSIS_HPP
#define FAINT_ANALYSIS_HPP

#include "Framework/InstructionAnalysis.hpp"
#include "Framework/FrameworkHelper.hpp"
#include "Framework/KillGenHelper.hpp"

using namespace llvm;
using namespace framework;

class FaintAnalysis
    : public InstructionAnalysis<FaintAnalysis, DenseSet<Value*>, PASS_TYPE::BACKWARDS>,
      public KillGenHelper<FaintAnalysis, Instruction* , DenseSet<Value*>> {
private:
    friend InstructionAnalysis<FaintAnalysis, DenseSet<Value*>, PASS_TYPE::BACKWARDS>;
    friend KillGenHelper<FaintAnalysis, Instruction* , DenseSet<Value*>>;

    LatticeValT top() const { return this->full(); } 
    LatticeValT boundary() const { return this->full(); }
    LatticeValT meet(const LatticeValT& lhs, const LatticeValT& rhs) const { return this->interserctionOp(lhs, rhs); } 

    LatticeValT narrow(LatticeValT outVal, LatticeValT) const { return outVal; }
    LatticeValT widen(LatticeValT outVal, LatticeValT, size_t) const { return outVal; }
    LatticeValT getNodePathSensitiveOutput(Instruction*, Instruction*, LatticeValT parentOutput) { return parentOutput; }

    LatticeValT depGen(Instruction* I, LatticeValT& inVal) const; 
    LatticeValT constGen(Instruction* I) const;
    LatticeValT depKill(Instruction* I, LatticeValT& inVal) const;
    LatticeValT constKill(Instruction* I) const;
    
    LatticeValT getUniverse(Function* function);

    LatticeValT transfer(Instruction* I, LatticeValT& inVal) const { 
        return this->killGenTransfer(I, inVal);
    }

public:
    // Constructer
    FaintAnalysis():
        InstructionAnalysis<FaintAnalysis, DenseSet<Value*>, PASS_TYPE::BACKWARDS>(),
        KillGenHelper<FaintAnalysis, Instruction* , DenseSet<Value*>>()
    { }

    bool isFaint(Instruction* I);

    LatticeValT getFaintsInBlock(Instruction* I);
    bool isFaintInBlock(Instruction* I, Value* V);
};

#endif // !DOMINATOR_ANALYSIS_HPP