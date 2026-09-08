// RUN: rm -rf %t && mkdir -p %t
// RUN: dycc --dayan-root %S/Inputs --device-cxx %clangxx --host-cxx %clangxx \
// RUN:   --save-temps=%t -c %s -o %t/inline-asm.o
// RUN: llvm-objdump --mattr=+txbase,+txlsu,+txmat,+txvec,+txmacro -d \
// RUN:   %t/inline-asm.cpp.dycc.device.o | FileCheck %s

#include <dayan_kernel.h>

__kernel__ void inline_asm_kernel(int *output, int value) {
  asm volatile("addi %0, %0, 1" : "+r"(value));
  asm volatile("tx.base.tcsr.write %0, 0, 0" : : "r"(value));
  asm volatile("tx.lsu.tcsr.write %0, 14" : : "r"(value));
  asm volatile("tx.mat.mm %0, %0, %0, 0" : "+r"(value));
  asm volatile("tx.vec.cvt.fp32.int32 1, 3, 0");
  asm volatile("tx.macro.reduce.add.s32 %0, %0, 0" : "+r"(value));
  *output = value;
}

DAYAN_KERNEL_DESC(inline_asm_kernel, 0, 0, 0, 1, 1, 1, 1);

// CHECK-LABEL: <_Z17inline_asm_kernelPii>:
// CHECK: addi
// CHECK: tx.base.tcsr.write
// CHECK: tx.lsu.tcsr.write
// CHECK: tx.mat.mm
// CHECK: tx.vec.cvt.fp32.int32
// CHECK: tx.macro.reduce.add.s32
