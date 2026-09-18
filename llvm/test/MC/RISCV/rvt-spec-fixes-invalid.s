# RUN: not llvm-mc %s -triple=riscv64 -mattr=+txvec,+txmat,+txmacro 2>&1 | FileCheck %s

# CHECK: error:
tx.vec.s.bitwise 2, 1, 3, 16

# CHECK: immediate must be an integer in the range [-128, 127]
tx.vec.s.bitwise.beq -129, 1, 3

# CHECK: immediate must be an integer in the range [-128, 127]
tx.vec.s.int32.blt 128, 1, 3

# CHECK: error:
tx.vec.s.iter.getdim 2, 4

# CHECK: error:
tx.vec.s.iter.getdim 2, 1, 3

# CHECK: error:
tx.mat.setptr a0, a1, a2, 7

# CHECK: error:
tx.macro.reduce.max.s32 a0, a1, a2, 7, 2

# CHECK: error:
tx.macro.shuffle.compress a0, a1, a2, 8


# Obsolete encodings must not silently assemble as current VEC LSU instructions.
# CHECK: error:
tx.vec.v.load a0, 1, 2
# CHECK: error:
tx.vec.s.load a0, 1, 2
# CHECK: error:
tx.vec.const.load a0, 1
# CHECK: error:
tx.vec.v.store a0, 1, 2
# CHECK: error:
tx.vec.s.store a0, 1, 2
