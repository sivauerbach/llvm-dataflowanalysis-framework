#include "RangePass.hpp"

#include <optional>

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;

void killEmptyRanges(RangeAnalysis&, Instruction&) {
}

void RangePass::collapseSingletons(RangeAnalysis& RA, Instruction& I) {
    if (! I.getType()->isIntegerTy() || isa<ConstantInt>(&I)) return;

    auto rangesMap = RA.getInstructionRanges(&I);

    for (unsigned i = 0; i < I.getNumOperands(); ++i) {
        Value* value = I.getOperand(i);
        if (! value->getType()->isIntegerTy() || isa<ConstantInt>(value) 
                || ! rangesMap.contains(value) || ! rangesMap[value].isSingleton()) continue;

        outs() << "\t\tCollaping value of " << value->getName() << " from \"";
        I.print(outs());

        Constant *C = ConstantInt::get(
            cast<IntegerType>(value->getType()),
            rangesMap[value].getLower()
        );

        I.setOperand(i, C);

        outs() << "\" to \"";
        I.print(outs());
        outs() << "\"\n";
    }
}

bool RangePass::killUnreachableBraches(RangeAnalysis& RA, Instruction* I) {
    if (! I) return false;

    // We only care about conditional branches
    auto *br = dyn_cast<BranchInst>(I);
    if (! br || !br->isConditional()) return false;

    auto *icmp = dyn_cast<ICmpInst>(br->getCondition());
    if (! icmp) return false;

    Value *lhsValue = icmp->getOperand(0), *rhsValue = icmp->getOperand(1);

    if (! RA.hasValueRange(I, lhsValue) || ! RA.hasValueRange(I, rhsValue)) return false;

    auto lhsRange = RA.getRange(I, lhsValue), rhsRange = RA.getRange(I, rhsValue);

    outs() << "Range of " << lhsValue->getName() << " is " << static_cast<std::string>(lhsRange) << "\n";
    std::optional<uint8_t> branch = std::nullopt;
    switch (icmp->getPredicate()) {
        case ICmpInst::ICMP_SGT:
            if (lhsRange > rhsRange) {
                branch = 0;
            } else if (lhsRange <= rhsRange) {
                branch = 1;
            }

            break;

        case ICmpInst::ICMP_SGE:
            if (lhsRange >= rhsRange) {
                branch = 0;
            } else if (lhsRange < rhsRange) {
                branch = 1;
            }
            break;

        case ICmpInst::ICMP_SLT:
            if (lhsRange < rhsRange) {
                branch = 0;
            } else if (lhsRange >= rhsRange) {
                branch = 1;
            }
            break;

        case ICmpInst::ICMP_SLE:
            if (lhsRange <= rhsRange) {
                branch = 0;
            } else if (lhsRange > rhsRange) {
                branch = 1;
            }
            break;

        case ICmpInst::ICMP_EQ:
            if (lhsRange.isSingleton() && lhsRange == rhsRange) {
                branch = 0;
            } else if (lhsRange < rhsRange || lhsRange > rhsRange) {
                branch = 1;
            }
            break;

        case ICmpInst::ICMP_NE:
            if (lhsRange < rhsRange || lhsRange > rhsRange) {
                branch = 0;
            } else if (lhsRange.isSingleton() && lhsRange == rhsRange) {
                branch = 1;
            }
            break;

        default:
            break;
    }

    if (! branch) return false;

    outs() << "\t\tKilling unreachable branch from \"";
    I->print(outs());
    outs() << "\" with ranges: lhs - " << lhsValue->getName()  << ": " << static_cast<std::string>(lhsRange) << ", rhs - " << lhsValue->getName()  << ": " << static_cast<std::string>(lhsRange);

    BasicBlock *Dest = br->getSuccessor(*branch);
    auto newInst = BranchInst::Create(Dest, br->getIterator());
    br->eraseFromParent();

    outs() << "\" to \"";
    newInst->print(outs());
    outs() << "\"\n";

    return true;
}

PreservedAnalyses RangePass::run(Function& F, FunctionAnalysisManager& FAM) {
    // Run our custom range dataflow     
    RangeAnalysis RA{};
    RA.init(&F);
    RA.run(&F);

    outs() << "==== RangePass - Function: ";
    F.printAsOperand(outs(), false);
    outs() << " ====\n";

    for (BasicBlock& B : F) {
        outs() << "\tCollapsing singletons in block: " << B.getName() << "\n";
        for (Instruction& I: B) {
            collapseSingletons(RA, I);
        }
        outs() << "\n";
    }

    for (BasicBlock& B : F) {
        outs() << "\tKilling unreachable branches in block: " << B.getName() << "\n";
        for (Instruction& I: B) {
            if (killUnreachableBraches(RA, &I)) break;
        }
        outs() << "\n";
    }
    


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
