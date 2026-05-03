#include "RangePass.hpp"

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Constants.h>

PreservedAnalyses RangePass::run(Function& F, FunctionAnalysisManager& FAM) {
    // Run our custom range dataflow     
    RangeAnalysis RA{};
    RA.init(&F);
    RA.run(&F);

    outs() << "==== Function: ";
    F.printAsOperand(outs(), false);
    outs() << " ====\n";

    for (BasicBlock& B : F) {
        std::vector<Instruction *> toErase;

        outs() << "\tBlock: " << B.getName() << "\n";
        for (Instruction& I: B) {
            outs() << "\t\tinstruction: ";
            I.print(outs());
            outs() << "\n";

            outs() << "\t\t\tDenseMap contents:\n";
            for (const auto &Entry : RA.getInstructionRanges(&I)) {
                // getFirst() returns the key, getSecond() returns the value
                outs() << "\t\t\t\tKey: " << Entry.first->getName()
                       << ", Value: " << Entry.second.getRangeString() << "\n";
            }
        }
        outs() << "\n";
    }
    
    outs() << "\n";
    return PreservedAnalyses::all();
}
