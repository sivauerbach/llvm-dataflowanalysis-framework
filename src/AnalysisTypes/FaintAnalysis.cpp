#include "FaintAnalysis.hpp"

#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Instructions.h>

FaintAnalysis::LatticeValT FaintAnalysis::getUniverse(Function* function) {
    LatticeValT universe;

    for (auto& BB : *function) {
        for (auto& I : BB) {
            
            universe.insert(&I);

            for (Use& U : I.operands()) {
                Value* V = U.get();
                if (!V || V->getType()->isVoidTy()) continue;
                if (isa<Instruction>(V) || isa<Argument>(V))
                    universe.insert(V);
            }
        }
    }
    return universe;
}

FaintAnalysis::LatticeValT FaintAnalysis::depGen(Instruction* I, FaintAnalysis::LatticeValT& inVal) const { 
    return {};
}

FaintAnalysis::LatticeValT FaintAnalysis::constGen(Instruction* I) const { 
    LatticeValT constGen;
    
    if (I->getType()->isVoidTy()) {
        constGen.insert(I);
    } else {
        bool selfOperand = false;
        for (Use& U : I->operands()) {
            if (U.get() == I) { selfOperand = true; break;}
        }
        if (!selfOperand)         
            constGen.insert(I);
    }

    return constGen;
}

FaintAnalysis::LatticeValT FaintAnalysis::depKill(Instruction* I, FaintAnalysis::LatticeValT& inVal) const { 
    LatticeValT depKill;

    if (! I->getType()->isVoidTy() && !inVal.contains(I)) {
        for (Use& U : I->operands()) {
            depKill.insert(U.get());
        }
    }

    return depKill;
}

FaintAnalysis::LatticeValT FaintAnalysis::constKill(Instruction* I) const { 
    LatticeValT constKill;
    
    // ConstKill: x is in constKill if instruction USES x
    if (I->getType()->isVoidTy()) {
        for (Use& U : I->operands()) {
            constKill.insert(U.get());
        }
    }
    
    return constKill;
};

// FaintAnalysis::LatticeValT FaintAnalysis::transfer(Instruction* I, FaintAnalysis::LatticeValT inVal) const { 
//     // f(IN) = Gen (IN) U (IN - ConstKill U DepKill(IN))
//     LatticeValT kill, constKill, depKill, gen, outVal;

//     // ConstKill: x is in constKill if instruction USES x
//     if (I->getType()->isVoidTy()) {
//         for (Use& U : I->operands()) {
//             constKill.insert(U.get());
//         }
//     // DepKill:
//     } else if (!inVal.contains(I)) {
//         for (Use& U : I->operands()) {
//             depKill.insert(U.get());
//         }
//     }

//     kill = this->unionOp(depKill, constKill);
    
//     if (I->getType()->isVoidTy()) 
//         gen.insert(I);
//     else {
//         bool selfOperand = false;
//         for (Use& U : I->operands()) {
//             if (U.get() == I) { selfOperand = true; break;}
//         }
//         if (!selfOperand)         
//             gen.insert(I);
//     }

//     outVal = this->unionOp(this->subtractOp(inVal, kill), gen);
//     return outVal;
    
// }

bool FaintAnalysis::isFaint(Instruction* I) {
    if (I->getType()->isVoidTy() || isa<CallInst>(*I) || I->isTerminator())
        return false;

    return out[I].count(I);
}

FaintAnalysis::LatticeValT FaintAnalysis::getFaintsInBlock(Instruction* I) {
    return in[I];
}

bool FaintAnalysis::isFaintInBlock(Instruction* I, Value* V) {
    return getFaintsInBlock(I).contains(V);
}