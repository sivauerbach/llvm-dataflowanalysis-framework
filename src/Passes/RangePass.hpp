#include "AnalysisTypes/RangeAnalysis.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/BasicBlock.h>


// Use the Range Dataflow Analysis to Pass over CFG and Print
struct RangePass : PassInfoMixin<RangePass> {
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);
    
    // Tell the pass manager we need LoopAnalysis
    static bool isRequired() { return true; }
};