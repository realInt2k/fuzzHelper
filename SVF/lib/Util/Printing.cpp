#include "Util/Printing.h"  

#include "SVF-FE/LLVMUtil.h"
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

llvm::raw_fd_ostream &writeGraph(llvm::StringRef str) {
   std::error_code EC;
   static llvm::raw_fd_ostream S1(str, EC, llvm::sys::fs::FileAccess::FA_Write);
   return S1;
}

llvm::raw_fd_ostream &writeInfo(llvm::StringRef str) {
   std::error_code EC;
   static llvm::raw_fd_ostream S2(str, EC, llvm::sys::fs::FileAccess::FA_Write);
   return S2;
}