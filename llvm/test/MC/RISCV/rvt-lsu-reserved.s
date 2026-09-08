# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+txlsu < %s \
# RUN:   | llvm-objdump --mattr=+txlsu -d - \
# RUN:   | FileCheck %s

# Bit 24 is reserved and must not be decoded as a valid LSU DMA instruction.
# CHECK: .word 0x0935850b
# CHECK-NOT: tx.lsu.dma
.word 0x0935850b
