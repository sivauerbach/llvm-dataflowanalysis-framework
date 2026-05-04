#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>

#include "Passes/DominatorsPass.hpp"
#include "Passes/FaintPass.hpp"
#include "Passes/RangePass.hpp"

using namespace llvm;

// ============================================================
//  Plugin registration
// ============================================================
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "UnifiedPass",
        LLVM_VERSION_STRING,
        [](PassBuilder& PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {                  
                    if (Name == "faint") {
                        FPM.addPass(FaintPass{});
                        return true;
                    }
                    return false;
                });
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "dominators") {
                        FPM.addPass(DominatorsPass{});
                        return true;
                    }                    
                    return false;
                });
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "range") {
                        FPM.addPass(RangePass{});
                        return true;
                    }                    
                    return false;
                });
        }
    };
}