#include "InstructionAnalysis.hpp"

#include <type_traits>

using namespace llvm;

namespace framework {

template <typename Derived, typename LatticeValT, PASS_TYPE PassType>
std::vector<Instruction*> InstructionAnalysis<Derived, LatticeValT, PassType>::getNodePredecessors(Instruction* I, Function*) { 
    if (! isFirstInstruction(I)) { 
        return std::vector<Instruction*>({I->getPrevNode()}); 
    } else {
        std::vector<Instruction*> vec;
        for (BasicBlock* B: predecessors(I->getParent())) { 
            vec.push_back(&B->back());
        }

        return vec;
    }
}

template <typename Derived, typename LatticeValT, PASS_TYPE PassType>
std::vector<Instruction*> InstructionAnalysis<Derived, LatticeValT, PassType>::getNodeSuccessors(Instruction* I, Function*) { 
    if (! isLastInstruction(I)) { 
        return std::vector<Instruction*>({I->getNextNode()}); 
    } else {
        std::vector<Instruction*> vec;
        for (BasicBlock* B: successors(I->getParent())) { 
            vec.push_back(&B->front());
        }

        return vec;
    }
}

template <typename Derived, typename LatticeValT, PASS_TYPE PassType>
std::vector<Instruction*> InstructionAnalysis<Derived, LatticeValT, PassType>::getIter(Function* F) { 
    std::vector<Instruction *> iter;

    for(BasicBlock& B: *F) {
        for (Instruction& I: B) {
            iter.push_back(&I);
        }
    }
    
    return iter; 
}

}