# RUN: not llvm-mc %s -triple=riscv64 -mattr=+txlsu -show-encoding 2>&1 \
# RUN:   | FileCheck %s

# The ISA defines .sg, not .gs.
# CHECK: error: unrecognized instruction mnemonic
# CHECK-NEXT: tx.lsu.dma.u2u.gs a0, a1, 0
tx.lsu.dma.u2u.gs a0, a1, 0

# Gather/scatter are selected by mnemonic suffixes, not extra operands.
# CHECK: error: invalid operand for instruction
# CHECK-NEXT: tx.lsu.dma.u2u a0, a1, 0, 1, 1
tx.lsu.dma.u2u a0, a1, 0, 1, 1
