if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

add_llvm_library( LLVMHandlerPass6 MODULE BUILDTREE_ONLY
  HandlerPass6.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
