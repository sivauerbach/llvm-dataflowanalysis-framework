#ifndef DOMINATORS_PASS_HPP
#define DOMINATORS_PASS_HPP

#include "AnalysisTypes/DominatorAnalysis.hpp"

#include <llvm/IR/Function.h>

#include <llvm/IR/PassManager.h>
#include <llvm/IR/BasicBlock.h>


// Use the DOminator Dataflow Analysis to Pass over CFG and Print
struct DominatorsPass : PassInfoMixin<DominatorsPass> {
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);

    // Tell the pass manager we need LoopAnalysis
    static bool isRequired() { return true; }
};

#endif // !DOMINATORS_PASS_HPP