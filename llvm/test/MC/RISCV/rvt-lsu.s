# RUN: llvm-mc %s -triple=riscv64 -mattr=+txlsu -show-encoding \
# RUN:   | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+txlsu < %s \
# RUN:   | llvm-objdump --mattr=+txlsu -d - \
# RUN:   | FileCheck --check-prefix=CHECK-ASM-AND-OBJ %s

# RequestReadSize.
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.write a0, {{(14|0xe)}}
# CHECK-ASM: encoding: [0x0b,0xc5,0x81,0x00]
tx.lsu.tcsr.write a0, 14

# RequestWriteSize, TableSize, and TableStride.
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.write a0, {{(18|0x12)}}
# CHECK-ASM: encoding: [0x0b,0x45,0x82,0x00]
tx.lsu.tcsr.write a0, 18
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.write a0, {{(19|0x13)}}
# CHECK-ASM: encoding: [0x0b,0x65,0x82,0x00]
tx.lsu.tcsr.write a0, 19
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.write a0, {{(20|0x14)}}
# CHECK-ASM: encoding: [0x0b,0x85,0x82,0x00]
tx.lsu.tcsr.write a0, 20

# The same specification-defined offsets are readable through the generic LSU
# TCSR read instruction.
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.read a0, {{(14|0xe)}}
# CHECK-ASM: encoding: [0x0b,0xd5,0x81,0x00]
tx.lsu.tcsr.read a0, 14
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.read a0, {{(18|0x12)}}
# CHECK-ASM: encoding: [0x0b,0x55,0x82,0x00]
tx.lsu.tcsr.read a0, 18
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.read a0, {{(19|0x13)}}
# CHECK-ASM: encoding: [0x0b,0x75,0x82,0x00]
tx.lsu.tcsr.read a0, 19
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.read a0, {{(20|0x14)}}
# CHECK-ASM: encoding: [0x0b,0x95,0x82,0x00]
tx.lsu.tcsr.read a0, 20

# LSU TCSR address-space upper boundary (0x0ff).
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.write a0, {{(255|0xff)}}
# CHECK-ASM: encoding: [0x0b,0xe5,0x9f,0x00]
tx.lsu.tcsr.write a0, 255
# CHECK-ASM-AND-OBJ: tx.lsu.tcsr.read a0, {{(255|0xff)}}
# CHECK-ASM: encoding: [0x0b,0xf5,0x9f,0x00]
tx.lsu.tcsr.read a0, 255

# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2u a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0x85,0x35,0x08]
tx.lsu.dma.u2u a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2u.g a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0x85,0xb5,0x08]
tx.lsu.dma.u2u.g a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2u.s a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xc5,0x35,0x08]
tx.lsu.dma.u2u.s a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2u.sg a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xc5,0xb5,0x08]
tx.lsu.dma.u2u.sg a0, a1, 3

# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2d a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0x95,0x35,0x08]
tx.lsu.dma.u2d a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2d.g a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0x95,0xb5,0x08]
tx.lsu.dma.u2d.g a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2d.s a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xd5,0x35,0x08]
tx.lsu.dma.u2d.s a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2d.sg a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xd5,0xb5,0x08]
tx.lsu.dma.u2d.sg a0, a1, 3

# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2u a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xa5,0x35,0x08]
tx.lsu.dma.d2u a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2u.g a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xa5,0xb5,0x08]
tx.lsu.dma.d2u.g a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2u.s a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xe5,0x35,0x08]
tx.lsu.dma.d2u.s a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2u.sg a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xe5,0xb5,0x08]
tx.lsu.dma.d2u.sg a0, a1, 3

# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2d a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xb5,0x35,0x08]
tx.lsu.dma.d2d a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2d.g a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xb5,0xb5,0x08]
tx.lsu.dma.d2d.g a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2d.s a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xf5,0x35,0x08]
tx.lsu.dma.d2d.s a0, a1, 3
# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2d.sg a0, a1, {{(3|0x3)}}
# CHECK-ASM: encoding: [0x0b,0xf5,0xb5,0x08]
tx.lsu.dma.d2d.sg a0, a1, 3

# Group field boundaries.
# CHECK-ASM-AND-OBJ: tx.lsu.dma.u2u a0, a1, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x0b,0x85,0x05,0x08]
tx.lsu.dma.u2u a0, a1, 0
# CHECK-ASM-AND-OBJ: tx.lsu.dma.d2d.sg a0, a1, {{(7|0x7)}}
# CHECK-ASM: encoding: [0x0b,0xf5,0xf5,0x08]
tx.lsu.dma.d2d.sg a0, a1, 7
