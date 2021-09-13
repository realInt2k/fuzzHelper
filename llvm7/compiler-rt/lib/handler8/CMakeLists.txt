set(handler8_SOURCES
  handler8.cc
  )

  include_directories(..)

  set(handler8_CFLAGS ${SANITIZER_COMMON_CFLAGS})

  add_custom_target(handler8)

  add_compiler_rt_runtime(clang_rt.handler8
    STATIC
    ARCHS x86_64
    SOURCES ${handler8_SOURCES}
    CFLAGS ${handler8_CFLAGS}
    PARENT_TARGET handler8)

add_sanitizer_rt_symbols(clang_rt.handler8)

add_dependencies(compiler-rt handler8)
