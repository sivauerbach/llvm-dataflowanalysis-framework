#ifndef RANGE_ANALYSIS_HPP
#define RANGE_ANALYSIS_HPP

#include "Framework/InstructionAnalysis.hpp"
#include "AnalysisTypes/Helpers/SignedRange.hpp"

using namespace llvm;
using namespace framework;

class RangeAnalysis
    : public InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS> {
private:
    size_t widdeningTreshold;
    LatticeValT boundaryLatticeVal;

    static SignedRange getRangeFromValue(Value* value, LatticeValT& inVal);

protected:
    friend InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS>;

    LatticeValT top() const { return LatticeValT{ }; } 
    LatticeValT boundary() const { return boundaryLatticeVal; }
    LatticeValT meet(const LatticeValT& lhs, const LatticeValT& rhs) const;

    LatticeValT transfer(Instruction* I, LatticeValT inVal) const ;

    LatticeValT narrow(Instruction* I, LatticeValT outVal, LatticeValT oldOutVal) const;
    LatticeValT widen(Instruction* I, LatticeValT outVal, LatticeValT oldOutVal, size_t visits) const;

    LatticeValT getNodePathSensitiveOutput(Instruction* node, Instruction* parent, LatticeValT parentOutput);

public:
    explicit RangeAnalysis(size_t _widdeningTreshold = 2): 
        InstructionAnalysis<RangeAnalysis, DenseMap<Value*, SignedRange>, PASS_TYPE::FORWARDS>(),
        widdeningTreshold(_widdeningTreshold),
        boundaryLatticeVal() 
    { }

    bool init(Function* F);

    LatticeValT getInstructionRanges(Instruction* I) { return out[I]; }
};

#endif // !RANGE_ANALYSIS_HPP