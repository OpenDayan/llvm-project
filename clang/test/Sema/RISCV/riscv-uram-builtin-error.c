// RUN: %clang_cc1 -triple riscv64 -target-feature +txbase -fsyntax-only -verify %s

int test_uram_negative(void) {
  return __builtin_riscv_is_uram(-1); // expected-error {{argument value -1 is outside the valid range [0, 1572863]}}
}

int test_uram_too_large(void) {
  return __builtin_riscv_is_uram(0x180000); // expected-error {{argument value 1572864 is outside the valid range [0, 1572863]}}
}

int test_uram_top_of_window(void) {
  return __builtin_riscv_is_uram(0x17ffff);
}

int test_uram_dynamic(long addr) {
  return __builtin_riscv_is_uram(addr);
}
