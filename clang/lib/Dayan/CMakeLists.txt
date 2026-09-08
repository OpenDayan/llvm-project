set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_library(clangDayan
  Dayan.cpp

  LINK_LIBS
  clangAST
  clangBasic
  clangFrontend
  clangLex
  clangTooling
  )
