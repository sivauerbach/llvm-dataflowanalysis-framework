#include "RangeAnalysis.hpp"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>

SignedRange RangeAnalysis::getRangeFromValue(Value* value, LatticeValT& inVal) {
    if (auto *CI = dyn_cast<ConstantInt>(value)) {
        int64_t value = CI->getValue().getSExtValue();
        return SignedRange(value, value);
    }
    
    if (inVal.contains(value)) {
        return inVal[value];
    }

    return SignedRange(true);
}

RangeAnalysis::LatticeValT RangeAnalysis::meet(const LatticeValT& lhs, const LatticeValT& rhs) const {     
    DenseMap<Value*, SignedRange> result = lhs;
    
    for (auto &entry : rhs) {
        if (result.contains(entry.first)) {
            // call signed range meet only for vars defined in both branches
            result[entry.first] = SignedRange::meet(result[entry.first], entry.second); 
        } else {
            result.insert(entry);
        }
    }
    return result;    
} 

RangeAnalysis::LatticeValT RangeAnalysis::transfer(Instruction* I, LatticeValT inVal) const {
    // non-assignment instruction does not modify the range of any variable
    if (!I || I->getType()->isVoidTy() || !I->getType()->isIntegerTy()) return inVal;

    Value* lhs = I;
    LatticeValT result = inVal;

    if (auto binOp = dyn_cast<BinaryOperator>(I)) {
        // Both operands of the binary operator should be in the inVal map as they must be defined before using them.
        SignedRange rhs1 = getRangeFromValue(binOp->getOperand(0), inVal), rhs2 = getRangeFromValue(binOp->getOperand(1), inVal), 
            resultRange = SignedRange::full(lhs->getType()->getIntegerBitWidth());

        // If either operand has an empty range, the result is also empty, as 
        //      value being empty implies we won't have any concrete value for the lhs.
        // TODO: check if either of the sides are constants , if so then make a constant range for it
        if (rhs1.isEmpty() || rhs2.isEmpty()) 
            resultRange = SignedRange(true);
        else {
            switch (binOp->getOpcode()) {
                case Instruction::Add: 
                    resultRange = SignedRange::addRanges(rhs1, rhs2, binOp->hasNoSignedWrap(), lhs->getType()->getIntegerBitWidth());
                    break;
                case Instruction::Sub: 
                    resultRange = SignedRange::subRanges(rhs1, rhs2, binOp->hasNoSignedWrap(), lhs->getType()->getIntegerBitWidth());

                    break;
                case Instruction::Mul: 
                    resultRange = SignedRange::mulRanges(rhs1, rhs2, binOp->hasNoSignedWrap(), lhs->getType()->getIntegerBitWidth());    

                    break;

                case Instruction::SDiv:
                    resultRange = SignedRange::signDivRanges(rhs1, rhs2, binOp->hasNoSignedWrap(), lhs->getType()->getIntegerBitWidth());

                    break;

                case Instruction::UDiv:
                    break;

                default: 
                    break;
            }
        }

        result[lhs] = resultRange;
    } else if (auto *PhiIns = dyn_cast<PHINode>(I)) {
        SignedRange resultRange = getRangeFromValue(PhiIns->getIncomingValue(0), inVal);

        for (size_t i = 1; i < PhiIns->getNumIncomingValues(); i++) {
            resultRange = SignedRange::meet(resultRange, getRangeFromValue(PhiIns->getIncomingValue(i), inVal));
        }

        result[lhs] = resultRange;
    } else if (ConstantInt *ConstInstraction = dyn_cast<ConstantInt>(lhs)) {
        result[lhs] = SignedRange(ConstInstraction->getSExtValue(), ConstInstraction->getSExtValue());
    } else if (auto *SignExtIns = dyn_cast<SExtInst>(I)) {
        result[lhs] = getRangeFromValue(SignExtIns->getOperand(0), inVal);
    } else if (auto *TruncIns = dyn_cast<TruncInst>(I)) {
        if (auto dstType = dyn_cast<IntegerType>(TruncIns->getDestTy())) {
            result[lhs] = SignedRange::singedWrapCast(
                getRangeFromValue(TruncIns->getOperand(0), inVal),
                dstType->getBitWidth());
        }
    }

    return result;
}

RangeAnalysis::LatticeValT RangeAnalysis::narrow(Instruction* I, LatticeValT outVal, LatticeValT oldOutVal) const { 
    if (!I || I->getType()->isVoidTy() || !I->getType()->isIntegerTy()) return outVal;
    
    DenseMap<Value*, SignedRange> result = outVal;
    Value* lhs = I;

    if (oldOutVal.contains(lhs) && outVal.contains(lhs)) {
        result[lhs] = SignedRange::narrow(outVal[lhs], oldOutVal[lhs], lhs->getType()->getIntegerBitWidth());
    }

    return result;   
}

RangeAnalysis::LatticeValT RangeAnalysis::widen(Instruction* I, LatticeValT outVal, LatticeValT oldOutVal, size_t visits) const {
    if (visits <= widdeningTreshold || !I || I->getType()->isVoidTy() || !I->getType()->isIntegerTy()) return outVal;

    DenseMap<Value*, SignedRange> result = outVal;
    Value* lhs = I;

    if (oldOutVal.contains(lhs) && outVal.contains(lhs)) {
        result[lhs] = SignedRange::widen(outVal[lhs], oldOutVal[lhs], lhs->getType()->getIntegerBitWidth());
    }

    return result;   
}

RangeAnalysis::LatticeValT RangeAnalysis::getNodePathSensitiveOutput(Instruction* node, Instruction* parent, LatticeValT parentOutput) {
    // Start with parent's output
    LatticeValT result = parentOutput;

    // We only care about conditional branches
    auto *br = dyn_cast<BranchInst>(parent);
    if (!br || !br->isConditional())
        return result;

    Value *cond = br->getCondition();

    auto *icmp = dyn_cast<ICmpInst>(cond);
    if (!icmp)
        return result;

    // Determine whether node is on TRUE or FALSE edge
    BasicBlock *trueBB  = br->getSuccessor(0);
    BasicBlock *falseBB = br->getSuccessor(1);

    BasicBlock *childBB = node->getParent();

    bool takingTrueEdge = (childBB == trueBB);
    bool takingFalseEdge = (childBB == falseBB);

    if (!takingTrueEdge && !takingFalseEdge)
        return result;

    Value *lhs = icmp->getOperand(0);
    Value *rhs = icmp->getOperand(1);

    // Only handle variable op constant for now
    auto *c = dyn_cast<ConstantInt>(rhs);
    if (!c)
        return result;

    int64_t constant = c->getSExtValue();

    // Current range
    SignedRange oldRange = result[lhs];

    SignedRange refined = oldRange;

    ICmpInst::Predicate pred = icmp->getPredicate();

    // Refine according to edge taken
    if (takingTrueEdge) {

        switch (pred) {
            case ICmpInst::ICMP_SGT:
                refined.lower = std::max(oldRange.lower, constant + 1);
                break;

            case ICmpInst::ICMP_SGE:
                refined.lower = std::max(oldRange.lower, constant);
                break;

            case ICmpInst::ICMP_SLT:
                refined.upper = std::min(oldRange.upper, constant - 1);
                break;

            case ICmpInst::ICMP_SLE:
                refined.upper = std::min(oldRange.upper, constant);
                break;

            case ICmpInst::ICMP_EQ:
                refined.lower = constant;
                refined.upper = constant;
                break;

            case ICmpInst::ICMP_NE:
                // harder to represent with intervals
                break;

            default:
                break;
        }

    } else if (takingFalseEdge) {

        switch (pred) {
            case ICmpInst::ICMP_SGT:
                refined.upper = std::min(oldRange.upper, constant);
                break;

            case ICmpInst::ICMP_SGE:
                refined.upper = std::min(oldRange.upper, constant - 1);
                break;

            case ICmpInst::ICMP_SLT:
                refined.lower = std::max(oldRange.lower, constant);
                break;

            case ICmpInst::ICMP_SLE:
                refined.lower = std::max(oldRange.lower, constant + 1);
                break;

            case ICmpInst::ICMP_EQ:
                // not equal
                break;

            case ICmpInst::ICMP_NE:
                refined.lower = constant;
                refined.upper = constant;
                break;

            default:
                break;
        }
    }

    result[lhs] = refined;

    return result;
}

bool RangeAnalysis::init(Function* F) {
    BasicBlock &entry = F->getEntryBlock();

    for (Argument &Arg : F->args()) {

        // Only handle integer arguments
        if (!Arg.getType()->isIntegerTy())
            continue;

        unsigned numBits = Arg.getType()->getIntegerBitWidth();
        boundaryLatticeVal[static_cast<Value*>(&Arg)] = SignedRange::full(numBits);
    }

    return true;
}
