# RUN: not llvm-mc %s -triple=riscv64 -show-encoding 2>&1 \
# RUN:   | FileCheck --check-prefix=NOFEATURE %s
# RUN: llvm-mc %s -triple=riscv64 \
# RUN:   -mattr=+txbase,+txlsu,+txmat,+txvec,+txmacro -show-encoding > /dev/null
# RUN: llvm-mc %s -triple=riscv64 \
# RUN:   -mattr=+ztxbase,+ztxlsu,+ztxmat,+ztxvec,+ztxmacro -show-encoding > /dev/null

# NOFEATURE: error: instruction requires the following: 'txbase' or 'ztxbase'
tx.base.tcsr.read a0, 1, 0
# NOFEATURE: error: instruction requires the following: 'txlsu' or 'ztxlsu'
tx.lsu.dma.u2u a0, a1, 0
# NOFEATURE: error: instruction requires the following: 'txmat' or 'ztxmat'
tx.mat.mm a0, a1, a2, 0
# NOFEATURE: error: instruction requires the following: 'txvec' or 'ztxvec'
tx.vec.cvt.fp32.int32 1, 3, 0
# NOFEATURE: error: instruction requires the following: 'txmacro' or 'ztxmacro'
tx.macro.reduce.add.s32 a0, a1, 0
