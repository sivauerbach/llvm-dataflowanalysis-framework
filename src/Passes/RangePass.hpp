#include "AnalysisTypes/RangeAnalysis.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/BasicBlock.h>


// Use the Range Dataflow Analysis to Pass over CFG and Print
struct RangePass : public PassInfoMixin<RangePass> {
private:
    void killEmptyRanges(RangeAnalysis& RA, Instruction& I);
    void collapseSingletons(RangeAnalysis& RA, Instruction& I);
    bool killUnreachableBraches(RangeAnalysis& RA, Instruction* I);

public:
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM);
    
    // Tell the pass manager we need LoopAnalysis
    static bool isRequired() { return true; }
};