// this pass is dead, use for testing purpose only
#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h" // callbase
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"

#include <stdlib.h>
#include <fstream>
#include <string>
#include "../handlerFile2.cpp"
#include<unistd.h>
#include<stdlib.h>
#include <string.h>
using namespace llvm;
using namespace std;

string outputFile;

namespace {
  struct Handler8 : public ModulePass {
  public:
    static char ID;
    Handler8() : ModulePass(ID) {};
    bool runOnModule(Module &M) {
      errs() << "\ngluten tag, 동료 >:)\n";
      LLVMContext &C = M.getContext();
      vector<Type*> params;
      FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
      M.getOrInsertFunction("handler8", FT);
      Function *handler8 = M.getFunction("handler8");
      for(auto f = M.begin(), fe = M.end(); f!=fe; ++f) {
        Function *F = &*f;
        if(F->getName().compare("main") == 0) 
        { 
          BasicBlock *bStart = &*(F->begin());
          IRBuilder<> builder(bStart, bStart->begin());
          vector<Value*> params;
          builder.CreateCall(handler8, params);
        }
      }
      return false;
    }
    ~Handler8() {
    }
  };
}

char Handler8::ID = 0;

INITIALIZE_PASS(Handler8, "Handler8", 
    "Handler8Pass: test pass", 
    false, false)

ModulePass *llvm::createHandler8Pass() {
  return new Handler8();
}
