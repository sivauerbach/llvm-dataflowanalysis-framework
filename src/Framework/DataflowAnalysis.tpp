#include "DataflowAnalysis.hpp"

#include <type_traits>
#include <utility>

#include <llvm/Support/raw_ostream.h>

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
    std::vector<std::pair<NodeT, NodeT>> worklist;

    for (NodeT node: derived().getIter(args ...)) {
        if (isEdgeNode(node, args ...)) {
            worklist.push_back(std::make_pair(node, NodeT{ }));
        }
    }

    DenseMap<NodeT, size_t> visits;

    while (! worklist.empty()) {
        // pop the front node
        bool changed = false;
        std::pair<NodeT, NodeT> entry = worklist.front();
        NodeT node = entry.first;
        prevNode = entry.second;

        worklist.erase(worklist.begin());
        // visit node
        visits[node]++;
        
        // outs() << "Visited: "; node->print(outs()); outs() << " with prev: "; if (prevNode) prevNode->print(outs()); else outs() << "null"; outs() << "\n";

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
                
                changed = true;
            }
        }

        if (1 == visits[node] || changed) {
            for (NodeT succ : derived().getNodeNext(node, args ...)) { // REPORT
                worklist.push_back(std::make_pair(succ, node));
            }
        }
    }
}

}