/*
 * This pass isn't meant to be ran alone.
 */

#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/InitializePasses.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h" // callbase
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace llvm;

Function *printFunc;
Function *exitFunc;
Function *int2kCheck;
Function *int2kTranslateName; // Function name to ID
Function *int2kHandleArg; // handling exe arguments
Function *int2kInitFlag; // init flags and stuffs
Function *int2kDeleteFlag; // delete flags.
Function *int2kAddEdge; // help metadata
Function *markFunctionID; // this one calls all the NameToID and TranslateAddr
Function *metaData; // this one will call the AddEdge
std::vector<Function*> func;
int nFunction; // number of functions;
std::string output;
Value *flag; // GlobalVariable type
Type *flagType;
std::map<std::string, bool> focusedFunction;
//std::map<Function*, Value*> funcNameStr;

Value *getAndInsertFuncPtr(IRBuilder<> &builder, LLVMContext &C, Function* F);
Value *getFuncNameStr(IRBuilder<> &builder, Function* F);
Function *makeMarkFunctionID(Module &M);
Function *makeMetaData(Module &M);
Function *getPrint(Module &M);
Function *getQuit(Module &M);
Function *getInt2kCheck(Module &M); // exiter
Function *getInt2kInitFlag(Module &M);
Function *getInt2kDeleteFlag(Module &M);
Function *getInt2kAddEdge(Module &M);
Function *getInt2kHandleArg(Module &M);
Function *getInt2kTranslateName(Module &M);

std::map<std::string, bool> needInstrument;

struct Int2kVisitor : public InstVisitor<Int2kVisitor>
{
  void visitFunction(Function &F) 
  {
    if(F.begin()==F.end() || !needInstrument[F.getName().str()])
      return;
      
    LLVMContext &C = F.getParent()->getContext();
    BasicBlock *b = &*F.begin();
    IRBuilder<> builder(b, b->begin());
    Value *funcNameStr = getFuncNameStr(builder, &F);
    SmallVector<Value*, 1> params;
    params.push_back(funcNameStr);
    builder.CreateCall(int2kCheck, params);
  }
};

void getThings(Module &M)
{
 		printFunc = getPrint(M);
    exitFunc = getQuit(M);
    int2kCheck = getInt2kCheck(M);
    int2kTranslateName = getInt2kTranslateName(M);
    int2kInitFlag = getInt2kInitFlag(M);
    int2kDeleteFlag = getInt2kDeleteFlag(M);
    int2kAddEdge = getInt2kAddEdge(M);
    int2kHandleArg = getInt2kHandleArg(M);
    makeMetaData(M);
    makeMarkFunctionID(M); 
}
void insertThings(Module &M)
{
    nFunction = 0;
    errs() << "Function list: \n";
    for(auto f = M.begin(), fe = M.end(); f!=fe; ++f)
    {
      nFunction++;
      func.push_back(&*f);
      errs () << f->getName() << " ";
    }
    errs() << "\n";
    Int2kVisitor int2kVisitor;
    int2kVisitor.visit(M);
    for(auto f = M.begin(), fe = M.end(); f!=fe; ++f)
    {
      Function *F = &*f;
      if(F->getName().compare("main") == 0)
      {
        LLVMContext &C = M.getContext(); 
        std::vector<Value*> arg;
        for(auto i = F->arg_begin(), iE = F->arg_end(); i != iE; ++i)
            arg.push_back(i);
        BasicBlock *bStart = &*(F->begin());
        BasicBlock *bEnd = &(F->back());
        IRBuilder<> builder(bStart, bStart->begin());
        // insert init flags, metadata
        SmallVector<Value*, 1> params;
        params.push_back(ConstantInt::get(Type::getInt32Ty(C), nFunction));
        builder.CreateCall(int2kInitFlag, params); 
        // insert metaData function
        SmallVector<Value*, 0> params1;
        builder.CreateCall(metaData, params1);
        // insert init translation from funcPtr||funcName to ID
        builder.CreateCall(markFunctionID, params1);
        // insert argument handler
        SmallVector<Value*, 2> params2;
        params2.push_back(arg[0]);
        params2.push_back(arg[1]);
        builder.CreateCall(int2kHandleArg, params2);
        BasicBlock::iterator it = bEnd->end();
        it--;
        IRBuilder<> builderEnd(bEnd, it);
        // insert function that delete allocated arrays to free memeory
        params.clear();
        builderEnd.CreateCall(int2kDeleteFlag, params);
        break;
      }
    }
}

namespace {
  struct Handler8Helper : public ModulePass {
  public:
    static char ID;
    Handler8Helper() : ModulePass(ID) {};
    bool runOnModule(Module &M) {
     	errs() << "\ngluten tag, 동료 >:)\n";
      M.dumpToFile(); // int2kBC.bc;
      M.runSVF();
			M.readSVF();
			M.injectSVFInfo(); 
      for(int i = 0; i < (int)M.needInstrumentedFunc.size(); ++i) {
        errs () << M.needInstrumentedFunc[i] << "\n";
        needInstrument[M.needInstrumentedFunc[i]] = true;
      }
      getThings(M);
      insertThings(M);
      system("rm callgraph_final.dot callgraph_initial.dot");
      system("rm int2kGraph.txt int2kInfo.txt int2kBC.txt int2kBC.bc");
      errs() << "\n";
      M.dumpToFile();
      return false;
    }
  };
}

char Handler8Helper::ID = 0;

INITIALIZE_PASS(Handler8Helper, "Handler8Helper", 
    "Handler8HelperPass: helps the Handler8pass", 
    false, false)

ModulePass *llvm::createHandler8HelperPass() {
  return new Handler8Helper();
}

/*
* insert call to this function at begining of main
* this function translates ALL functions pointer at run-time into unique ID
*/
Function* makeMarkFunctionID(Module &M)
{
  LLVMContext &C = M.getContext();
  SmallVector<Type*, 0> params;
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("markFunctionID", FT);
  markFunctionID = M.getFunction("markFunctionID");
  BasicBlock* block = BasicBlock::Create(C, "entry", markFunctionID);
  IRBuilder<> builder(block);
  int funcID = 0;
  
  for(auto f = M.begin(), fe = M.end(); f!=fe; ++f)
  {
    Function *F = &*f;
    //errs() << F->getName() << "\n";
    //Value *castFunc = getAndInsertFuncPtr(builder, C, F);
    SmallVector<Value*, 2> args;
    //args.push_back(castFunc);
    //args.push_back(ConstantInt::get(Type::getInt32Ty(C), funcID));
    //builder.CreateCall(int2kTranslateAddr, args);
    args.clear();
    // print info at run-time
    Value *funcName = getFuncNameStr(builder, F);
    args.push_back(funcName);
    args.push_back(ConstantInt::get(Type::getInt32Ty(C), funcID));
    builder.CreateCall(int2kTranslateName, args);
    funcID ++;
  }
  builder.CreateRetVoid();
  return M.getFunction("markFunctionID");
}

/* 
 * insert call to this function after the MarkFunctionID to 
 * make meta data of unrelated functions 
 * using _int2k_add_edge
 */
Function *makeMetaData(Module &M)
{
  LLVMContext &C = M.getContext();
  SmallVector<Type*, 0> params;
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("metaData", FT);
  metaData = M.getFunction("metaData");
  BasicBlock *block = BasicBlock::Create(C, "entry", metaData);
  IRBuilder<> builder(block);
  for(auto f1 = M.begin(), f1e = M.end(); f1!=f1e; ++f1)
    for(int i = 0; i < (int)f1->unrelatedFunc.size(); ++i)
    { 
      Function *F1 = &*f1;
      Function *F2 = f1->unrelatedFunc[i];
      Value *F1Name = getFuncNameStr(builder, F1);
      Value *F2Name = getFuncNameStr(builder, F2);      
      SmallVector<Value*, 2> args;
      args.push_back(F1Name);
      args.push_back(F2Name);
      builder.CreateCall(int2kAddEdge, args);
    }
  builder.CreateRetVoid();
  return M.getFunction("metaData");
}

// insert IR instructions translates Function ptr to int32ptr at runtime.
Value* getAndInsertFuncPtr(IRBuilder<> &builder, LLVMContext &C, Function* F)
{
  Value *ptr = builder.CreateAlloca(Type::getInt32PtrTy(C), nullptr);
  Value *cast = builder.CreateBitCast(F, Type::getInt32PtrTy(C));
  builder.CreateStore(cast, ptr);
  return cast;
}
Value *getFuncNameStr(IRBuilder<> &builder, Function* F)
{
  //if(funcNameStr.find(F) == funcNameStr.end())
  //{
    Value *tmp = builder.CreateGlobalStringPtr(F->getName());
    //funcNameStr[F] = tmp;
  //}
  return tmp;
}

Function *getPrint(Module &M)
{
  Function *tmp = M.getFunction("printf"); // external
  if(tmp == NULL)
  {
    LLVMContext &C = M.getContext();
    std::vector<Type*> params;
    params.push_back(Type::getInt8PtrTy(C));
    FunctionType *printType = 
      FunctionType::get(Type::getInt32Ty(C), params, true);
    Function *x = Function::Create(printType, 
      Function::ExternalLinkage, "printf", &M);
    tmp = x;
  }
  return tmp;
}
Function *getQuit(Module &M)
{
  Function *tmp = M.getFunction("exit"); // external
  if(tmp == NULL)
  {
    LLVMContext &C = M.getContext();
    std::vector<Type*> params;
    params.push_back(Type::getInt32Ty(C));
    FunctionType *exitType = 
      FunctionType::get(Type::getVoidTy(C), params, false);
    Function *x = Function::Create(exitType,
      Function::ExternalLinkage, "exit", &M);
    tmp = x;
  }
  return tmp;
}
Function *getInt2kCheck(Module &M) // exiter
{
  LLVMContext &C = M.getContext();
  std::vector<Type *> params;
  params.push_back(Type::getInt8PtrTy(C));// run-time func ptr
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("_int2k_check", FT);
  return M.getFunction("_int2k_check");
}
Function *getInt2kInitFlag(Module &M) 
{
  LLVMContext &C = M.getContext();
  SmallVector<Type *, 1> params;
  params.push_back(Type::getInt32Ty(C));
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("_int2k_init_flag", FT);
  return M.getFunction("_int2k_init_flag");
}
Function *getInt2kDeleteFlag(Module &M)
{
  LLVMContext &C = M.getContext();
  SmallVector<Type *, 0> params;
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("_int2k_delete_flag", FT);
  return M.getFunction("_int2k_delete_flag");
}
Function *getInt2kAddEdge(Module &M)
{
  LLVMContext &C = M.getContext();
  SmallVector<Type*, 2> params;
  for(int i = 0; i < 2; ++i)
    params.push_back(Type::getInt8PtrTy(C));
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("_int2k_add_edge", FT);
  return M.getFunction("_int2k_add_edge");
}
Function *getInt2kHandleArg(Module &M)
{
  LLVMContext &C = M.getContext();
  SmallVector<Type*, 2> params;
  params.push_back(Type::getInt32Ty(C)); // i32
  params.push_back(PointerType::getUnqual(Type::getInt8PtrTy(C))); //i8**
  FunctionType *FT = FunctionType::get(Type::getInt32Ty(C), params, false);
  M.getOrInsertFunction("_int2k_handle_arg", FT);
  return M.getFunction("_int2k_handle_arg");
}
Function *getInt2kTranslateName(Module &M)
{
  LLVMContext &C = M.getContext();
  SmallVector<Type*, 2> params;
  params.push_back(Type::getInt8PtrTy(C));
  params.push_back(Type::getInt32Ty(C)); 
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("_int2k_translateName", FT);
  return M.getFunction("_int2k_translateName");
}
