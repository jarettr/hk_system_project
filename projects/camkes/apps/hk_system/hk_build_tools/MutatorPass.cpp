/**
 * @file MutatorPass.cpp
 * @brief LLVM transformation pass for arithmetic obfuscation and mutation.
 * * Implements a transformation of (a + b) into (a - (~b) - 1) for functions
 * marked with the 'hk_mutate' attribute.
 */

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {

struct BinaryObfuscationPass : public PassInfoMixin<BinaryObfuscationPass> {
    /**
     * @brief Core transformation logic.
     * Searches for integer addition and replaces it with an obfuscated sequence.
     */
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
        bool changed = false;

        // Target functions marked with the HK_MUTATE SDK macro
        if (!F.hasFnAttribute("hk_mutate")) {
            return PreservedAnalyses::all();
        }

        for (auto &BB : F) {
            for (auto it = BB.begin(); it != BB.end(); ) {
                Instruction &I = *it++;
                
                // Transformation: a = b + c  --->  a = b - (~c) - 1
                if (I.getOpcode() == Instruction::Add && I.getType()->isIntegerTy()) {
                    IRBuilder<> Builder(&I);
                    Value *op1 = I.getOperand(0);
                    Value *op2 = I.getOperand(1);

                    // 1. Bitwise NOT: (~c)
                    Value *notOp2 = Builder.CreateNot(op2, "obf.not");
                    // 2. Subtraction: b - (~c)
                    Value *sub1 = Builder.CreateSub(op1, notOp2, "obf.sub1");
                    // 3. Final alignment: (b - (~c)) - 1
                    Value *one = ConstantInt::get(I.getType(), 1);
                    Value *finalSub = Builder.CreateSub(sub1, one, "obf.final");

                    // Replace original instruction and erase it
                    I.replaceAllUsesWith(finalSub);
                    I.eraseFromParent();
                    changed = true;
                }
            }
        }

        // Inform LLVM about CFG preservation
        return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }
};

} // namespace

/* --- LLVM Plugin Registration --- */
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "HeuristicMutator", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "hk-mutate") {
                        FPM.addPass(BinaryObfuscationPass());
                        return true;
                    }
                    return false;
                });
        }};
}
