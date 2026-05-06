#include "RangePass.hpp"

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Constants.h>

void killEmptyRanges(RangeAnalysis&, Instruction&) {
}

void RangePass::collapseSingletons(RangeAnalysis& RA, Instruction& I) {
    auto rangesMap = RA.getInstructionRanges(&I);

    for (unsigned i = 0; i < I.getNumOperands(); ++i) {
        Value* value = I.getOperand(i);
        if (! value->getType()->isIntegerTy() || isa<ConstantInt>(value) 
                || ! rangesMap.contains(value) || ! rangesMap[value].isSingleton()) continue;

        outs() << "\tCollaping value of " << static_cast<std::string>(rangesMap[value]) << value->getName() << " from ";
        I.print(outs());

        Constant *C = ConstantInt::get(
            cast<IntegerType>(value->getType()),
            rangesMap[value].getLower()
        );

        I.setOperand(i, C);

        outs() << "\t to ";
        I.print(outs());
        outs() << "\n";
    }
}

void RangePass::killUnreachableBraches(RangeAnalysis& RA, Instruction& I) {
    // We only care about conditional branches
    auto *br = dyn_cast<BranchInst>(parent);
    if (! br || !br->isConditional()) return;

    auto *icmp = dyn_cast<ICmpInst>(br->getCondition());
    if (! icmp) return;
}

PreservedAnalyses RangePass::run(Function& F, FunctionAnalysisManager& FAM) {
    // Run our custom range dataflow     
    RangeAnalysis RA{};
    RA.init(&F);
    RA.run(&F);

    outs() << "==== Function: ";
    F.printAsOperand(outs(), false);
    outs() << " ====\n";

    

    for (BasicBlock& B : F) {
        outs() << "\tBlock: " << B.getName() << "\n";
        for (Instruction& I: B) {
            collapseSingletons(RA, I);
        }
    }

    outs() << "\n";

    // for (BasicBlock& B : F) {
    //     std::vector<Instruction *> toErase;

    //     outs() << "\tBlock: " << B.getName() << "\n";
    //     for (Instruction& I: B) {
    //         outs() << "\t\tinstruction: ";
    //         I.print(outs());
    //         outs() << "\n";

    //         outs() << "\t\t\tDenseMap contents:\n";
    //         for (const auto &Entry : RA.getInstructionRanges(&I)) {
    //             // getFirst() returns the key, getSecond() returns the value
    //             outs() << "\t\t\t\tKey: " << Entry.first->getName()
    //                    << ", Value: " << Entry.second.getRangeString() << "\n";
    //         }
    //     }
    //     outs() << "\n";
    // }
    
    // outs() << "\n";
    return PreservedAnalyses::all();
}
