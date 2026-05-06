#include "DominatorAnalysis.hpp"

DominatorAnalysis::LatticeValT DominatorAnalysis::transfer(BasicBlock* B, DominatorAnalysis::LatticeValT inVal) const { 
    inVal.insert(B);
    return inVal;
}

BasicBlock* DominatorAnalysis::immediateDominator(Function* function, BasicBlock* B) {
    if (&function->getEntryBlock() == B) return nullptr;

    // Dom(B) \ {B} — strict dominators
    std::vector<BasicBlock*> strictDoms;

    for (BasicBlock* D : out[B])
        if (D != B)
            strictDoms.push_back(D);

    // idom is the unique dominator d ≠ B such that
    // d is dominated by every other strict dominator of B.
    // Equivalently: idom has the largest Dom set among strict doms.
    BasicBlock* idom = nullptr;
    size_t maxSize = 0;
    for (BasicBlock* D : strictDoms) {
        size_t sz = out[D].size();
        if (sz > maxSize) {
            maxSize = sz;
            idom = D;
        }
    }
    return idom;
}