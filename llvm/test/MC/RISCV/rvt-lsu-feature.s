# RUN: not llvm-mc %s -triple=riscv64 -show-encoding 2>&1 \
# RUN:   | FileCheck --check-prefix=NOFEATURE %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+txlsu -show-encoding > /dev/null
# RUN: llvm-mc %s -triple=riscv64 -mattr=+ztxlsu -show-encoding > /dev/null

# NOFEATURE: error: instruction requires the following: 'txlsu' or 'ztxlsu'
tx.lsu.dma.u2u a0, a1, 0
