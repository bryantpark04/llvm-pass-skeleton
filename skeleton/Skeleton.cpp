#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <unordered_set>

using namespace llvm;

namespace {

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        bool is_changed = false;
        for (Function &F : M) {
            for (BasicBlock &B : F) {
                std::unordered_set<Instruction*> to_erase;
                for (Instruction &I : B) {
                    // Pattern matching

                    CallInst *f_mul_add_call = dyn_cast<CallInst>(&I);
                    if (!f_mul_add_call 
                        || !f_mul_add_call->getCalledFunction() 
                        || f_mul_add_call->getCalledFunction()->getIntrinsicID() != Intrinsic::fmuladd
                    ) continue;

                    Value *arg_0 = f_mul_add_call->getArgOperand(0);
                    Value *arg_1 = f_mul_add_call->getArgOperand(1);
                    Value *arg_2 = f_mul_add_call->getArgOperand(2);

                    if (arg_0 != arg_1) continue;

                    ExtractElementInst *extract_0 = dyn_cast<ExtractElementInst>(arg_0);
                    if (!extract_0) continue;

                    Value *vector_op = extract_0->getVectorOperand();
                    ConstantInt *idx_0 = dyn_cast<ConstantInt>(extract_0->getIndexOperand());
                    if (!idx_0 || idx_0->getZExtValue() != 0) continue;

                    ExtractElementInst *extract_1 = dyn_cast<ExtractElementInst>(arg_2);
                    if (!extract_1) continue;

                    Value *squared_vec = extract_1->getVectorOperand();
                    ConstantInt *idx_1 = dyn_cast<ConstantInt>(extract_1->getIndexOperand());
                    if (!idx_1 || idx_1->getZExtValue() != 1) continue;

                    Instruction *f_mul = dyn_cast<Instruction>(squared_vec);
                    if (!f_mul || f_mul->getOpcode() != f_mul->FMul) continue; // TODO check if right

                    Value *vec_op_0 = f_mul->getOperand(0);
                    Value *vec_op_1 = f_mul->getOperand(1);

                    if (vec_op_0 != vec_op_1) continue;
                    if (vec_op_0 != vector_op) continue;

                    errs() << "I found the pattern!";
                    I.print(errs());
                    errs() << "\n";
                }
                for (Instruction* inst : to_erase) {
                    inst->eraseFromParent();
                }
            }
        }
        return is_changed ? PreservedAnalyses::all() : PreservedAnalyses::none();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}
