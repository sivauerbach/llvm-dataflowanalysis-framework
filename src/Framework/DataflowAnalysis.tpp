#include "DataflowAnalysis.hpp"

#include <type_traits>
#include <utility>

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

    // TODO: for (PHASE phase : derived().getPhases()) {
    //     derived().runPhase(phase, args ...);
    // }
    derived().runPhase(PHASE::WIDENING, args ...);
    derived().runPhase(PHASE::NARROWING, args ...);
}

template <typename Derived, typename NodeT, typename LatticeValT, PASS_TYPE PassType, typename IteratorType>
template <typename ... Args>
void DataflowAnalysis<Derived, NodeT, LatticeValT, PassType, IteratorType>::runPhase(PHASE phase, Args ... args) {
    std::vector<NodeT> worklist;

    for (NodeT node: derived().getIter(args ...)) {
        if (isEdgeNode(node, args ...)) {
            worklist.push_back(node);
        }
    }

    DenseMap<NodeT, size_t> visits;

    while (! worklist.empty()) {
        bool thisNodeChanged = false;
        
        // pop the front node
        NodeT node = worklist.front();
        worklist.erase(worklist.begin());
        
        // visit node
        visits[node]++;
        
        if (! isEdgeNode(node, args ...)) {
            // Meet over all predecessors
            LatticeValT newInput = derived().top(); // derived() returns instance casted to correct derived class
            for (NodeT pNode : derived().getNodePrev(node, args ...))
                newInput = derived().meet(newInput, derived().getNodePathSensitiveOutput(node, pNode, getNodeOutput(pNode)));

            // Transfer fucntion
            LatticeValT candidate = derived().transfer(node, newInput), newOutput;

            // Widening/Narrowing
            switch (phase) {
                case PHASE::WIDENING:
                    newOutput = derived().widen(node, candidate, derived().getNodeOutput(node), visits[node]);
                    break;
            
                case PHASE::NARROWING:
                    newOutput = derived().narrow(node, candidate, derived().getNodeOutput(node));
                    break;
            }
    
            if (newInput != derived().getNodeInput(node) || newOutput != derived().getNodeOutput(node)) {
                derived().getNodeInput(node)  = newInput;
                derived().getNodeOutput(node) = newOutput;
                
                thisNodeChanged = true;
            }
        }
        
        // visit successors if node changed or this is its first visit
        if (1 == visits[node] || thisNodeChanged) {
            for (NodeT succ : derived().getNodeNext(node, args ...)) { // REPORT
                worklist.push_back(succ);
            }
        }
    }
}

}