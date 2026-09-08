set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_tool(dycc
  dycc.cpp
  )

clang_target_link_libraries(dycc
  PRIVATE
  clangDayan
  )
