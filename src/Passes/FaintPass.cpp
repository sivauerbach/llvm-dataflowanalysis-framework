#include "FaintPass.hpp"

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Constants.h>

PreservedAnalyses FaintPass::run(Function& F, FunctionAnalysisManager& FAM) {
    // Run our custom dominator dataflow     
    FaintAnalysis FA{};
    FA.init(&F);
    FA.run(&F);

    outs() << "==== Function: ";
    F.printAsOperand(outs(), false);
    outs() << " ====\n";

    for (BasicBlock& B : F) {
        std::vector<Instruction *> toErase;

        outs() << "\tBlock: " << B.getName() << "\n";
        for (Instruction& I: B) {
            if (FA.isFaint(&I)) {
                outs() << "\t\tRemoving faint instruction: ";
                I.print(outs());
                outs() << "\n";

                toErase.push_back(&I);
            }
        }
        outs() << "\n";

        for (Instruction* I : toErase) {
            I->replaceAllUsesWith(UndefValue::get(I->getType()));
            I->eraseFromParent();
        }
    }
    
    outs() << "\n";
    return PreservedAnalyses::all();
}
