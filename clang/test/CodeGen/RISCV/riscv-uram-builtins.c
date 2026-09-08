// RUN: %clang_cc1 -triple riscv64 -target-feature +txbase -emit-llvm -o - %s | FileCheck %s

#define __uram__ __attribute__((npu_uram))

__uram__ int uram_scalar;

int test_local_uram_scalar(void) {
  __uram__ int local_uram_scalar;
  local_uram_scalar = 7;
  return local_uram_scalar;
}

int test_is_uram(long addr) {
  return __builtin_riscv_is_uram(addr);
}

int test_uram_scalar_load(void) {
  return uram_scalar;
}

void test_uram_scalar_store(int value) {
  uram_scalar = value;
}

// CHECK-LABEL: @test_local_uram_scalar.local_uram_scalar
// CHECK-SAME: internal addrspace
// CHECK-SAME: global i32 0
// CHECK-SAME: section ".npu_uram"
// CHECK-LABEL: @uram_scalar
// CHECK-SAME: addrspace
// CHECK-SAME: global i32 0
// CHECK-SAME: section ".npu_uram"
//
// CHECK-LABEL: @test_local_uram_scalar
// CHECK: store i32 7, ptr addrspace
// CHECK-SAME: local_uram_scalar
// CHECK: load i32, ptr addrspace
// CHECK-SAME: local_uram_scalar
//
// CHECK-LABEL: @test_is_uram
// CHECK: icmp ult i64 %{{.*}}, 1572864
// CHECK: zext i1 %{{.*}} to i32
//
// CHECK-LABEL: @test_uram_scalar_load
// CHECK: load i32, ptr addrspace
// CHECK-SAME: @uram_scalar
//
// CHECK-LABEL: @test_uram_scalar_store
// CHECK: store i32 %{{.*}}, ptr addrspace
// CHECK-SAME: @uram_scalar
