#ifndef RANGE_ANALYSIS_HPP
#define RANGE_ANALYSIS_HPP

#include "Framework/InstructionAnalysis.hpp"

#include <limits>
#include <type_traits>

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
private:
    int64_t lower;
    int64_t upper;
    bool empty;

public:
    // constructor
    explicit SignedRange (bool _empty): lower(0), upper(0), empty(_empty) {}
    SignedRange (int64_t _lower, int64_t _upper): lower(_lower), upper(_upper), empty(false) {}
    SignedRange (int64_t _lower, int64_t _upper, bool _empty): lower(_lower), upper(_upper), empty(_empty) {}

    static fullInBits(size_t sizeInBits) {
        if (sizeInBits >= 64) 
            return SignedRange(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());

        return SignedRange(-(2 << (sizeInBits - 1)), (2 << (sizeInBits - 1)) - );
    }

    static SignedRange meet(SignedRange r1, SignedRange r2) {
        if (r1.isempty()) return r2;
        if (r2.isempty()) return r1;

        int64_t newLower = std::min(r1.lower, r2.lower);
        int64_t newUpper = std::max(r1.upper, r2.upper);
        return SignedRange(newLower, newUpper);
    }

    static SignedRange addRanges(SignedRange r1, SignedRange r2) {
        if (r1.isempty() || r2.isempty()) return SignedRange(true);

        int64_t newLower = std::min(r1.lower, r2.lower);
        int64_t newUpper = std::max(r1.upper, r2.upper);
        return SignedRange(newLower, newUpper);
    }
    
    SignedRange singedWrapCast(SignedRange range, size_t sizeInBits) requires std::is_same_v<T, uint64_t> {
        if (range.empty() || sizeInBits == 0) return SignedRange(true);
        if (sizeInBits >= 64) return range; // no change for 64 bit or larger types

        if (range.lower >> sizeInBits == range.upper >> sizeInBits) 
            return SignedRange(range.lower & ((2 << sizeInBits) - 1) , range.upper & ((2 << sizeInBits) - 1));
        else
            return fullInBits(sizeInBits);

    }
    

    bool isempty() {
        return empty; 
    }
}

template<typename T>
class RangeAnalysis
    : public InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS>,
protected:
    friend InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS>;

    LatticeValT top() const { return LatticeValT{ }; } 
    LatticeValT boundary() const { return LatticeValT{ }; }
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
                resultRange = SignedRange::fullInBits(lhs->getType()->getIntegerBitWidth());

            switch (binOp->getOpcode()) {
                case Instruction::Add: 
                    
                    break;
                case Instruction::Sub: 
                    
                    break;
                case Instruction::Mul: 
                    
                    break;

                case Instruction::SDiv:
                    break;

                default: 
                    break;
            }

            result[lhs] = resultRange;
        }

        return result;
    }

public:
    RangeAnalysis(): 
        InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS>() { }

};

#endif // !RANGE_ANALYSIS_HPP