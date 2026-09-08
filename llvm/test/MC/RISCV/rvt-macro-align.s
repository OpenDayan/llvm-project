# RUN: llvm-mc %s -triple=riscv64 -mattr=+txmacro -filetype=asm \
# RUN:   | FileCheck --check-prefix=CHECK-ASM %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+txmacro -filetype=obj \
# RUN:   | llvm-objdump --mattr=+txmacro -d - \
# RUN:   | FileCheck --check-prefix=CHECK-OBJ %s

# 64-bit tensor instructions must be 8-byte aligned. After a 32-bit
# instruction the assembler inserts padding so the 64-bit encoding starts
# on an 8-byte boundary.

# CHECK-ASM: addi a0, a0, 1
# CHECK-ASM-NEXT: .p2align{{.*}}3
# CHECK-ASM-NEXT: tx.macro.reduce.add.s32 a0, a1, {{0|0x0}}
#
# CHECK-OBJ: addi a0, a0, 0x1
# CHECK-OBJ-NEXT: {{nop|addi zero, zero, 0}}
# CHECK-OBJ-NEXT: tx.macro.reduce.add.s32 a0, a1, 0x0
addi a0, a0, 1
tx.macro.reduce.add.s32 a0, a1, 0
