#include "DataflowAnalysis.hpp"

#include <type_traits>

using namespace llvm;

namespace framework {

template <typename Node, typename T>
struct GetNode;

template <typename Derived, typename NodeT, typename LatticeValT, PASS_TYPE PassType, typename IteratorType>
template <typename ... Args>
void DataflowAnalysis<Derived, NodeT, LatticeValT, PassType, IteratorType>::initializeBlocks(Args ... args) {
    for (NodeT node : derived().getIter(args ...)) {
        if (isEdgeNode(node, args ...)) {
            derived().getNodeInput(node) = derived().boundary();
            derived().getNodeOutput(node) = derived().transfer(node, getNodeInput(node));
        } else {
            derived().getNodeInput(node) = derived().top();
            derived().getNodeOutput(node) = derived().top();
        }
    }
}

template <typename Derived, typename NodeT, typename LatticeValT, PASS_TYPE PassType, typename IteratorType>
template <typename ... Args>
void DataflowAnalysis<Derived, NodeT, LatticeValT, PassType, IteratorType>::runImpl(Args ... args) {
    derived().initializeBlocks(args ...);
    derived().runPhase(PHASE::WIDENING, args ...);
    derived().runPhase(PHASE::NARROWING, args ...);
}

template <typename Derived, typename NodeT, typename LatticeValT, PASS_TYPE PassType, typename IteratorType>
template <typename ... Args>
void DataflowAnalysis<Derived, NodeT, LatticeValT, PassType, IteratorType>::runPhase(PHASE phase, Args ... args) {
    auto& iter = derived().getIter(args ...);
    std::vector<NodeT> worklist(iter.begin(), iter.end());
    DenseMap<NodeT, size_t> visits;

    while (! worklist.empty()) {
        visits[NodeT node = worklist.pop()]++;
        if (isEdgeNode(node, args ...)) continue;

        // Meet over all predecessors
        LatticeValT newInput = derived().top();
        for (NodeT pNode : derived().getNodePrev(node, args ...))
            newInput = derived().meet(newInput, derived().getNodePathSensitiveOutput(node, pNode, getNodeOutput(pNode), args...));

        // Transfer fucntion
        LatticeValT candidate = derived().transfer(node, newInput), newOut;

        // Widening/Nerrowing
        switch (phase) {
            case PHASE::WIDENING:
                if (derived().getNodeOutput(node) == derived().top()) {
                    newOut = candidate;
                } else {
                    derived().widen(candidate, derived().getNodeOutput(node), visits[node]);
                }
                break;
        
            case PHASE::NARROWING:
                derived().narrow(candidate, derived().getNodeOutput(node));
        }

        if (newInput != derived().getNodeInput(node) || newOutput != derived().getNodeOutput(node)) {
            derived().getNodeInput(node)  = newInput;
            derived().getNodeOutput(node) = newOutput;
            
            for (NodeT succ : derived().getNodeNext(node)) {
                worklist.push(Succ);
            }
        }
    }
}

}