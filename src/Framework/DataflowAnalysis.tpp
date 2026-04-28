#include "DataflowAnalysis.hpp"

#include <type_traits>

using namespace llvm;

namespace framework {

template <typename Node, typename T>
struct GetNode;

template <typename Derived, typename NodeT, typename LatticeVal, PASS_TYPE PassType, typename IteratorType>
template <typename ... Args>
void DataflowAnalysis<Derived, NodeT, LatticeVal, PassType, IteratorType>::initializeBlocks(Args ... args) {
    for (NodeT node : this->getIter(args ...)) {
        if (isEdgeNode(node, args ...)) {
            getNodeInput(node) = this->boundary();
            getNodeOutput(node) = this->transfer(node, getNodeInput(node));
        } else {
            getNodeInput(node) = this->top();
            getNodeOutput(node) = this->top();
        }
    }
}

template <typename Derived, typename NodeT, typename LatticeVal, PASS_TYPE PassType, typename IteratorType>
template <typename ... Args>
void DataflowAnalysis<Derived, NodeT, LatticeVal, PassType, IteratorType>::runImpl(Args ... args) {
    initializeBlocks(args ...);

    bool changed = true;
    while (changed) {
        changed = false;

        for (NodeT node : this->getIter(args ...)) {
            if (isEdgeNode(node, args ...)) continue;

            // meet over all predecessors
            LatticeVal newInput = this->top();
            for (NodeT pNode : getNodePrev(node, args ...))
                newInput = this->meet(newInput, getNodeOutput(pNode));

            LatticeVal newOutput = this->transfer(node, newInput);

            if (newInput != getNodeInput(node) || newOutput != getNodeOutput(node)) {
                getNodeInput(node)  = newInput;
                getNodeOutput(node) = newOutput;
                changed = true;
            }
        }
    }
}

}