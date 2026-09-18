// RUN: %clang_cc1 -triple riscv64 -target-feature +txmacro -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple riscv64 -target-feature +txmacro -emit-obj -o %t %s

// CHECK-LABEL: @elemwise_vv_add_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.add.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_add_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_add_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_add_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.add.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_add_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_add_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_add_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.add.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_add_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_add_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_sub_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.sub.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_sub_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_sub_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_sub_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.sub.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_sub_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_sub_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_sub_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.sub.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_sub_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_sub_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_mul_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.mul.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_mul_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_mul_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_mul_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.mul.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_mul_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_mul_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_mul_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.mul.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_mul_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_mul_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_max_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.max.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_max_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_max_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_max_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.max.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_max_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_max_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_max_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.max.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_max_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_max_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_min_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.min.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_min_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_min_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_min_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.min.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_min_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_min_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_min_fp32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.min.fp32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_min_fp32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_min_fp32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_add_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.add.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_add_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_add_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_add_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.add.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_add_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_add_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_add_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.add.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_add_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_add_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_sub_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.sub.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_sub_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_sub_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_sub_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.sub.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_sub_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_sub_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_sub_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.sub.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_sub_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_sub_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_mul_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.mul.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_mul_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_mul_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_mul_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.mul.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_mul_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_mul_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_mul_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.mul.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_mul_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_mul_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_max_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.max.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_max_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_max_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_max_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.max.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_max_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_max_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_max_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.max.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_max_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_max_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_min_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.min.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_min_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_min_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_min_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.min.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_min_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_min_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_min_s32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.min.s32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_min_s32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_min_s32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_add_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.add.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_add_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_add_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_add_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.add.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_add_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_add_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_add_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.add.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_add_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_add_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_sub_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.sub.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_sub_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_sub_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_sub_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.sub.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_sub_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_sub_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_sub_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.sub.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_sub_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_sub_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_mul_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.mul.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_mul_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_mul_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_mul_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.mul.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_mul_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_mul_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_mul_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.mul.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_mul_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_mul_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_max_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.max.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_max_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_max_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_max_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.max.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_max_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_max_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_max_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.max.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_max_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_max_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_vv_min_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.vv.min.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_vv_min_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_vv_min_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_sv_min_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.sv.min.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_sv_min_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_sv_min_u32(dst, src0, src1, 7);
}

// CHECK-LABEL: @elemwise_iv_min_u32(
// CHECK: call void asm sideeffect "tx.macro.elemwise.iv.min.u32 $0, $1, $2, $3", "r,r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i32 7)
void elemwise_iv_min_u32(long dst, long src0, long src1) {
  __builtin_riscv_tx_macro_elemwise_iv_min_u32(dst, src0, src1, 7);
}

