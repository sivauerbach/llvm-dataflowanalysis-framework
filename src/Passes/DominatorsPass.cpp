#include "DominatorsPass.hpp"

#include <llvm/Support/raw_ostream.h>

PreservedAnalyses DominatorsPass::run(Function& F, FunctionAnalysisManager& FAM) {
    // Run our custom dominator dataflow
    DominatorAnalysis DA{};
    DA.init(&F);
    DA.run(&F);

    
    outs() << "=== DominatorsPass - Function: ";
    F.printAsOperand(outs(), false);
    outs() << " ===\n";

    for (BasicBlock& BB : F) {
        BasicBlock* idom = DA.immediateDominator(&F, &BB);
        outs() << "  Block: " << BB.getName()
                << "  |  idom: "
                << (idom ? idom->getName() : StringRef("(none)"))
                << "\n";
    }
    
    outs() << "\n";
    return PreservedAnalyses::all();
}