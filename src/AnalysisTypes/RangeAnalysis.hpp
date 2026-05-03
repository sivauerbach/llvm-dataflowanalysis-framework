#ifndef RANGE_ANALYSIS_HPP
#define RANGE_ANALYSIS_HPP

#include "Framework/InstructionAnalysis.hpp"
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include "llvm/IR/Constants.h"

#include <limits>
#include <type_traits>
#include <string>

using namespace llvm;
using namespace framework;

template<typename T>
bool willAddOverflow(T a, T b) requires std::is_integral_v<T> && std::is_signed<T>::value {
    if ((b > 0) && (a > std::numeric_limits<T>::max() - b))
        return true;
    if ((b < 0) && (a < std::numeric_limits<T>::min() - b))
        return true;
    return false;
}

template<typename T>
bool willSubOverflow(T a, T b) requires std::is_integral_v<T> && std::is_signed<T>::value {
    if ((b < 0) && (a > std::numeric_limits<T>::max() + b))
        return true;
    if ((b > 0) && (a < std::numeric_limits<T>::min() + b))
        return true;
    
    return false;
}

template<typename T>
bool willMulOverflow(T a, T b) requires std::is_integral_v<T> && std::is_signed<T>::value {
    if (a == 0 || b == 0)
        return false;

    if (a == -1 && b == std::numeric_limits<T>::min())
        return true;
    if (b == -1 && a == std::numeric_limits<T>::min())
        return true;

    if (a > 0) {
        if (b > 0)
            return a > std::numeric_limits<T>::max() / b;
        else
            return b < std::numeric_limits<T>::min() / a;
    } else {
        if (b > 0)
            return a < std::numeric_limits<T>::min() / b;
        else
            return a != 0 && b < std::numeric_limits<T>::max() / a;
    }

    return false;
}

template<typename T>
bool willDivOverflow(T a, T b) requires std::is_integral_v<T> && std::is_signed<T>::value {
    if (b == 0)
        return true;

    if (a == std::numeric_limits<T>::min() && b == -1)
        return true;

    return false;
}


class SignedRange {
public:
    int64_t lower;
    int64_t upper;
    bool empty;

    static int64_t min(size_t sizeInBits) {
        if (sizeInBits >= 64) 
            return std::numeric_limits<int64_t>::min();

        return -(1ULL << (sizeInBits - 1));
    }

    static int64_t max(size_t sizeInBits) {
        if (sizeInBits >= 64) 
            return std::numeric_limits<int64_t>::max();

        return (1ULL << (sizeInBits - 1)) - 1;
    }

public:
    // constructor
    SignedRange(): SignedRange(true) { }
    explicit SignedRange (bool _empty): lower(0), upper(0), empty(_empty) {}
    SignedRange (int64_t _lower, int64_t _upper): lower(_lower), upper(_upper), empty(false) {}
    SignedRange (int64_t _lower, int64_t _upper, bool _empty): lower(_lower), upper(_upper), empty(_empty) {}

    SignedRange(const SignedRange& other): lower(other.lower), upper(other.upper), empty(other.empty) {}
    SignedRange(SignedRange&& other) noexcept: lower(std::move(other.lower)), upper(std::move(other.upper)), empty(std::move(other.empty)) {}

    SignedRange& operator=(const SignedRange& other) {
        if (this != &other) {
            lower = other.lower;
            upper = other.upper;
            empty = other.empty;
        }
        return *this;
    }

    SignedRange& operator=(SignedRange&& other) noexcept {
        if (this != &other) {
            lower = std::move(other.lower);
            upper = std::move(other.upper);
            empty = std::move(other.empty);
        }
        return *this;
    }

    bool operator==(const SignedRange& other) const {
        return empty == other.empty && lower == other.lower && upper == other.upper;
    }

    bool operator!=(const SignedRange& other) const {
        return !(*this == other);
    }
    
    static SignedRange full(size_t sizeInBits = 64) {
        return SignedRange(min(sizeInBits), max(sizeInBits));
    }

    static SignedRange meet(SignedRange r1, SignedRange r2) {
        if (r1.isEmpty()) return r2;
        if (r2.isEmpty()) return r1;

        int64_t newLower = std::min(r1.lower, r2.lower);
        int64_t newUpper = std::max(r1.upper, r2.upper);
        return SignedRange(newLower, newUpper);
    }

    static SignedRange addRanges(SignedRange r1, SignedRange r2, bool noSignedWrap, size_t sizeInBits) {
        int64_t newLower =0, newUpper = 0;

        if (noSignedWrap) {
            if (!willAddOverflow(r1.lower, r2.lower)) {
                if (r1.lower + r2.lower > max(sizeInBits)) {
                    return SignedRange(true);
                } else {
                    newLower = std::max(r1.lower + r2.lower, min(sizeInBits));
                }
            } else {
                if (r1.lower < 0) {
                    newLower = min(sizeInBits);
                } else {
                    return SignedRange(true);
                }
            }           
            
            if (!willAddOverflow(r1.upper, r2.upper)) {
                if (r1.upper + r2.upper < min(sizeInBits)) {
                    return SignedRange(true);
                } else {
                    newUpper = std::min(r1.upper + r2.upper, max(sizeInBits));
                }
            } else {
                if (r1.upper > 0) {
                    newUpper = max(sizeInBits);
                } else {
                    return SignedRange(true);
                }
            }

            return SignedRange(newLower, newUpper);  
        } else {
            // For starters, we assume only NSW, NUW operations will be used, so we can just return the full range for the result.
        
            return SignedRange(min(sizeInBits), max(sizeInBits));
        }
    }
    
    // Maybe need for no nsw case.
    SignedRange singedWrapCast(SignedRange range, size_t sizeInBits) {
        if (range.isEmpty() || sizeInBits == 0) return SignedRange(true);
        if (sizeInBits >= 64) return range; // no change for 64 bit or larger types

        if (range.lower >> sizeInBits == range.upper >> sizeInBits) 
            return SignedRange(range.lower & ((1ULL << sizeInBits) - 1) , range.upper & ((1ULL << sizeInBits) - 1));
        else
            return full(sizeInBits);
    }
    
    bool isEmpty() {
        return empty; 
    }

    std::string getRangeString() const {
        if (empty) return "empty";
        return "[" + std::to_string(lower) + ", " + std::to_string(upper) + "]";
    }
};

class RangeAnalysis
    : public InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS> {
private:
    LatticeValT boundaryLatticeVal;

protected:
    friend InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS>;

    LatticeValT top() const { return LatticeValT{ }; } 
    LatticeValT boundary() const { return boundaryLatticeVal; }
    LatticeValT meet(const LatticeValT& lhs, const LatticeValT& rhs) const {     
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

    LatticeValT transfer(Instruction* I, LatticeValT inVal) const {
        // non-assignment instruction does not modify the range of any variable
        if (!I || I->getType()->isVoidTy() || !I->getType()->isIntegerTy()) return inVal;

        Value* lhs = I;
        LatticeValT result = inVal;
        BinaryOperator* binOp = dyn_cast<BinaryOperator>(I);
        
        if (binOp) {
            // Both operands of the binary operator should be in the inVal map as they must be defined before using them.
            SignedRange rhs1 = inVal[binOp->getOperand(0)], rhs2 = inVal[binOp->getOperand(1)], 
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
                    // TODO: add check for edge cases for subtraction overflow, e.g. min - 1, max - (-1)
                        resultRange = SignedRange::addRanges(rhs1, SignedRange(-rhs2.upper, -rhs2.lower), binOp->hasNoSignedWrap(), lhs->getType()->getIntegerBitWidth());
                        break;
                    case Instruction::Mul: 
                        
                        break;

                    case Instruction::SDiv:
                        break;

                    case Instruction::UDiv:
                        break;

                    default: 
                        break;
                }
            }

            result[lhs] = resultRange;
        }

        return result;
    }

    LatticeValT getNodePathSensitiveOutput(Instruction* node, Instruction* parent, LatticeValT parentOutput, Function*) {
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
public:
    bool init(Function* F) {

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

    LatticeValT getInstructionRanges(Instruction* I) {
        return out[I];
    }

public:
    RangeAnalysis(): 
        InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS>(),
        boundaryLatticeVal() 
    { }

};

#endif // !RANGE_ANALYSIS_HPP