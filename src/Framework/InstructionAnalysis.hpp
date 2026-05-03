#ifndef INSTRUCTION_ANALYSIS_HPP
#define INSTRUCTION_ANALYSIS_HPP

#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/CFG.h>
#include <type_traits>

#include "DataflowAnalysis.hpp"

using namespace llvm;

namespace framework {

template <typename Derived, typename LatticeVal, PASS_TYPE PassType>
class InstructionAnalysis: public DataflowAnalysis<InstructionAnalysis<Derived, LatticeVal, PassType>, Instruction*, LatticeVal, PassType> {
private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

    using BaseT = DataflowAnalysis<InstructionAnalysis<Derived, LatticeVal, PassType>, Instruction*, LatticeVal, PassType>;
    friend BaseT;

    bool isLastInstruction(Instruction* I) { return &(I->getParent()->back()) == I; }
    bool isFirstInstruction(Instruction* I) { return &(I->getParent()->front()) == I; }

protected: //implementing from DataflowAnalysis
    using LatticeValT = BaseT::LatticeValT;

    std::vector<Instruction*> getNodePredecessors(Instruction* I, Function*);
    std::vector<Instruction*> getNodeSuccessors(Instruction* I, Function*);

    bool isEntryNode(Instruction* I, Function* F) { return &F->getEntryBlock() == I->getParent() && isFirstInstruction(I); }
    bool isExitNode(Instruction* I, Function*) { return succ_begin(I->getParent()) == succ_end(I->getParent()) && isLastInstruction(I); }

    std::vector<Instruction*> getIter(Function* F);

protected: 
    // Interface: (To be implemented by derived class)
    LatticeValT top() const { return derived().top(); } 
    LatticeValT boundary() const { return derived().boundary(); }
    LatticeValT meet(const LatticeValT& lhs, const LatticeValT& rhs) const { return derived().meet(lhs, rhs); }
    LatticeValT transfer(Instruction* node, LatticeValT inVal) const { return derived().transfer(node, inVal); };
    
    LatticeVal getNodePathSensitiveOutput(Instruction* node, Instruction* parent, LatticeVal parentOutput, Function* function) 
        { return derived().getNodePathSensitiveOutput(node, parent, parentOutput, function); }

public:
    void run(Function* function) { this->runImpl(function); }
};    

}

#include "InstructionAnalysis.tpp"

#endif // !INSTRUCTION_ANALYSIS_HPP
