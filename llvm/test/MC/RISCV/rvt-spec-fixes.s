# RUN: llvm-mc %s -triple=riscv64 -mattr=+txvec,+txmat,+txmacro -show-encoding | FileCheck %s --check-prefixes=ASM,COMMON
# RUN: llvm-mc %s -triple=riscv64 -mattr=+ztxvec,+ztxmat,+ztxmacro -show-encoding | FileCheck %s --check-prefixes=ASM,COMMON
# RUN: llvm-mc %s -triple=riscv64 -mattr=+txvec,+txmat,+txmacro -filetype=obj | llvm-objdump --mattr=+txvec,+txmat,+txmacro --no-print-imm-hex -d - | FileCheck %s --check-prefix=COMMON

# COMMON: tx.vec.s.bitwise 2, 1, 3, 0
# ASM: encoding: [0x8b,0x30,0x02,0xf9]
tx.vec.s.bitwise 2, 1, 3, 0

# COMMON: tx.vec.s.bitwise 2, 1, 3, 1
# ASM: encoding: [0x8b,0x34,0x02,0xf9]
tx.vec.s.bitwise 2, 1, 3, 1

# COMMON: tx.vec.s.bitwise 2, 1, 3, 2
# ASM: encoding: [0x8b,0x38,0x02,0xf9]
tx.vec.s.bitwise 2, 1, 3, 2

# COMMON: tx.vec.s.bitwise 2, 1, 3, 3
# ASM: encoding: [0x8b,0x3c,0x02,0xf9]
tx.vec.s.bitwise 2, 1, 3, 3

# COMMON: tx.vec.s.bitwise 2, 1, 3, 4
# ASM: encoding: [0x8b,0xb0,0x02,0xf9]
tx.vec.s.bitwise 2, 1, 3, 4

# COMMON: tx.vec.s.bitwise 2, 1, 3, 5
# ASM: encoding: [0x8b,0xb4,0x02,0xf9]
tx.vec.s.bitwise 2, 1, 3, 5

# COMMON: tx.vec.s.bitwise 2, 1, 3, 6
# ASM: encoding: [0x8b,0xb8,0x02,0xf9]
tx.vec.s.bitwise 2, 1, 3, 6

# COMMON: tx.vec.s.bitwise 2, 1, 3, 7
# ASM: encoding: [0x8b,0xbc,0x02,0xf9]
tx.vec.s.bitwise 2, 1, 3, 7

# COMMON: tx.vec.s.bitwise 2, 1, 3, 8
# ASM: encoding: [0x8b,0x30,0x03,0xf9]
tx.vec.s.bitwise 2, 1, 3, 8

# COMMON: tx.vec.s.bitwise 2, 1, 3, 9
# ASM: encoding: [0x8b,0x34,0x03,0xf9]
tx.vec.s.bitwise 2, 1, 3, 9

# COMMON: tx.vec.s.bitwise 2, 1, 3, 10
# ASM: encoding: [0x8b,0x38,0x03,0xf9]
tx.vec.s.bitwise 2, 1, 3, 10

# COMMON: tx.vec.s.bitwise 2, 1, 3, 11
# ASM: encoding: [0x8b,0x3c,0x03,0xf9]
tx.vec.s.bitwise 2, 1, 3, 11

# COMMON: tx.vec.s.bitwise 2, 1, 3, 12
# ASM: encoding: [0x8b,0xb0,0x03,0xf9]
tx.vec.s.bitwise 2, 1, 3, 12

# COMMON: tx.vec.s.bitwise 2, 1, 3, 13
# ASM: encoding: [0x8b,0xb4,0x03,0xf9]
tx.vec.s.bitwise 2, 1, 3, 13

# COMMON: tx.vec.s.bitwise 2, 1, 3, 14
# ASM: encoding: [0x8b,0xb8,0x03,0xf9]
tx.vec.s.bitwise 2, 1, 3, 14

# COMMON: tx.vec.s.bitwise 2, 1, 3, 15
# ASM: encoding: [0x8b,0xbc,0x03,0xf9]
tx.vec.s.bitwise 2, 1, 3, 15

# COMMON: tx.vec.s.bitwise.beq -128, 1, 3
# ASM: encoding: [0x8b,0x30,0x40,0xfc]
tx.vec.s.bitwise.beq -128, 1, 3

# COMMON: tx.vec.s.bitwise.beq -1, 1, 3
# ASM: encoding: [0x8b,0xb0,0x7f,0xfc]
tx.vec.s.bitwise.beq -1, 1, 3

# COMMON: tx.vec.s.bitwise.beq 0, 1, 3
# ASM: encoding: [0x8b,0x30,0x00,0xfc]
tx.vec.s.bitwise.beq 0, 1, 3

# COMMON: tx.vec.s.bitwise.beq 127, 1, 3
# ASM: encoding: [0x8b,0xb0,0x3f,0xfc]
tx.vec.s.bitwise.beq 127, 1, 3

# COMMON: tx.vec.s.bitwise.bne -128, 1, 3
# ASM: encoding: [0x8b,0x34,0x40,0xfc]
tx.vec.s.bitwise.bne -128, 1, 3

# COMMON: tx.vec.s.bitwise.bne -1, 1, 3
# ASM: encoding: [0x8b,0xb4,0x7f,0xfc]
tx.vec.s.bitwise.bne -1, 1, 3

# COMMON: tx.vec.s.bitwise.bne 0, 1, 3
# ASM: encoding: [0x8b,0x34,0x00,0xfc]
tx.vec.s.bitwise.bne 0, 1, 3

# COMMON: tx.vec.s.bitwise.bne 127, 1, 3
# ASM: encoding: [0x8b,0xb4,0x3f,0xfc]
tx.vec.s.bitwise.bne 127, 1, 3

# COMMON: tx.vec.s.int32.blt -128, 1, 3
# ASM: encoding: [0x8b,0x38,0x40,0xfc]
tx.vec.s.int32.blt -128, 1, 3

# COMMON: tx.vec.s.int32.blt -1, 1, 3
# ASM: encoding: [0x8b,0xb8,0x7f,0xfc]
tx.vec.s.int32.blt -1, 1, 3

# COMMON: tx.vec.s.int32.blt 0, 1, 3
# ASM: encoding: [0x8b,0x38,0x00,0xfc]
tx.vec.s.int32.blt 0, 1, 3

# COMMON: tx.vec.s.int32.blt 127, 1, 3
# ASM: encoding: [0x8b,0xb8,0x3f,0xfc]
tx.vec.s.int32.blt 127, 1, 3

# COMMON: tx.vec.s.int32.bge -128, 1, 3
# ASM: encoding: [0x8b,0x3c,0x40,0xfc]
tx.vec.s.int32.bge -128, 1, 3

# COMMON: tx.vec.s.int32.bge -1, 1, 3
# ASM: encoding: [0x8b,0xbc,0x7f,0xfc]
tx.vec.s.int32.bge -1, 1, 3

# COMMON: tx.vec.s.int32.bge 0, 1, 3
# ASM: encoding: [0x8b,0x3c,0x00,0xfc]
tx.vec.s.int32.bge 0, 1, 3

# COMMON: tx.vec.s.int32.bge 127, 1, 3
# ASM: encoding: [0x8b,0xbc,0x3f,0xfc]
tx.vec.s.int32.bge 127, 1, 3

# COMMON: tx.vec.s.iter.getdim 2, 0
# ASM: encoding: [0x0b,0x8c,0x00,0xf9]
tx.vec.s.iter.getdim 2, 0

# COMMON: tx.vec.s.iter.getdim 2, 1
# ASM: encoding: [0x8b,0x8c,0x00,0xf9]
tx.vec.s.iter.getdim 2, 1

# COMMON: tx.vec.s.iter.getdim 2, 2
# ASM: encoding: [0x0b,0x8d,0x00,0xf9]
tx.vec.s.iter.getdim 2, 2

# COMMON: tx.vec.s.iter.getdim 2, 3
# ASM: encoding: [0x8b,0x8d,0x00,0xf9]
tx.vec.s.iter.getdim 2, 3

# COMMON: tx.mat.setptr a1, a2, 7
# ASM: encoding: [0x0b,0xc0,0xc5,0x2e]
tx.mat.setptr a1, a2, 7

# COMMON: tx.mat.mm a0, a1, a2, 7
# ASM: encoding: [0x0b,0x85,0xc5,0x2e]
tx.mat.mm a0, a1, a2, 7

# COMMON: tx.mat.mma a0, a1, a2, 7
# ASM: encoding: [0x0b,0x95,0xc5,0x2e]
tx.mat.mma a0, a1, a2, 7

# COMMON: tx.mat.conv2d a0, a1, a2, 7
# ASM: encoding: [0x0b,0xa5,0xc5,0x2e]
tx.mat.conv2d a0, a1, a2, 7

# COMMON: tx.mat.conv2da a0, a1, a2, 7
# ASM: encoding: [0x0b,0xb5,0xc5,0x2e]
tx.mat.conv2da a0, a1, a2, 7

.p2align 3
# COMMON: tx.macro.reduce.add.s32 a0, a1, 7
# ASM: encoding: [0x3f,0xf5,0x05,0x06,0x10,0x00,0x00,0x00]
tx.macro.reduce.add.s32 a0, a1, 7

# COMMON: tx.macro.reduce.max.s32 a0, a1, a2, 7, 0
# ASM: encoding: [0x3f,0xf5,0xc5,0x06,0x10,0x04,0x00,0x00]
tx.macro.reduce.max.s32 a0, a1, a2, 7, 0

# COMMON: tx.macro.reduce.max.s32 a0, a1, a2, 7, 1
# ASM: encoding: [0x3f,0xf5,0xc5,0x06,0x10,0x44,0x00,0x00]
tx.macro.reduce.max.s32 a0, a1, a2, 7, 1

# COMMON: tx.macro.reduce.min.s32 a0, a1, a2, 7, 0
# ASM: encoding: [0x3f,0xf5,0xc5,0x06,0x10,0x05,0x00,0x00]
tx.macro.reduce.min.s32 a0, a1, a2, 7, 0

# COMMON: tx.macro.reduce.min.s32 a0, a1, a2, 7, 1
# ASM: encoding: [0x3f,0xf5,0xc5,0x06,0x10,0x45,0x00,0x00]
tx.macro.reduce.min.s32 a0, a1, a2, 7, 1

# COMMON: tx.macro.shuffle.compress a0, a1, a2, 7
# ASM: encoding: [0x3f,0xf5,0xc5,0x0a,0x01,0x00,0x00,0x00]
tx.macro.shuffle.compress a0, a1, a2, 7

# COMMON: tx.macro.sfu.log2 a0, a1, 7
# ASM: encoding: [0x3f,0xf5,0x05,0x04,0x00,0x00,0x00,0x00]
tx.macro.sfu.log2 a0, a1, 7

# COMMON: tx.macro.sfu.exp2 a0, a1, 7
# ASM: encoding: [0x3f,0xf5,0x05,0x04,0x01,0x00,0x00,0x00]
tx.macro.sfu.exp2 a0, a1, 7

# COMMON: tx.macro.sfu.sin a0, a1, 7
# ASM: encoding: [0x3f,0xf5,0x05,0x04,0x02,0x00,0x00,0x00]
tx.macro.sfu.sin a0, a1, 7

# COMMON: tx.macro.sfu.cos a0, a1, 7
# ASM: encoding: [0x3f,0xf5,0x05,0x04,0x03,0x00,0x00,0x00]
tx.macro.sfu.cos a0, a1, 7

# COMMON: tx.macro.sfu.rsqrt a0, a1, 7
# ASM: encoding: [0x3f,0xf5,0x05,0x04,0x04,0x00,0x00,0x00]
tx.macro.sfu.rsqrt a0, a1, 7

# COMMON: tx.macro.sfu.recip a0, a1, 7
# ASM: encoding: [0x3f,0xf5,0x05,0x04,0x05,0x00,0x00,0x00]
tx.macro.sfu.recip a0, a1, 7

