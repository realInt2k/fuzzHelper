set(handler8Helper_SOURCES
  handler8Helper.cc
  )

  include_directories(..)

  set(handler8Helper_CFLAGS ${SANITIZER_COMMON_CFLAGS})

  add_custom_target(handler8Helper)

  add_compiler_rt_runtime(clang_rt.handler8Helper
    STATIC
    ARCHS x86_64
    SOURCES ${handler8Helper_SOURCES}
    CFLAGS ${handler8Helper_CFLAGS}
    PARENT_TARGET handler8Helper)

add_sanitizer_rt_symbols(clang_rt.handler8Helper)

add_dependencies(compiler-rt handler8Helper)
