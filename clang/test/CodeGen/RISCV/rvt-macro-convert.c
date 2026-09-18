// RUN: %clang_cc1 -triple riscv64 -target-feature +txmacro -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple riscv64 -target-feature +txmacro -emit-obj -o %t %s

// CHECK-LABEL: @convert_fp32_s32(
// CHECK: call void asm sideeffect "tx.macro.convert.fp32.s32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_fp32_s32(long dst, long src) {
  __builtin_riscv_tx_macro_convert_fp32_s32(dst, src, 7);
}

// CHECK-LABEL: @convert_s32_fp32(
// CHECK: call void asm sideeffect "tx.macro.convert.s32.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_s32_fp32(long dst, long src) {
  __builtin_riscv_tx_macro_convert_s32_fp32(dst, src, 7);
}

// CHECK-LABEL: @convert_s16_s32(
// CHECK: call void asm sideeffect "tx.macro.convert.s16.s32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_s16_s32(long dst, long src) {
  __builtin_riscv_tx_macro_convert_s16_s32(dst, src, 7);
}

// CHECK-LABEL: @convert_bf16_fp32(
// CHECK: call void asm sideeffect "tx.macro.convert.bf16.fp32 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_bf16_fp32(long dst, long src) {
  __builtin_riscv_tx_macro_convert_bf16_fp32(dst, src, 7);
}

// CHECK-LABEL: @convert_fp32_bf16(
// CHECK: call void asm sideeffect "tx.macro.convert.fp32.bf16 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_fp32_bf16(long dst, long src) {
  __builtin_riscv_tx_macro_convert_fp32_bf16(dst, src, 7);
}

// CHECK-LABEL: @convert_s32_s16(
// CHECK: call void asm sideeffect "tx.macro.convert.s32.s16 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_s32_s16(long dst, long src) {
  __builtin_riscv_tx_macro_convert_s32_s16(dst, src, 7);
}

// CHECK-LABEL: @convert_mxfp8_bf16(
// CHECK: call void asm sideeffect "tx.macro.convert.mxfp8.bf16 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_mxfp8_bf16(long dst, long src) {
  __builtin_riscv_tx_macro_convert_mxfp8_bf16(dst, src, 7);
}

// CHECK-LABEL: @convert_s8_s16(
// CHECK: call void asm sideeffect "tx.macro.convert.s8.s16 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_s8_s16(long dst, long src) {
  __builtin_riscv_tx_macro_convert_s8_s16(dst, src, 7);
}

// CHECK-LABEL: @convert_s16_s8(
// CHECK: call void asm sideeffect "tx.macro.convert.s16.s8 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_s16_s8(long dst, long src) {
  __builtin_riscv_tx_macro_convert_s16_s8(dst, src, 7);
}

// CHECK-LABEL: @convert_bf16_s8(
// CHECK: call void asm sideeffect "tx.macro.convert.bf16.s8 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_bf16_s8(long dst, long src) {
  __builtin_riscv_tx_macro_convert_bf16_s8(dst, src, 7);
}

// CHECK-LABEL: @convert_bf16_u8(
// CHECK: call void asm sideeffect "tx.macro.convert.bf16.u8 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_bf16_u8(long dst, long src) {
  __builtin_riscv_tx_macro_convert_bf16_u8(dst, src, 7);
}

// CHECK-LABEL: @convert_e4m3_s4(
// CHECK: call void asm sideeffect "tx.macro.convert.e4m3.s4 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_e4m3_s4(long dst, long src) {
  __builtin_riscv_tx_macro_convert_e4m3_s4(dst, src, 7);
}

// CHECK-LABEL: @convert_e4m3_u4(
// CHECK: call void asm sideeffect "tx.macro.convert.e4m3.u4 $0, $1, $2", "r,r,i"(i64 %{{.*}}, i64 %{{.*}}, i32 7)
void convert_e4m3_u4(long dst, long src) {
  __builtin_riscv_tx_macro_convert_e4m3_u4(dst, src, 7);
}

