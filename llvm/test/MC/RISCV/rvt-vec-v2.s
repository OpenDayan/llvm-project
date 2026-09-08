# RUN: llvm-mc %s -triple=riscv64 -mattr=+txvec -filetype=asm | FileCheck %s

tx.vec.vv.int32.rol 1, 3, 7
tx.vec.vv.int32.sll 1, 3, 7
tx.vec.vv.int32.sra 1, 3, 7
tx.vec.vv.int32.srl 1, 3, 7
tx.vec.vv.int32.add 1, 3, 7
tx.vec.vv.int32.sub 1, 3, 7
tx.vec.vv.int32.mulu24 1, 3, 7
tx.vec.vv.int32.mul24 1, 3, 7
tx.vec.vv.int32.bitwise 1, 3, 7, 5
tx.vec.vv.fp32.add 1, 3, 7, 0
tx.vec.vv.fp32.sub 1, 3, 7, 0
tx.vec.vv.fp32.mul 1, 3, 7, 0
tx.vec.vv.mux2 1, 3, 7, 2
tx.vec.vv.int32.cmp 3, 7, 2, 4
tx.vec.vv.fp32.cmp 3, 7, 2, 4
tx.vec.v.int32.sll 1, 3, 4
tx.vec.v.int32.sra 1, 3, 4
tx.vec.v.int32.srl 1, 3, 4
tx.vec.v.float32.sfu 1, 3, 2
tx.vec.cvt.fp32.int32 1, 3, 0
tx.vec.cvt.s32.fp32 1, 3, 0
tx.vec.cvt.fp32.fp16 1, 3, 0
tx.vec.cvt.fp32.bf16 1, 3, 0
tx.vec.cvt.fp16.fp32 1, 3, 0
tx.vec.cvt.bf16.fp32 1, 3, 0
tx.vec.cvt.mxfp8.bf16 1, 3, 0
tx.vec.cvt.mxfp4.bf16 1, 3, 0
tx.vec.s.int32.rol 2, 1, 3
tx.vec.s.int32.sll 2, 1, 3
tx.vec.s.int32.sra 2, 1, 3
tx.vec.s.int32.srl 2, 1, 3
tx.vec.s.int32.add 2, 1, 3
tx.vec.s.int32.sub 2, 1, 3
tx.vec.s.int32.mul 2, 1, 3
tx.vec.s.iter.getdim 2, 1, 3
tx.vec.s.iter.gets 2, 1, 3, 1
tx.vec.s.iter.getv 2, 1, 3, 1
tx.vec.s.bitwise 2, 1, 3, 3
tx.vec.s.bitwise.beq 8, 1, 3
tx.vec.s.bitwise.bne 8, 1, 3
tx.vec.s.int32.blt 8, 1, 3
tx.vec.s.int32.bge 8, 1, 3
tx.vec.lsu.u2v x5, 1, 2
tx.vec.lsu.sm2v x5, 1, 2
tx.vec.lsu.u2v.broadcast32 x5, 1, 2
tx.vec.lsu.sm2v.broadcast32 x5, 1, 2
tx.vec.lsu.s2v.broadcast32 1, 2, 3
tx.vec.lsu.g2v.broadcast32 x5, 1, 2
tx.vec.lsu.u2s x5, 2, 1
tx.vec.lsu.sm2s x5, 2, 1
tx.vec.lsu.g2s x5, 2
tx.vec.lsu.u2v.mask x5, 1, 3, 2
tx.vec.lsu.sm2v.mask x5, 1, 3, 2
tx.vec.lsu.u2v.mask.broadcast32 x5, 1, 3, 2
tx.vec.lsu.sm2v.mask.broadcast32 x5, 1, 3, 2
tx.vec.lsu.s2v.mask.broadcast32 1, 3, 2, 1
tx.vec.lsu.g2v.mask.broadcast32 x5, 1, 3, 2
tx.vec.lsu.v2u x5, 3, 2
tx.vec.lsu.v2sm x5, 3, 2
tx.vec.lsu.s2u x5, 3, 2
tx.vec.lsu.s2sm x5, 3, 2
tx.vec.lsu.v2u.mask x5, 3, 2, 1
tx.vec.lsu.v2sm.mask x5, 3, 2, 1
tx.vec.sm2v.sparse x5, 1, 3, 7, 2
tx.vec.v2sm.sparse x5, 3, 7, 2
tx.vec.v2sm.sparse.int32.add x5, 1, 3, 7, 2
tx.vec.v2sm.sparse.int32.and x5, 1, 3, 7, 2
tx.vec.v2sm.sparse.int32.or x5, 1, 3, 7, 2
tx.vec.v2sm.sparse.int32.xor x5, 1, 3, 7, 2
tx.vec.sm2v.dyn 1, 3, 7, 2, 1
tx.vec.v2sm.dyn 3, 7, 2, 1
tx.vec.v2sm.dyn.int32.add 1, 3, 7, 2, 1
tx.vec.v2sm.dyn.int32.and 1, 3, 7, 2, 1
tx.vec.v2sm.dyn.int32.or 1, 3, 7, 2, 1
tx.vec.v2sm.dyn.int32.xor 1, 3, 7, 2, 1
tx.vec.shuffle.xor x5, 1, 3
tx.vec.shuffle.xor.dyn 1, 3, 2
tx.vec.shuffle.ror x5, 1, 3
tx.vec.shuffle.ror.dyn 1, 3, 2
tx.vec.shuffle.broadcast x5, 1, 3
tx.vec.shuffle.broadcast.dyn 1, 3, 2
tx.vec.shuffle.compress x5, 1, 3
tx.vec.shuffle.compress.dyn 1, 3, 2

# CHECK: tx.vec.vv.int32.rol 1, 3, 7
# CHECK: tx.vec.vv.int32.sll 1, 3, 7
# CHECK: tx.vec.vv.int32.sra 1, 3, 7
# CHECK: tx.vec.vv.int32.srl 1, 3, 7
# CHECK: tx.vec.vv.int32.add 1, 3, 7
# CHECK: tx.vec.vv.int32.sub 1, 3, 7
# CHECK: tx.vec.vv.int32.mulu24 1, 3, 7
# CHECK: tx.vec.vv.int32.mul24 1, 3, 7
# CHECK: tx.vec.vv.int32.bitwise 1, 3, 7, 5
# CHECK: tx.vec.vv.fp32.add 1, 3, 7, 0
# CHECK: tx.vec.vv.fp32.sub 1, 3, 7, 0
# CHECK: tx.vec.vv.fp32.mul 1, 3, 7, 0
# CHECK: tx.vec.vv.mux2 1, 3, 7, 2
# CHECK: tx.vec.vv.int32.cmp 3, 7, 2, 4
# CHECK: tx.vec.vv.fp32.cmp 3, 7, 2, 4
# CHECK: tx.vec.v.int32.sll 1, 3, 4
# CHECK: tx.vec.v.int32.sra 1, 3, 4
# CHECK: tx.vec.v.int32.srl 1, 3, 4
# CHECK: tx.vec.v.float32.sfu 1, 3, 2
# CHECK: tx.vec.cvt.fp32.int32 1, 3, 0
# CHECK: tx.vec.cvt.s32.fp32 1, 3, 0
# CHECK: tx.vec.cvt.fp32.fp16 1, 3, 0
# CHECK: tx.vec.cvt.fp32.bf16 1, 3, 0
# CHECK: tx.vec.cvt.fp16.fp32 1, 3, 0
# CHECK: tx.vec.cvt.bf16.fp32 1, 3, 0
# CHECK: tx.vec.cvt.mxfp8.bf16 1, 3, 0
# CHECK: tx.vec.cvt.mxfp4.bf16 1, 3, 0
# CHECK: tx.vec.s.int32.rol 2, 1, 3
# CHECK: tx.vec.s.int32.sll 2, 1, 3
# CHECK: tx.vec.s.int32.sra 2, 1, 3
# CHECK: tx.vec.s.int32.srl 2, 1, 3
# CHECK: tx.vec.s.int32.add 2, 1, 3
# CHECK: tx.vec.s.int32.sub 2, 1, 3
# CHECK: tx.vec.s.int32.mul 2, 1, 3
# CHECK: tx.vec.s.iter.getdim 2, 1, 3
# CHECK: tx.vec.s.iter.gets 2, 1, 3, 1
# CHECK: tx.vec.s.iter.getv 2, 1, 3, 1
# CHECK: tx.vec.s.bitwise 2, 1, 3, 3
# CHECK: tx.vec.s.bitwise.beq 8, 1, 3
# CHECK: tx.vec.s.bitwise.bne 8, 1, 3
# CHECK: tx.vec.s.int32.blt 8, 1, 3
# CHECK: tx.vec.s.int32.bge 8, 1, 3
# CHECK: tx.vec.lsu.u2v {{(x5|t0)}}, 1, 2
# CHECK: tx.vec.lsu.sm2v {{(x5|t0)}}, 1, 2
# CHECK: tx.vec.lsu.u2v.broadcast32 {{(x5|t0)}}, 1, 2
# CHECK: tx.vec.lsu.sm2v.broadcast32 {{(x5|t0)}}, 1, 2
# CHECK: tx.vec.lsu.s2v.broadcast32 1, 2, 3
# CHECK: tx.vec.lsu.g2v.broadcast32 {{(x5|t0)}}, 1, 2
# CHECK: tx.vec.lsu.u2s {{(x5|t0)}}, 2, 1
# CHECK: tx.vec.lsu.sm2s {{(x5|t0)}}, 2, 1
# CHECK: tx.vec.lsu.g2s {{(x5|t0)}}, 2
# CHECK: tx.vec.lsu.u2v.mask {{(x5|t0)}}, 1, 3, 2
# CHECK: tx.vec.lsu.sm2v.mask {{(x5|t0)}}, 1, 3, 2
# CHECK: tx.vec.lsu.u2v.mask.broadcast32 {{(x5|t0)}}, 1, 3, 2
# CHECK: tx.vec.lsu.sm2v.mask.broadcast32 {{(x5|t0)}}, 1, 3, 2
# CHECK: tx.vec.lsu.s2v.mask.broadcast32 1, 3, 2, 1
# CHECK: tx.vec.lsu.g2v.mask.broadcast32 {{(x5|t0)}}, 1, 3, 2
# CHECK: tx.vec.lsu.v2u {{(x5|t0)}}, 3, 2
# CHECK: tx.vec.lsu.v2sm {{(x5|t0)}}, 3, 2
# CHECK: tx.vec.lsu.s2u {{(x5|t0)}}, 3, 2
# CHECK: tx.vec.lsu.s2sm {{(x5|t0)}}, 3, 2
# CHECK: tx.vec.lsu.v2u.mask {{(x5|t0)}}, 3, 2, 1
# CHECK: tx.vec.lsu.v2sm.mask {{(x5|t0)}}, 3, 2, 1
# CHECK: tx.vec.sm2v.sparse {{(x5|t0)}}, 1, 3, 7, 2
# CHECK: tx.vec.v2sm.sparse {{(x5|t0)}}, 3, 7, 2
# CHECK: tx.vec.v2sm.sparse.int32.add {{(x5|t0)}}, 1, 3, 7, 2
# CHECK: tx.vec.v2sm.sparse.int32.and {{(x5|t0)}}, 1, 3, 7, 2
# CHECK: tx.vec.v2sm.sparse.int32.or {{(x5|t0)}}, 1, 3, 7, 2
# CHECK: tx.vec.v2sm.sparse.int32.xor {{(x5|t0)}}, 1, 3, 7, 2
# CHECK: tx.vec.sm2v.dyn 1, 3, 7, 2, 1
# CHECK: tx.vec.v2sm.dyn 3, 7, 2, 1
# CHECK: tx.vec.v2sm.dyn.int32.add 1, 3, 7, 2, 1
# CHECK: tx.vec.v2sm.dyn.int32.and 1, 3, 7, 2, 1
# CHECK: tx.vec.v2sm.dyn.int32.or 1, 3, 7, 2, 1
# CHECK: tx.vec.v2sm.dyn.int32.xor 1, 3, 7, 2, 1
# CHECK: tx.vec.shuffle.xor {{(x5|t0)}}, 1, 3
# CHECK: tx.vec.shuffle.xor.dyn 1, 3, 2
# CHECK: tx.vec.shuffle.ror {{(x5|t0)}}, 1, 3
# CHECK: tx.vec.shuffle.ror.dyn 1, 3, 2
# CHECK: tx.vec.shuffle.broadcast {{(x5|t0)}}, 1, 3
# CHECK: tx.vec.shuffle.broadcast.dyn 1, 3, 2
# CHECK: tx.vec.shuffle.compress {{(x5|t0)}}, 1, 3
# CHECK: tx.vec.shuffle.compress.dyn 1, 3, 2
