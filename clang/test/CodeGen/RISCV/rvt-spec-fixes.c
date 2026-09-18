// RUN: %clang_cc1 -triple riscv64 -target-feature +txvec -target-feature +txmat -target-feature +txmacro -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple riscv64 -target-feature +txvec -target-feature +txmat -target-feature +txmacro -emit-obj -o %t %s

// CHECK-LABEL: @test_mat_setptr(
// CHECK: call void asm sideeffect "tx.mat.setptr $0, $1, $2", "r,r,i"
void test_mat_setptr(long a, long b) {
  __builtin_riscv_tx_mat_setptr(a, b, 7);
}

// CHECK-LABEL: @test_vec_s_bitwise(
// CHECK: call void asm sideeffect "tx.vec.s.bitwise $0, $1, $2, $3", "i,i,i,i"
void test_vec_s_bitwise(void) {
  __builtin_riscv_tx_vec_s_bitwise(2, 1, 3, 6);
}

// CHECK-LABEL: @test_vec_s_iter_getdim(
// CHECK: call void asm sideeffect "tx.vec.s.iter.getdim $0, $1", "i,i"
void test_vec_s_iter_getdim(void) {
  __builtin_riscv_tx_vec_s_iter_getdim(2, 3);
}

// CHECK-LABEL: @test_macro_shuffle_compress(
// CHECK: call void asm sideeffect "tx.macro.shuffle.compress $0, $1, $2, $3", "r,r,r,i"
void test_macro_shuffle_compress(long d, long a, long b) {
  __builtin_riscv_tx_macro_shuffle_compress(d, a, b, 7);
}

// CHECK-LABEL: @test_vec_s_bitwise_beq(
// CHECK: call void asm sideeffect "tx.vec.s.bitwise.beq $0, $1, $2", "i,i,i"
// CHECK-SAME: (i32 -128, i32 1, i32 3)
void test_vec_s_bitwise_beq(void) {
  __builtin_riscv_tx_vec_s_bitwise_beq(-128, 1, 3);
}

// CHECK-LABEL: @test_vec_s_bitwise_bne(
// CHECK: call void asm sideeffect "tx.vec.s.bitwise.bne $0, $1, $2", "i,i,i"
// CHECK-SAME: (i32 -128, i32 1, i32 3)
void test_vec_s_bitwise_bne(void) {
  __builtin_riscv_tx_vec_s_bitwise_bne(-128, 1, 3);
}

// CHECK-LABEL: @test_vec_s_int32_blt(
// CHECK: call void asm sideeffect "tx.vec.s.int32.blt $0, $1, $2", "i,i,i"
// CHECK-SAME: (i32 -128, i32 1, i32 3)
void test_vec_s_int32_blt(void) {
  __builtin_riscv_tx_vec_s_int32_blt(-128, 1, 3);
}

// CHECK-LABEL: @test_vec_s_int32_bge(
// CHECK: call void asm sideeffect "tx.vec.s.int32.bge $0, $1, $2", "i,i,i"
// CHECK-SAME: (i32 -128, i32 1, i32 3)
void test_vec_s_int32_bge(void) {
  __builtin_riscv_tx_vec_s_int32_bge(-128, 1, 3);
}

// CHECK-LABEL: @test_macro_reduce_max_s32(
// CHECK: call void asm sideeffect "tx.macro.reduce.max.s32 $0, $1, $2, $3, $4", "r,r,r,i,i"
void test_macro_reduce_max_s32(long d, long a, long i) {
  __builtin_riscv_tx_macro_reduce_max_s32(d, a, i, 7, 1);
}

// CHECK-LABEL: @test_macro_reduce_min_s32(
// CHECK: call void asm sideeffect "tx.macro.reduce.min.s32 $0, $1, $2, $3, $4", "r,r,r,i,i"
void test_macro_reduce_min_s32(long d, long a, long i) {
  __builtin_riscv_tx_macro_reduce_min_s32(d, a, i, 7, 1);
}

// CHECK-LABEL: @test_macro_sfu_log2(
// CHECK: call void asm sideeffect "tx.macro.sfu.log2 $0, $1, $2", "r,r,i"
void test_macro_sfu_log2(long d, long a) {
  __builtin_riscv_tx_macro_sfu_log2(d, a, 7);
}

// CHECK-LABEL: @test_macro_sfu_exp2(
// CHECK: call void asm sideeffect "tx.macro.sfu.exp2 $0, $1, $2", "r,r,i"
void test_macro_sfu_exp2(long d, long a) {
  __builtin_riscv_tx_macro_sfu_exp2(d, a, 7);
}

// CHECK-LABEL: @test_macro_sfu_sin(
// CHECK: call void asm sideeffect "tx.macro.sfu.sin $0, $1, $2", "r,r,i"
void test_macro_sfu_sin(long d, long a) {
  __builtin_riscv_tx_macro_sfu_sin(d, a, 7);
}

// CHECK-LABEL: @test_macro_sfu_cos(
// CHECK: call void asm sideeffect "tx.macro.sfu.cos $0, $1, $2", "r,r,i"
void test_macro_sfu_cos(long d, long a) {
  __builtin_riscv_tx_macro_sfu_cos(d, a, 7);
}

// CHECK-LABEL: @test_macro_sfu_rsqrt(
// CHECK: call void asm sideeffect "tx.macro.sfu.rsqrt $0, $1, $2", "r,r,i"
void test_macro_sfu_rsqrt(long d, long a) {
  __builtin_riscv_tx_macro_sfu_rsqrt(d, a, 7);
}

// CHECK-LABEL: @test_macro_sfu_recip(
// CHECK: call void asm sideeffect "tx.macro.sfu.recip $0, $1, $2", "r,r,i"
void test_macro_sfu_recip(long d, long a) {
  __builtin_riscv_tx_macro_sfu_recip(d, a, 7);
}

