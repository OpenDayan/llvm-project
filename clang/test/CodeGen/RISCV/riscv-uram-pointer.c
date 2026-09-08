// RUN: %clang_cc1 -triple riscv64 -emit-llvm -o - %s | FileCheck %s

#define __uram__ __attribute__((npu_uram))

// An object declared with __uram__ lives in URAM.
// CHECK: @uram_tile = {{.*}}addrspace(1) global [64 x i32] zeroinitializer{{.*}}section ".npu_uram"
__uram__ int uram_tile[64];

// A pointer into URAM is an ordinary object and stays out of the URAM section.
// CHECK: @uram_cursor = {{.*}}global ptr addrspace(1) null
__uram__ int *uram_cursor;

// CHECK-LABEL: @read_uram
// CHECK: alloca ptr addrspace(1)
// CHECK: load i32, ptr addrspace(1)
int read_uram(__uram__ int *p) { return p[3]; }

// CHECK-LABEL: @copy_dram_to_uram
// CHECK: load i32, ptr %
// CHECK: store i32 %{{.*}}, ptr addrspace(1)
void copy_dram_to_uram(__uram__ int *destination, const int *source) {
  *destination = *source;
}

// Casting between the two address spaces is explicit and free at the machine
// level, because URAM shares the byte-addressed RISC-V space.
// CHECK-LABEL: @to_uram
// CHECK: addrspacecast ptr %{{.*}} to ptr addrspace(1)
__uram__ int *to_uram(int *dram) { return (__uram__ int *)dram; }

// URAM pointers convert implicitly to the generic/DRAM address space.
// CHECK-LABEL: @from_uram
// CHECK: addrspacecast ptr addrspace(1) %{{.*}} to ptr
int *from_uram(__uram__ int *uram) { return uram; }

// Integer URAM addresses convert implicitly to URAM pointers.
// CHECK-LABEL: @from_integer
// CHECK: inttoptr
__uram__ int *from_integer(unsigned long addr) { return addr; }
