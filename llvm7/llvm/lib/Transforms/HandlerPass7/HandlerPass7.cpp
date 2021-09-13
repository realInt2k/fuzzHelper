#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h" // callbase
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "../handlerFile4.cpp"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "llvm/IR/BasicBlock.h"
#define MAXLEN 1000
#include <iostream>
#include <fstream>

using namespace llvm;

Function *print;
Function *quit;
Function *int2kCheck;
Function *int2kTranslateAddr; // Function name to Addr
Function *int2kTranslateName; // Function name to ID
Function *int2kHandleArg; // handling exe arguments
Function *int2kInitFlag; // init flags and stuffs .. called the very first
Function *int2kDeleteFlag; // delete flags.
Function *int2kAddEdge; // help metadata
Function *int2kInsertMapAddr;
Function *int2kSortMapAddr;
Function *int2kInsertMapName;
Function *int2kSortMapName;
Function *insertAllFunctionAddrAndName; // insert all function addr to mapAddr, mapName and call sort
Function *markFunctionID; // this one calls all the NameToID and TranslateAddr
Function *metaData; // this one will call the AddEdge
std::vector<Function*> func;
int nFunction; // number of functions;
std::string output;
Value *flag; // GlobalVariable type
Type *flagType;

Value* getAndInsertFuncPtr(IRBuilder<> &builder, LLVMContext &C, Function *F);

struct Int2kVisitor : public InstVisitor<Int2kVisitor>
{
  void visitCallInst (CallInst &I)
  {
    LLVMContext &C = I.getContext();
    Function *callF = I.getCalledFunction();
    if(callF->getName().compare("printf") == 0)
    {
      Instruction *Inst = &I;
      errs() << "bingo ";
      IRBuilder<> builder(Inst, nullptr);
      SmallVector<Value*, 1> params;
      Value *castFunc = getAndInsertFuncPtr(builder, C, print); 
      params.push_back(castFunc);
      builder.CreateCall(int2kCheck, params);
    }
  }
  void visitFunction(Function &F) 
  {
    if(F.begin()==F.end())
      return;
    LLVMContext &C = F.getParent()->getContext();
    BasicBlock *b = &*F.begin();
    IRBuilder<> builder(b, b->begin());
    Value *castFunc = getAndInsertFuncPtr(builder, C, &F);
    SmallVector<Value*, 1> params;
    params.push_back(castFunc);
    builder.CreateCall(int2kCheck, params);
  }
};

Value* getAndInsertFuncPtr(IRBuilder<> &builder, LLVMContext &C, Function* F)
{
  Value *ptr = builder.CreateAlloca(Type::getInt32PtrTy(C), nullptr);
  Value *cast = builder.CreateBitCast(F, Type::getInt32PtrTy(C));
  builder.CreateStore(cast, ptr);
  return cast;
}

void makeInsertAllFunctionAddrAndName(Module &M)
{
  LLVMContext &C = M.getContext();
  SmallVector<Type*, 0> params;
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("insertAllFunctionAddrAndName", FT);
  insertAllFunctionAddrAndName = M.getFunction("insertAllFunctionAddrAndName");
  BasicBlock* block = BasicBlock::Create(C, "entry", insertAllFunctionAddrAndName);
  IRBuilder<> builder(block);
  for(auto f = M.begin(), fe = M.end(); f!=fe; ++f)
  {
    Function *F = &*f;
    Value *castFunc = getAndInsertFuncPtr(builder, C, F);
    SmallVector<Value*, 1> args;
    args.push_back(castFunc);
    builder.CreateCall(int2kInsertMapAddr, args);
    args.clear();
    Value *funcName = builder.CreateGlobalStringPtr(F->getName());
    args.push_back(funcName);
    builder.CreateCall(int2kInsertMapName, args);
    args.clear();
    //errs() << F->getName() << " ";
  }
  //errs() << "\n";
  SmallVector<Value*, 0> args;
  builder.CreateCall(int2kSortMapAddr, args);
  builder.CreateCall(int2kSortMapName, args);
  builder.CreateRetVoid();
}

// insert call to this function at begining of main
// this function translates ALL functions pointer at run-time into unique ID
void makeMarkFunctionID(Module &M)
{
  LLVMContext &C = M.getContext();
  SmallVector<Type*, 0> params;
  FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
  M.getOrInsertFunction("markFunctionID", FT);
  markFunctionID = M.getFunction("markFunctionID");
  BasicBlock* block = BasicBlock::Create(C, "entry", markFunctionID);
  IRBuilder<> builder(block);
  int funcID = 0;
  Constant *formatStr = builder.CreateGlobalStringPtr("func %s addr %p to %d\n");
  for(auto f = M.begin(), fe = M.end(); f!=fe; ++f)
  {
    Function *F = &*f;
    Value *castFunc = getAndInsertFuncPtr(builder, C, F);
    SmallVector<Value*, 2> args;
    args.push_back(castFunc);
    args.push_back(ConstantInt::get(Type::getInt32Ty(C), funcID));
    builder.CreateCall(int2kTranslateAddr, args);
    Value *funcName = builder.CreateGlobalStringPtr(F->getName());
    args.clear();
    args.push_back(funcName);
    args.push_back(ConstantInt::get(Type::getInt32Ty(C), funcID));
    builder.CreateCall(int2kTranslateName, args);
    args.clear();
    errs() << F->getName() << 
      " will be translated at run-time\n";
    funcID ++;
  }
  builder.CreateRetVoid();
}

// insert call to this function after the MarkFunctionID to 
// make meta data of unrelated functions 
// using _int2k_add_edge
void makeMetaData(Module &M)
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
      //errs() << F1->getName() << " and " << F2->getName() << " \n";
      Value *ptr1 = builder.CreateAlloca(Type::getInt32PtrTy(C), nullptr);
      Value *cast1 = builder.CreateBitCast(F1, Type::getInt32PtrTy(C));
      builder.CreateStore(cast1, ptr1);
      Value *ptr2 = builder.CreateAlloca(Type::getInt32PtrTy(C), nullptr);
      Value *cast2 = builder.CreateBitCast(F2, Type::getInt32PtrTy(C));
      builder.CreateStore(cast2, ptr2);
      //cast1->getType()->print(errs()); errs()<<" ";
      //cast2->getType()->print(errs()); errs()<<"\n";
      SmallVector<Value*, 2> args;
      args.push_back(cast1);
      args.push_back(cast2);
      builder.CreateCall(int2kAddEdge, args);
    }
  builder.CreateRetVoid();
}

namespace
{
  void getPrint(Module &M)
  {
    print = M.getFunction("printf"); // external
    if(print == NULL)
    {
      LLVMContext &C = M.getContext();
      std::vector<Type*> params;
      params.push_back(Type::getInt8PtrTy(C));
      FunctionType *printType = 
        FunctionType::get(Type::getInt32Ty(C), params, true);
      Function *x = Function::Create(printType, 
        Function::ExternalLinkage, "printf", &M);
      print = x;
    }
  }
  void getQuit(Module &M)
  {
    quit = M.getFunction("exit"); // external
    if(quit == NULL)
    {
      LLVMContext &C = M.getContext();
      std::vector<Type*> params;
      params.push_back(Type::getInt32Ty(C));
      FunctionType *exitType = 
        FunctionType::get(Type::getVoidTy(C), params, false);
      Function *x = Function::Create(exitType,
        Function::ExternalLinkage, "exit", &M);
      quit = x;
    }
  }
  void getInt2kCheck(Module &M) // exiter
  {
    LLVMContext &C = M.getContext();
    std::vector<Type *> params;
    params.push_back(Type::getInt32PtrTy(C));// run-time func ptr
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_check", FT);
    int2kCheck = M.getFunction("_int2k_check");
  }
  void getInt2kTranslateAddr(Module &M) // from func ptr to ID
  {
    LLVMContext &C = M.getContext();
    std::vector<Type *> params;
    params.push_back(Type::getInt32PtrTy(C)); // run-time func ptr
    params.push_back(Type::getInt32Ty(C)); //id
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_translateAddr", FT);
    int2kTranslateAddr = M.getFunction("_int2k_translateAddr");
  }
  void getInt2kInitFlag(Module &M) 
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type *, 1> params;
    params.push_back(Type::getInt32Ty(C));
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_init_flag", FT);
    int2kInitFlag = M.getFunction("_int2k_init_flag");
  }
  void getInt2kDeleteFlag(Module &M)
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type *, 0> params;
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_delete_flag", FT);
    int2kDeleteFlag = M.getFunction("_int2k_delete_flag");
  }
  void getInt2kAddEdge(Module &M)
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type*, 2> params;
    for(int i = 0; i < 2; ++i)
      params.push_back(Type::getInt32PtrTy(C));
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_add_edge", FT);
    int2kAddEdge = M.getFunction("_int2k_add_edge");
  }
  void getInt2kHandleArg(Module &M)
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type*, 2> params;
    params.push_back(Type::getInt32Ty(C)); // i32
    params.push_back(PointerType::getUnqual(Type::getInt8PtrTy(C))); //i8**
    FunctionType *FT = FunctionType::get(Type::getInt32Ty(C), params, false);
    M.getOrInsertFunction("_int2k_handle_arg", FT);
    int2kHandleArg = M.getFunction("_int2k_handle_arg");
  }
  void getInt2kTranslateName(Module &M)
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type*, 2> params;
    params.push_back(Type::getInt8PtrTy(C));
    params.push_back(Type::getInt32Ty(C)); 
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_translateName", FT);
    int2kTranslateName = M.getFunction("_int2k_translateName");
  }
  void getInt2kInsertMapAddr(Module &M)
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type*, 1> params;
    params.push_back(Type::getInt32PtrTy(C));
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_insertMapAddr", FT);
    int2kInsertMapAddr = M.getFunction("_int2k_insertMapAddr");
  }
  void getInt2kSortMapAddr(Module &M)
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type*, 0> params;
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_sortMapAddr", FT);
    int2kSortMapAddr = M.getFunction("_int2k_sortMapAddr");
  }
  void getInt2kInsertMapName(Module &M)
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type*, 1> params;
    params.push_back(Type::getInt8PtrTy(C));
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_insertMapName", FT);
    int2kInsertMapName = M.getFunction("_int2k_insertMapName");
  }
  void getInt2kSortMapName(Module &M)
  {
    LLVMContext &C = M.getContext();
    SmallVector<Type*, 0> params;
    FunctionType *FT = FunctionType::get(Type::getVoidTy(C), params, false);
    M.getOrInsertFunction("_int2k_sortMapName", FT);
    int2kSortMapName = M.getFunction("_int2k_sortMapName");
  }
  void writeHandler()
  {
    std::ofstream write;
    write.open("handler.cpp", std::ios::out);
    char const *s = customHandler;
    write << s;
    write.close();
    system("clang++ -emit-llvm handler.cpp -c");
  }
  void insertThings(Module &M)
  {
    getPrint(M);
    getQuit(M);
    getInt2kCheck(M);
    getInt2kTranslateAddr(M);
    getInt2kTranslateName(M);
    makeMarkFunctionID(M);
    getInt2kInitFlag(M);
    getInt2kDeleteFlag(M);
    getInt2kAddEdge(M);
    getInt2kHandleArg(M);
    getInt2kInsertMapAddr(M);
    getInt2kSortMapAddr(M);
    getInt2kInsertMapName(M);
    getInt2kSortMapName(M);
    makeMetaData(M);
    //makeMarkFunctionID(M);
    makeInsertAllFunctionAddrAndName(M);
    writeHandler(); // cpp file
    // insert init and delete at begining and end of main.
    nFunction = 0;
    for(auto f = M.begin(), fe = M.end(); f!=fe; ++f)
    {
      nFunction++;
      func.push_back(&*f);
    }
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
          {
            arg.push_back(i);
          }
        // for loop to print arguments
        BasicBlock *oldEntry = &*(F->begin());
        oldEntry->setName("forEnd");
        BasicBlock *forInc = BasicBlock::Create(C, "forInc", F, oldEntry); 
        BasicBlock *forBody = BasicBlock::Create(C, "forBody", F, forInc);
        BasicBlock *forCond = BasicBlock::Create(C, "forCond", F, forBody);
        BasicBlock *entry = BasicBlock::Create(C, "entry", F, forCond);
        IRBuilder<> builderEntry(entry);
        IRBuilder<> builderForCond(forCond);
        IRBuilder<> builderForBody(forBody);
        IRBuilder<> builderForInc(forInc);
        Value *argcAddr = builderEntry.CreateAlloca(arg[0]->getType(), nullptr, "argcAddr");
        Value *argvAddr = builderEntry.CreateAlloca(arg[1]->getType(), nullptr, "argvAddr");
        Value *i = builderEntry.CreateAlloca(Type::getInt32Ty(C), nullptr, "i");
        Value *str = builderEntry.CreateAlloca(Type::getInt8PtrTy(C), nullptr, "str");
        builderEntry.CreateStore(arg[0], argcAddr);
        builderEntry.CreateStore(arg[1], argvAddr);
        builderEntry.CreateStore(ConstantInt::get(Type::getInt32Ty(C), 0), i);
        builderEntry.CreateBr(forCond);
        Value *tmp0 = builderForCond.CreateLoad(Type::getInt32Ty(C), i, "tmp0");
        Value *tmp1 = builderForCond.CreateLoad(Type::getInt32Ty(C), argcAddr, "tmp1");
        Value *cmp = builderForCond.CreateICmpSLT(tmp0, tmp1, "cmp");
        builderForCond.CreateCondBr(cmp, forBody, oldEntry);
        Value *tmp2 = builderForBody.CreateLoad(arg[1]->getType(), argvAddr, "tmp2");
        Value *tmp3 = builderForBody.CreateLoad(Type::getInt32Ty(C), i, "tmp3");
        Value *idxprom = builderForBody.CreateSExt(tmp3, Type::getInt64Ty(C), "idxprom");
        Value *arrayidx = builderForBody.CreateInBoundsGEP(Type::getInt8PtrTy(C), tmp2, idxprom, "arrayidx");
        Value *tmp4 = builderForBody.CreateLoad(Type::getInt8PtrTy(C), arrayidx, "tmp4");
        builderForBody.CreateStore(tmp4, str);
        Value *tmp5 = builderForBody.CreateLoad(Type::getInt8PtrTy(C), str, "tmp5");
        Value *formatStr = builderForBody.CreateGlobalStringPtr("arg: %s\n");
        SmallVector<Value*, 1> printParam; printParam.push_back(formatStr); printParam.push_back(tmp5);
        builderForBody.CreateCall(print, printParam);
        builderForBody.CreateBr(forInc);
        Value *tmp6 = builderForInc.CreateLoad(Type::getInt32Ty(C), i, "tmp6");
        Value *inc = builderForInc.CreateAdd(tmp6, ConstantInt::get(Type::getInt32Ty(C), 1), "inc", false, true);
        builderForInc.CreateStore(inc, i);
        //Value *tester = builderForInc.CreateAlloca(PointerType::getUnqual(Type::getInt32PtrTy(C)), nullptr, "tester");
        builderForInc.CreateBr(forCond);
        // done the for loop
        BasicBlock *bStart = &*(F->begin());
        BasicBlock *bEnd = &(F->back());
        IRBuilder<> builder(bStart, bStart->begin());
        // insert init flags, metadata
        SmallVector<Value*, 1> params;
        params.push_back(ConstantInt::get(Type::getInt32Ty(C), nFunction));
        builder.CreateCall(int2kInitFlag, params); 
        // insert all funcAddr and Name so the map struct can sort
        SmallVector<Value*, 0> params3;
        builder.CreateCall(insertAllFunctionAddrAndName, params3);
        SmallVector<Value*, 0> params1;
        // map Func Addr and Name to respective ID
        builder.CreateCall(markFunctionID, params1);
        // insert metaData function
        builder.CreateCall(metaData, params1);
        // insert argument handler
        SmallVector<Value*, 2> params2;
        params2.push_back(arg[0]);
        params2.push_back(arg[1]);
        builder.CreateCall(int2kHandleArg, params2);
        BasicBlock::iterator it = bEnd->end();
        it--;
        IRBuilder<> builderEnd(bEnd, it);
        // inser function that delete allocated arrays to free memeory
        params.clear();
        builderEnd.CreateCall(int2kDeleteFlag, params);
        break;
      }
    }
    //
  }
  struct HandlerPass7 : public ModulePass
  {
  public:
    static char ID;
    HandlerPass7() : ModulePass(ID) {};
    virtual bool runOnModule(Module &M)
    {
      insertThings(M);
      std::string cmd = "llvm-link handler.bc "+M.outputFileName+" -o "+M.outputFileName;
      M.postCmd = cmd;
      return true;     
    } 
  };
}

char HandlerPass7::ID =1;
static RegisterPass<HandlerPass7> X("HandlerPass7", "handling with global variable and link external IR functionsand support function blocking via the goddamn argument input :D");

static llvm::RegisterStandardPasses Y(
  llvm::PassManagerBuilder::EP_EarlyAsPossible,
  [](const llvm::PassManagerBuilder &Builder,
    llvm::legacy::PassManagerBase &PM){ PM.add(new HandlerPass7()); });
