# RUN: not llvm-mc %s -triple=riscv64 -mattr=+txlsu -show-encoding 2>&1 \
# RUN:   | FileCheck %s

# CHECK: error: immediate must be an integer in the range [0, 7]
# CHECK-NEXT: tx.lsu.dma.u2u.g a0, a1, -1
tx.lsu.dma.u2u.g a0, a1, -1

# CHECK: error: immediate must be an integer in the range [0, 7]
# CHECK-NEXT: tx.lsu.dma.d2d.sg a0, a1, 8
tx.lsu.dma.d2d.sg a0, a1, 8

# CHECK: error: immediate must be an integer in the range [0, 255]
# CHECK-NEXT: tx.lsu.tcsr.read a0, -1
tx.lsu.tcsr.read a0, -1

# CHECK: error: immediate must be an integer in the range [0, 255]
# CHECK-NEXT: tx.lsu.tcsr.write a0, 256
tx.lsu.tcsr.write a0, 256
