// RUN: %clang_cc1 -triple riscv64 -target-feature +txlsu -target-feature +txmat -target-feature +txbase -target-feature +txvec -target-feature +txmacro -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple riscv64 -target-feature +ztxlsu -target-feature +txmat -target-feature +txbase -target-feature +txvec -target-feature +txmacro -emit-llvm -o - %s | FileCheck %s
long test_lsu_tcsr_read_builtin(void) {
  return __builtin_riscv_tx_lsu_tcsr_read(3);
}

void test_lsu_tcsr_write_builtin(long v) {
  __builtin_riscv_tx_lsu_tcsr_write(v, 3);
}

void test_lsu_tcsr_extended_fields_builtin(long v) {
  __builtin_riscv_tx_lsu_tcsr_write(v, 14); // RequestReadSize
  __builtin_riscv_tx_lsu_tcsr_write(v, 18); // RequestWriteSize
  __builtin_riscv_tx_lsu_tcsr_write(v, 19); // TableSize
  __builtin_riscv_tx_lsu_tcsr_write(v, 20); // TableStride
}

void test_lsu_dma_d2u_builtin(long rd, long rs1) {
  __builtin_riscv_tx_lsu_dma_d2u(rd, rs1, 1);
}

void test_lsu_dma_u2u_builtin(long rd, long rs1) {
  __builtin_riscv_tx_lsu_dma_u2u(rd, rs1, 1);
}

void test_lsu_dma_u2d_builtin(long rd, long rs1) {
  __builtin_riscv_tx_lsu_dma_u2d(rd, rs1, 1);
}

void test_lsu_dma_d2d_builtin(long rd, long rs1) {
  __builtin_riscv_tx_lsu_dma_d2d(rd, rs1, 1);
}

void test_lsu_dma_table_modes_builtin(long rd, long rs1) {
  __builtin_riscv_tx_lsu_dma_u2u_g(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_u2u_s(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_u2u_sg(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_u2d_g(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_u2d_s(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_u2d_sg(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_d2u_g(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_d2u_s(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_d2u_sg(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_d2d_g(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_d2d_s(rd, rs1, 1);
  __builtin_riscv_tx_lsu_dma_d2d_sg(rd, rs1, 1);
}

long test_mat_tcsr_read_builtin(void) {
  return __builtin_riscv_tx_mat_tcsr_read(2);
}

void test_mat_tcsr_write_builtin(long v) {
  __builtin_riscv_tx_mat_tcsr_write(v, 2);
}

void test_mat_setptr_builtin(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_setptr(rd, rs1, rs2, 2);
}

void test_mat_mm_builtin(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_mm(rd, rs1, rs2, 2);
}

void test_mat_mma_builtin(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_mma(rd, rs1, rs2, 2);
}

void test_mat_conv2d_builtin(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_conv2d(rd, rs1, rs2, 2);
}

void test_mat_conv2da_builtin(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_conv2da(rd, rs1, rs2, 2);
}

long test_base_tcsr_read_builtin(void) {
  return __builtin_riscv_tx_base_tcsr_read(1, 3);
}

void test_base_tcsr_write_builtin(long v) {
  __builtin_riscv_tx_base_tcsr_write(v, 1, 3);
}

void test_macro_convert_s32_f32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_s32_f32(rd, rs1, 2);
}

void test_macro_sfu_exp_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_exp(rd, rs1, 2);
}

void test_macro_reduce_add_s32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_reduce_add_s32(rd, rs1, 2);
}

long test_macro_tcsr_read_builtin(void) {
  return __builtin_riscv_tx_macro_tcsr_read(5);
}

void test_macro_tcsr_write_builtin(long v) {
  __builtin_riscv_tx_macro_tcsr_write(v, 5);
}

void test_macro_sb_produce_builtin(long v) {
  __builtin_riscv_tx_macro_sb_produce(v);
}

long test_macro_sb_bar_builtin(void) {
  return __builtin_riscv_tx_macro_sb_bar();
}

void test_macro_convert_fp32_s32_round_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_s32_round(rd, rs1, 2);
}

void test_macro_convert_fp32_s32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_s32(rd, rs1, 2);
}

void test_macro_convert_fp32_e4m3_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_e4m3(rd, rs1, 2);
}

void test_macro_convert_fp32_fp16_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_fp16(rd, rs1, 2);
}

void test_macro_convert_fp32_bf16_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_bf16(rd, rs1, 2);
}

void test_macro_sfu_log2_fp32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_log2_fp32(rd, rs1, 2);
}

void test_macro_sfu_exp2_fp32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_exp2_fp32(rd, rs1, 2);
}

void test_macro_sfu_sin_fp32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_sin_fp32(rd, rs1, 2);
}

void test_macro_sfu_cos_fp32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_cos_fp32(rd, rs1, 2);
}

void test_macro_sfu_rsqrt_fp32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_rsqrt_fp32(rd, rs1, 2);
}

void test_macro_sfu_recip_fp32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_recip_fp32(rd, rs1, 2);
}

void test_macro_reduce_max_fp32_builtin(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_reduce_max_fp32(rd, rs1, rs2, 2, 1);
}

void test_macro_reduce_min_fp32_builtin(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_reduce_min_fp32(rd, rs1, rs2, 2, 1);
}

void test_macro_reduce_add_fp32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_reduce_add_fp32(rd, rs1, 2);
}

void test_macro_reduce_max_s32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_reduce_max_s32(rd, rs1, 2, 1);
}

void test_macro_reduce_min_s32_builtin(long rd, long rs1) {
  __builtin_riscv_tx_macro_reduce_min_s32(rd, rs1, 2, 1);
}

long test_vec_tcsr_read_builtin(void) {
  return __builtin_riscv_tx_vec_tcsr_read(4);
}

void test_vec_tcsr_write_builtin(long v) {
  __builtin_riscv_tx_vec_tcsr_write(v, 4);
}

void test_mat_sb_produce_builtin(long v) {
  __builtin_riscv_tx_mat_sb_produce(v);
}

long test_mat_sb_bar_builtin(void) {
  return __builtin_riscv_tx_mat_sb_bar();
}

void test_vec_sb_produce_builtin(long v) {
  __builtin_riscv_tx_vec_sb_produce(v);
}

long test_vec_sb_bar_builtin(void) {
  return __builtin_riscv_tx_vec_sb_bar();
}

void test_lsu_sb_produce_builtin(long v) {
  __builtin_riscv_tx_lsu_sb_produce(v);
}

long test_lsu_sb_bar_builtin(void) {
  return __builtin_riscv_tx_lsu_sb_bar();
}

long test_sb_get_any_builtin(void) {
  return __builtin_riscv_tx_sb_get_any();
}

void test_vec_v_load_builtin(long rs2) {
  __builtin_riscv_tx_vec_v_load(rs2, 1, 2);
}

void test_vec_s_load_builtin(long rs2) {
  __builtin_riscv_tx_vec_s_load(rs2, 1, 2);
}

void test_vec_const_load_builtin(long rs2) {
  __builtin_riscv_tx_vec_const_load(rs2, 1);
}

void test_vec_v_store_builtin(long rs2) {
  __builtin_riscv_tx_vec_v_store(rs2, 3, 2);
}

void test_vec_s_store_builtin(long rs2) {
  __builtin_riscv_tx_vec_s_store(rs2, 3, 2);
}

void test_vec_vv_int32_bitwise_builtin(void) {
  __builtin_riscv_tx_vec_vv_int32_bitwise(1, 3, 7, 5);
}

void test_vec_vv_int32_alu_builtins(void) {
  __builtin_riscv_tx_vec_vv_int32_rol(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_sll(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_sra(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_srl(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_add(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_sub(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_mulu24(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_mul24(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_maxu(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_minu(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_max(1, 3, 7);
  __builtin_riscv_tx_vec_vv_int32_min(1, 3, 7);
}

void test_vec_vv_fp32_builtins(void) {
  __builtin_riscv_tx_vec_vv_fp32_add(1, 3, 7, 0);
  __builtin_riscv_tx_vec_vv_fp32_sub(1, 3, 7, 0);
  __builtin_riscv_tx_vec_vv_fp32_mul(1, 3, 7, 0);
  __builtin_riscv_tx_vec_vv_fp32_max(1, 3, 7, 0);
  __builtin_riscv_tx_vec_vv_fp32_min(1, 3, 7, 0);
}

void test_vec_vv_predicate_builtins(void) {
  __builtin_riscv_tx_vec_vv_mux2(1, 3, 7, 2);
  __builtin_riscv_tx_vec_vv_int32_cmp(3, 7, 2, 4);
  __builtin_riscv_tx_vec_vv_fp32_cmp(3, 7, 2, 4);
}

void test_vec_v_unary_builtins(void) {
  __builtin_riscv_tx_vec_v_int32_sll(1, 3, 4);
  __builtin_riscv_tx_vec_v_int32_sra(1, 3, 4);
  __builtin_riscv_tx_vec_v_int32_srl(1, 3, 4);
  __builtin_riscv_tx_vec_v_fp32_sfu(1, 3, 2);
}

void test_vec_cvt_builtins(void) {
  __builtin_riscv_tx_vec_cvt_fp32_int32(1, 3, 0);
  __builtin_riscv_tx_vec_cvt_s32_fp32(1, 3, 0);
  __builtin_riscv_tx_vec_cvt_fp32_fp16(1, 3, 0);
  __builtin_riscv_tx_vec_cvt_fp32_bf16(1, 3, 0);
  __builtin_riscv_tx_vec_cvt_fp16_fp32(1, 3, 0);
  __builtin_riscv_tx_vec_cvt_bf16_fp32(1, 3, 0);
  __builtin_riscv_tx_vec_cvt_mxfp8_bf16(1, 3, 0);
  __builtin_riscv_tx_vec_cvt_mxfp4_bf16(1, 3, 0);
}

void test_vec_s_builtins(void) {
  __builtin_riscv_tx_vec_s_int32_rol(2, 1, 3);
  __builtin_riscv_tx_vec_s_int32_sll(2, 1, 3);
  __builtin_riscv_tx_vec_s_int32_sra(2, 1, 3);
  __builtin_riscv_tx_vec_s_int32_srl(2, 1, 3);
  __builtin_riscv_tx_vec_s_int32_add(2, 1, 3);
  __builtin_riscv_tx_vec_s_int32_sub(2, 1, 3);
  __builtin_riscv_tx_vec_s_int32_mul(2, 1, 3);
  __builtin_riscv_tx_vec_s_iter_getdim(2, 1, 3);
  __builtin_riscv_tx_vec_s_iter_gets(2, 1, 3, 1);
  __builtin_riscv_tx_vec_s_iter_getv(2, 1, 3, 1);
  __builtin_riscv_tx_vec_s_bitwise(2, 1, 3, 3);
  __builtin_riscv_tx_vec_s_bitwise_beq(8, 1, 3);
  __builtin_riscv_tx_vec_s_bitwise_bne(8, 1, 3);
  __builtin_riscv_tx_vec_s_int32_blt(8, 1, 3);
  __builtin_riscv_tx_vec_s_int32_bge(8, 1, 3);
}

void test_vec_lsu_builtins(long rd) {
  __builtin_riscv_tx_vec_lsu_u2v(rd, 1, 2);
  __builtin_riscv_tx_vec_lsu_sm2v(rd, 1, 2);
  __builtin_riscv_tx_vec_lsu_u2v_broadcast32(rd, 1, 2);
  __builtin_riscv_tx_vec_lsu_sm2v_broadcast32(rd, 1, 2);
  __builtin_riscv_tx_vec_lsu_s2v_broadcast32(1, 2, 3);
  __builtin_riscv_tx_vec_lsu_g2v_broadcast32(rd, 1, 2);
  __builtin_riscv_tx_vec_lsu_u2s(rd, 2, 1);
  __builtin_riscv_tx_vec_lsu_sm2s(rd, 2, 1);
  __builtin_riscv_tx_vec_lsu_g2s(rd, 2);
  __builtin_riscv_tx_vec_lsu_u2v_mask(rd, 1, 3, 2);
  __builtin_riscv_tx_vec_lsu_sm2v_mask(rd, 1, 3, 2);
  __builtin_riscv_tx_vec_lsu_u2v_mask_broadcast32(rd, 1, 3, 2);
  __builtin_riscv_tx_vec_lsu_sm2v_mask_broadcast32(rd, 1, 3, 2);
  __builtin_riscv_tx_vec_lsu_s2v_mask_broadcast32(1, 3, 2, 1);
  __builtin_riscv_tx_vec_lsu_g2v_mask_broadcast32(rd, 1, 3, 2);
  __builtin_riscv_tx_vec_lsu_v2u(rd, 3, 2);
  __builtin_riscv_tx_vec_lsu_v2sm(rd, 3, 2);
  __builtin_riscv_tx_vec_lsu_s2u(rd, 3, 2);
  __builtin_riscv_tx_vec_lsu_s2sm(rd, 3, 2);
  __builtin_riscv_tx_vec_lsu_v2u_mask(rd, 3, 2, 1);
  __builtin_riscv_tx_vec_lsu_v2sm_mask(rd, 3, 2, 1);
}

void test_vec_sparse_dyn_shuffle_builtins(long rd) {
  __builtin_riscv_tx_vec_sm2v_sparse(rd, 1, 3, 7, 2);
  __builtin_riscv_tx_vec_v2sm_sparse(rd, 3, 7, 2);
  __builtin_riscv_tx_vec_v2sm_sparse_int32_add(rd, 1, 3, 7, 2);
  __builtin_riscv_tx_vec_v2sm_sparse_int32_and(rd, 1, 3, 7, 2);
  __builtin_riscv_tx_vec_v2sm_sparse_int32_or(rd, 1, 3, 7, 2);
  __builtin_riscv_tx_vec_v2sm_sparse_int32_xor(rd, 1, 3, 7, 2);
  __builtin_riscv_tx_vec_sm2v_dyn(1, 3, 7, 2, 1);
  __builtin_riscv_tx_vec_v2sm_dyn(3, 7, 2, 1);
  __builtin_riscv_tx_vec_v2sm_dyn_int32_add(1, 3, 7, 2, 1);
  __builtin_riscv_tx_vec_v2sm_dyn_int32_and(1, 3, 7, 2, 1);
  __builtin_riscv_tx_vec_v2sm_dyn_int32_or(1, 3, 7, 2, 1);
  __builtin_riscv_tx_vec_v2sm_dyn_int32_xor(1, 3, 7, 2, 1);
  __builtin_riscv_tx_vec_shuffle_xor(rd, 1, 3);
  __builtin_riscv_tx_vec_shuffle_xor_dyn(1, 3, 2);
  __builtin_riscv_tx_vec_shuffle_ror(rd, 1, 3);
  __builtin_riscv_tx_vec_shuffle_ror_dyn(1, 3, 2);
  __builtin_riscv_tx_vec_shuffle_broadcast(rd, 1, 3);
  __builtin_riscv_tx_vec_shuffle_broadcast_dyn(1, 3, 2);
  __builtin_riscv_tx_vec_shuffle_compress(rd, 1, 3);
  __builtin_riscv_tx_vec_shuffle_compress_dyn(1, 3, 2);
}

// CHECK-LABEL: @test_lsu_tcsr_read_builtin
// CHECK: call i64 asm sideeffect "tx.lsu.tcsr.read $0, $1", "=r,i"(i32 3)
//
// CHECK-LABEL: @test_lsu_tcsr_write_builtin
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 3)
//
// CHECK-LABEL: @test_lsu_tcsr_extended_fields_builtin
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 14)
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 18)
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 19)
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 20)
//
// CHECK-LABEL: @test_lsu_dma_d2u_builtin
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
//
// CHECK-LABEL: @test_lsu_dma_u2u_builtin
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
//
// CHECK-LABEL: @test_lsu_dma_u2d_builtin
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
//
// CHECK-LABEL: @test_lsu_dma_d2d_builtin
// CHECK: call void asm sideeffect "tx.lsu.dma.d2d $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
//
// CHECK-LABEL: @test_lsu_dma_table_modes_builtin
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u.g $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u.s $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u.sg $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d.g $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d.s $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d.sg $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u.g $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u.s $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u.sg $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.d2d.g $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.d2d.s $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
// CHECK: call void asm sideeffect "tx.lsu.dma.d2d.sg $0, $1, $2", "r,r,i,~{memory}"(i64 %{{.*}}, i64 %{{.*}}, i32 1)
//
// CHECK-LABEL: @test_mat_tcsr_read_builtin
// CHECK: call i64 asm sideeffect "tx.mat.tcsr.read $0, $1", "=r,i"(i32 2)
//
// CHECK-LABEL: @test_mat_tcsr_write_builtin
// CHECK: call void asm sideeffect "tx.mat.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_mat_setptr_builtin
// CHECK: call void asm sideeffect "tx.mat.setptr $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_mat_mm_builtin
// CHECK: call void asm sideeffect "tx.mat.mm $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_mat_mma_builtin
// CHECK: call void asm sideeffect "tx.mat.mma $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_mat_conv2d_builtin
// CHECK: call void asm sideeffect "tx.mat.conv2d $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_mat_conv2da_builtin
// CHECK: call void asm sideeffect "tx.mat.conv2da $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_base_tcsr_read_builtin
// CHECK: call i64 asm sideeffect "tx.base.tcsr.read $0, $1, $2", "=r,i,i"(i32 1, i32 3)
//
// CHECK-LABEL: @test_base_tcsr_write_builtin
// CHECK: call void asm sideeffect "tx.base.tcsr.write $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 3)
//
//
//
//
// CHECK-LABEL: @test_macro_convert_s32_f32_builtin
// CHECK: call void asm sideeffect "tx.macro.convert.s32.f32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_sfu_exp_builtin
// CHECK: call void asm sideeffect "tx.macro.sfu.exp $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_reduce_add_s32_builtin
// CHECK: call void asm sideeffect "tx.macro.reduce.add.s32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_tcsr_read_builtin
// CHECK: call i64 asm sideeffect "tx.macro.tcsr.read $0, $1", "=r,i"(i32 5)
//
// CHECK-LABEL: @test_macro_tcsr_write_builtin
// CHECK: call void asm sideeffect "tx.macro.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 5)
//
// CHECK-LABEL: @test_macro_sb_produce_builtin
// CHECK: call void asm sideeffect "tx.macro.sb.produce $0", "r"(i64 %{{.*}})
//
// CHECK-LABEL: @test_macro_sb_bar_builtin
// CHECK: call i64 asm sideeffect "tx.macro.sb.bar $0", "=r"()
//
//
//
//
//
//
//
//
//
// CHECK-LABEL: @test_macro_convert_fp32_s32_round_builtin
// CHECK: call void asm sideeffect "tx.macro.convert.fp32.s32.round $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_convert_fp32_s32_builtin
// CHECK: call void asm sideeffect "tx.macro.convert.fp32.s32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_convert_fp32_e4m3_builtin
// CHECK: call void asm sideeffect "tx.macro.convert.fp32.e4m3 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_convert_fp32_fp16_builtin
// CHECK: call void asm sideeffect "tx.macro.convert.fp32.fp16 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_convert_fp32_bf16_builtin
// CHECK: call void asm sideeffect "tx.macro.convert.fp32.bf16 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_sfu_log2_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.sfu.log2.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_sfu_exp2_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.sfu.exp2.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_sfu_sin_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.sfu.sin.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_sfu_cos_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.sfu.cos.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_sfu_rsqrt_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.sfu.rsqrt.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_sfu_recip_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.sfu.recip.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_reduce_max_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.reduce.max.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 2, i32 1)
//
// CHECK-LABEL: @test_macro_reduce_min_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.reduce.min.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 2, i32 1)
//
// CHECK-LABEL: @test_macro_reduce_add_fp32_builtin
// CHECK: call void asm sideeffect "tx.macro.reduce.add.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2)
//
// CHECK-LABEL: @test_macro_reduce_max_s32_builtin
// CHECK: call void asm sideeffect "tx.macro.reduce.max.s32 $0, $1, $2, $3", "r,r,i,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2, i32 1)
//
// CHECK-LABEL: @test_macro_reduce_min_s32_builtin
// CHECK: call void asm sideeffect "tx.macro.reduce.min.s32 $0, $1, $2, $3", "r,r,i,i"(i64 %{{.*}}, i64 %{{.*}}, i32 2, i32 1)
//
// CHECK-LABEL: @test_vec_tcsr_read_builtin
// CHECK: call i64 asm sideeffect "tx.vec.tcsr.read $0, $1", "=r,i"(i32 4)
//
// CHECK-LABEL: @test_vec_tcsr_write_builtin
// CHECK: call void asm sideeffect "tx.vec.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 4)
//
// CHECK-LABEL: @test_mat_sb_produce_builtin
// CHECK: call void asm sideeffect "tx.mat.sb.produce $0", "r"(i64 %{{.*}})
//
// CHECK-LABEL: @test_mat_sb_bar_builtin
// CHECK: call i64 asm sideeffect "tx.mat.sb.bar $0", "=r"()
//
// CHECK-LABEL: @test_vec_sb_produce_builtin
// CHECK: call void asm sideeffect "tx.vec.sb.produce $0", "r"(i64 %{{.*}})
//
// CHECK-LABEL: @test_vec_sb_bar_builtin
// CHECK: call i64 asm sideeffect "tx.vec.sb.bar $0", "=r"()
//
// CHECK-LABEL: @test_lsu_sb_produce_builtin
// CHECK: call void asm sideeffect "tx.lsu.sb.produce $0", "r"(i64 %{{.*}})
//
// CHECK-LABEL: @test_lsu_sb_bar_builtin
// CHECK: call i64 asm sideeffect "tx.lsu.sb.bar $0", "=r"()
//
// CHECK-LABEL: @test_sb_get_any_builtin
// CHECK: call i64 asm sideeffect "tx.sb.get.any $0", "=r"()
//
// CHECK-LABEL: @test_vec_v_load_builtin
// CHECK: call void asm sideeffect "tx.vec.v.load $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 2)
//
// CHECK-LABEL: @test_vec_s_load_builtin
// CHECK: call void asm sideeffect "tx.vec.s.load $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 2)
//
// CHECK-LABEL: @test_vec_const_load_builtin
// CHECK: call void asm sideeffect "tx.vec.const.load $0, $1", "r,i"(i64 %{{.*}}, i32 1)
//
// CHECK-LABEL: @test_vec_v_store_builtin
// CHECK: call void asm sideeffect "tx.vec.v.store $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 3, i32 2)
//
// CHECK-LABEL: @test_vec_s_store_builtin
// CHECK: call void asm sideeffect "tx.vec.s.store $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 3, i32 2)
//

// CHECK-LABEL: @test_vec_vv_int32_bitwise_builtin
// CHECK: call void asm sideeffect "tx.vec.vv.int32.bitwise $0, $1, $2, $3", "i,i,i,i"(i32 1, i32 3, i32 7, i32 5)
//
// CHECK-LABEL: @test_vec_vv_int32_alu_builtins
// CHECK: call void asm sideeffect "tx.vec.vv.int32.rol $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.sll $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.sra $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.srl $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.add $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.sub $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.mulu24 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.mul24 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.maxu $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.minu $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.max $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.min $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 7)
//
// CHECK-LABEL: @test_vec_vv_fp32_builtins
// CHECK: call void asm sideeffect "tx.vec.vv.fp32.add $0, $1, $2, $3", "i,i,i,i"(i32 1, i32 3, i32 7, i32 0)
// CHECK: call void asm sideeffect "tx.vec.vv.fp32.sub $0, $1, $2, $3", "i,i,i,i"(i32 1, i32 3, i32 7, i32 0)
// CHECK: call void asm sideeffect "tx.vec.vv.fp32.mul $0, $1, $2, $3", "i,i,i,i"(i32 1, i32 3, i32 7, i32 0)
// CHECK: call void asm sideeffect "tx.vec.vv.fp32.max $0, $1, $2, $3", "i,i,i,i"(i32 1, i32 3, i32 7, i32 0)
// CHECK: call void asm sideeffect "tx.vec.vv.fp32.min $0, $1, $2, $3", "i,i,i,i"(i32 1, i32 3, i32 7, i32 0)
//
// CHECK-LABEL: @test_vec_vv_predicate_builtins
// CHECK: call void asm sideeffect "tx.vec.vv.mux2 $0, $1, $2, $3", "i,i,i,i"(i32 1, i32 3, i32 7, i32 2)
// CHECK: call void asm sideeffect "tx.vec.vv.int32.cmp $0, $1, $2, $3", "i,i,i,i"(i32 3, i32 7, i32 2, i32 4)
// CHECK: call void asm sideeffect "tx.vec.vv.fp32.cmp $0, $1, $2, $3", "i,i,i,i"(i32 3, i32 7, i32 2, i32 4)
//
// CHECK-LABEL: @test_vec_v_unary_builtins
// CHECK: call void asm sideeffect "tx.vec.v.int32.sll $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 4)
// CHECK: call void asm sideeffect "tx.vec.v.int32.sra $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 4)
// CHECK: call void asm sideeffect "tx.vec.v.int32.srl $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 4)
// CHECK: call void asm sideeffect "tx.vec.v.float32.sfu $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 2)
//
// CHECK-LABEL: @test_vec_cvt_builtins
// CHECK: call void asm sideeffect "tx.vec.cvt.fp32.int32 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 0)
// CHECK: call void asm sideeffect "tx.vec.cvt.s32.fp32 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 0)
// CHECK: call void asm sideeffect "tx.vec.cvt.fp32.fp16 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 0)
// CHECK: call void asm sideeffect "tx.vec.cvt.fp32.bf16 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 0)
// CHECK: call void asm sideeffect "tx.vec.cvt.fp16.fp32 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 0)
// CHECK: call void asm sideeffect "tx.vec.cvt.bf16.fp32 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 0)
// CHECK: call void asm sideeffect "tx.vec.cvt.mxfp8.bf16 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 0)
// CHECK: call void asm sideeffect "tx.vec.cvt.mxfp4.bf16 $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 0)
//
// CHECK-LABEL: @test_vec_s_builtins
// CHECK: call void asm sideeffect "tx.vec.s.int32.rol $0, $1, $2", "i,i,i"(i32 2, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.int32.sll $0, $1, $2", "i,i,i"(i32 2, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.int32.sra $0, $1, $2", "i,i,i"(i32 2, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.int32.srl $0, $1, $2", "i,i,i"(i32 2, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.int32.add $0, $1, $2", "i,i,i"(i32 2, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.int32.sub $0, $1, $2", "i,i,i"(i32 2, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.int32.mul $0, $1, $2", "i,i,i"(i32 2, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.iter.getdim $0, $1, $2", "i,i,i"(i32 2, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.iter.gets $0, $1, $2, $3", "i,i,i,i"(i32 2, i32 1, i32 3, i32 1)
// CHECK: call void asm sideeffect "tx.vec.s.iter.getv $0, $1, $2, $3", "i,i,i,i"(i32 2, i32 1, i32 3, i32 1)
// CHECK: call void asm sideeffect "tx.vec.s.bitwise $0, $1, $2, $3", "i,i,i,i"(i32 2, i32 1, i32 3, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.bitwise.beq $0, $1, $2", "i,i,i"(i32 8, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.bitwise.bne $0, $1, $2", "i,i,i"(i32 8, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.int32.blt $0, $1, $2", "i,i,i"(i32 8, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.s.int32.bge $0, $1, $2", "i,i,i"(i32 8, i32 1, i32 3)
//
// CHECK-LABEL: @test_vec_lsu_builtins
// CHECK: call void asm sideeffect "tx.vec.lsu.u2v $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.sm2v $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.u2v.broadcast32 $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.sm2v.broadcast32 $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.s2v.broadcast32 $0, $1, $2", "i,i,i"(i32 1, i32 2, i32 3)
// CHECK: call void asm sideeffect "tx.vec.lsu.g2v.broadcast32 $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.u2s $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.lsu.sm2s $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.lsu.g2s $0, $1", "r,i"(i64 %{{.*}}, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.u2v.mask $0, $1, $2, $3", "r,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.sm2v.mask $0, $1, $2, $3", "r,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.u2v.mask.broadcast32 $0, $1, $2, $3", "r,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.sm2v.mask.broadcast32 $0, $1, $2, $3", "r,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.s2v.mask.broadcast32 $0, $1, $2, $3", "i,i,i,i"(i32 1, i32 3, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.lsu.g2v.mask.broadcast32 $0, $1, $2, $3", "r,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.v2u $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.v2sm $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.s2u $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.s2sm $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.lsu.v2u.mask $0, $1, $2, $3", "r,i,i,i"(i64 %{{.*}}, i32 3, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.lsu.v2sm.mask $0, $1, $2, $3", "r,i,i,i"(i64 %{{.*}}, i32 3, i32 2, i32 1)
//
// CHECK-LABEL: @test_vec_sparse_dyn_shuffle_builtins
// CHECK: call void asm sideeffect "tx.vec.sm2v.sparse $0, $1, $2, $3, $4", "r,i,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 7, i32 2)
// CHECK: call void asm sideeffect "tx.vec.v2sm.sparse $0, $1, $2, $3", "r,i,i,i"(i64 %{{.*}}, i32 3, i32 7, i32 2)
// CHECK: call void asm sideeffect "tx.vec.v2sm.sparse.int32.add $0, $1, $2, $3, $4", "r,i,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 7, i32 2)
// CHECK: call void asm sideeffect "tx.vec.v2sm.sparse.int32.and $0, $1, $2, $3, $4", "r,i,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 7, i32 2)
// CHECK: call void asm sideeffect "tx.vec.v2sm.sparse.int32.or $0, $1, $2, $3, $4", "r,i,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 7, i32 2)
// CHECK: call void asm sideeffect "tx.vec.v2sm.sparse.int32.xor $0, $1, $2, $3, $4", "r,i,i,i,i"(i64 %{{.*}}, i32 1, i32 3, i32 7, i32 2)
// CHECK: call void asm sideeffect "tx.vec.sm2v.dyn $0, $1, $2, $3, $4", "i,i,i,i,i"(i32 1, i32 3, i32 7, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.v2sm.dyn $0, $1, $2, $3", "i,i,i,i"(i32 3, i32 7, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.v2sm.dyn.int32.add $0, $1, $2, $3, $4", "i,i,i,i,i"(i32 1, i32 3, i32 7, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.v2sm.dyn.int32.and $0, $1, $2, $3, $4", "i,i,i,i,i"(i32 1, i32 3, i32 7, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.v2sm.dyn.int32.or $0, $1, $2, $3, $4", "i,i,i,i,i"(i32 1, i32 3, i32 7, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.v2sm.dyn.int32.xor $0, $1, $2, $3, $4", "i,i,i,i,i"(i32 1, i32 3, i32 7, i32 2, i32 1)
// CHECK: call void asm sideeffect "tx.vec.shuffle.xor $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.shuffle.xor.dyn $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.shuffle.ror $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.shuffle.ror.dyn $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.shuffle.broadcast $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.shuffle.broadcast.dyn $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 2)
// CHECK: call void asm sideeffect "tx.vec.shuffle.compress $0, $1, $2", "r,i,i"(i64 %{{.*}}, i32 1, i32 3)
// CHECK: call void asm sideeffect "tx.vec.shuffle.compress.dyn $0, $1, $2", "i,i,i"(i32 1, i32 3, i32 2)
