// RUN: not %clang_cc1 -triple riscv64 -target-feature +txlsu -std=c++14 \
// RUN:   -internal-isystem %S/../../../lib/Headers -fsyntax-only %s 2>&1 \
// RUN:   | FileCheck %s

#include <rvt_intrinsics.h>

// CHECK: static assertion failed due to requirement
// CHECK-SAME: LSU TCSR group must be in the range [0, 7]
constexpr long invalid_group =
    __rvt_lsu_tcsr_addr<8>(__RVT_LSU_TCSR_TABLE_STRIDE);
