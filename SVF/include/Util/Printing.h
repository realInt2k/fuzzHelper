#ifndef PRINTING_H_
#define PRINTING_H_

#include <llvm/Support/raw_ostream.h>
#include "llvm/ADT/StringRef.h"

llvm::raw_fd_ostream &writeGraph(llvm::StringRef str);
llvm::raw_fd_ostream &writeInfo(llvm::StringRef str);

#endif