cmake \
  -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" \
  -DLLVM_TARGETS_TO_BUILD="X86" \
  -DLLVM_ABI_BREAKING_CHECKS="FORCE_OFF"\
  -DCMAKE_CXX_FLAGS="-Wno-narrowing" \
  -GNinja \
  ../llvm
