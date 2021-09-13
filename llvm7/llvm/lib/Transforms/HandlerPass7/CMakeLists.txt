if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

add_llvm_library( LLVMHandlerPass7 MODULE BUILDTREE_ONLY
  HandlerPass7.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
