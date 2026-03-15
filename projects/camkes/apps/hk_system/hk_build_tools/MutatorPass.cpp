#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {
// Наш агрессивный мутатор
struct AggressiveMutatorPass : public PassInfoMixin<AggressiveMutatorPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
        bool changed = false;

        // Ищем функции, помеченные нашим SDK-макросом HK_MUTATE
        if (!F.hasFnAttribute("hk_mutate")) {
            return PreservedAnalyses::all(); // Пропускаем критический код
        }

        for (auto &BB : F) {
            for (auto it = BB.begin(); it != BB.end(); ) {
                Instruction &I = *it++;
                
                // Агрессивная замена: a = b + c  --->  a = b - (~c) - 1
                if (I.getOpcode() == Instruction::Add) {
                    IRBuilder<> Builder(&I);
                    Value *op1 = I.getOperand(0);
                    Value *op2 = I.getOperand(1);

                    // 1. Создаем побитовое НЕ (NOT) для второго операнда
                    Value *notOp2 = Builder.CreateNot(op2, "obf.not");
                    // 2. Вычитаем инвертированный операнд из первого: b - (~c)
                    Value *sub1 = Builder.CreateSub(op1, notOp2, "obf.sub1");
                    // 3. Вычитаем единицу
                    Value *one = ConstantInt::get(I.getType(), 1);
                    Value *finalSub = Builder.CreateSub(sub1, one, "obf.final");

                    // Заменяем все использования старого сложения на наш монструозный код
                    I.replaceAllUsesWith(finalSub);
                    I.eraseFromParent(); // Удаляем старую инструкцию
                    changed = true;
                }
            }
        }
        return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }
};
} // namespace