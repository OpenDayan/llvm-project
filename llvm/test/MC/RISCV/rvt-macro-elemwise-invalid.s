# RUN: not llvm-mc %s -triple=riscv64 -mattr=+txmacro 2>&1 | FileCheck %s
# CHECK: error:
tx.macro.elemwise.add.fp32 a0, a1, a2, 0, 3
# CHECK: error:
tx.macro.elemwise.vv.max.s32 a0, a1, a2, 8
# CHECK: error:
tx.macro.elemwise.vv.min.fp32 a0, a1, a2, -1

# CHECK: error:
tx.macro.elemwise.max.u32 a0, a1, a2, 0, 3

# Legacy spellings are not part of the ISA.
# CHECK: error:
tx.macro.elemwise.add.fp32 a0, a1, a2, 0, 0
# CHECK: error:
tx.macro.elemwise.max.u32 a0, a1, a2, 0, 1
# CHECK: error:
tx.macro.elemwise.vv.add.s32 a0, a1, 0
# CHECK: error:
tx.macro.elemwise.sv.add.s32 a0, a1, 0
# CHECK: error:
tx.macro.elemwise.iv.add.s32 a0, a1, 0
