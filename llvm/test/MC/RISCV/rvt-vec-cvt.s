# RUN: llvm-mc %s -triple=riscv64 -mattr=+txvec -show-encoding \
# RUN:   | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+txvec < %s \
# RUN:   | llvm-objdump --mattr=+txvec -d - \
# RUN:   | FileCheck --check-prefix=CHECK-ASM-AND-OBJ %s

# Convert encodings follow the RVT spec:
#   [31:26]=100000, [25]=vb, [24:20]=00010, [19:18]=01 class,
#   [17:15]=cvt op, [14:12]=rm, [11:7]=vrs1, [6:0]=0001011.

# CHECK-ASM-AND-OBJ: tx.vec.cvt.fp32.int32 {{(1|0x1)}}, {{(3|0x3)}}, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x8b,0x01,0x24,0x82]
tx.vec.cvt.fp32.int32 1, 3, 0
# CHECK-ASM-AND-OBJ: tx.vec.cvt.s32.fp32 {{(1|0x1)}}, {{(3|0x3)}}, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x8b,0x81,0x24,0x82]
tx.vec.cvt.s32.fp32 1, 3, 0
# CHECK-ASM-AND-OBJ: tx.vec.cvt.fp32.fp16 {{(1|0x1)}}, {{(3|0x3)}}, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x8b,0x01,0x25,0x82]
tx.vec.cvt.fp32.fp16 1, 3, 0
# CHECK-ASM-AND-OBJ: tx.vec.cvt.fp32.bf16 {{(1|0x1)}}, {{(3|0x3)}}, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x8b,0x81,0x25,0x82]
tx.vec.cvt.fp32.bf16 1, 3, 0
# CHECK-ASM-AND-OBJ: tx.vec.cvt.fp16.fp32 {{(1|0x1)}}, {{(3|0x3)}}, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x8b,0x01,0x26,0x82]
tx.vec.cvt.fp16.fp32 1, 3, 0
# CHECK-ASM-AND-OBJ: tx.vec.cvt.bf16.fp32 {{(1|0x1)}}, {{(3|0x3)}}, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x8b,0x81,0x26,0x82]
tx.vec.cvt.bf16.fp32 1, 3, 0
# CHECK-ASM-AND-OBJ: tx.vec.cvt.mxfp8.bf16 {{(1|0x1)}}, {{(3|0x3)}}, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x8b,0x01,0x27,0x82]
tx.vec.cvt.mxfp8.bf16 1, 3, 0
# CHECK-ASM-AND-OBJ: tx.vec.cvt.mxfp4.bf16 {{(1|0x1)}}, {{(3|0x3)}}, {{(0|0x0)}}
# CHECK-ASM: encoding: [0x8b,0x81,0x27,0x82]
tx.vec.cvt.mxfp4.bf16 1, 3, 0
