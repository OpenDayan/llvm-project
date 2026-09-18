// RUN: %clang_cc1 -triple riscv64 -target-feature +txmacro -target-feature +txbase -target-feature +txmat -target-feature +txvec -target-feature +txlsu -internal-isystem %S/../../../lib/Headers -emit-obj -o %t %s
#include <rvt_intrinsics.h>

void elemwise(long dst, long src0, long src1) {
  __tx_macro_elemwise_vv_add_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_add_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_add_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_sub_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_sub_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_sub_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_mul_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_mul_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_mul_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_max_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_max_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_max_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_min_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_min_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_min_fp32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_add_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_add_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_add_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_sub_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_sub_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_sub_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_mul_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_mul_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_mul_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_max_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_max_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_max_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_min_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_min_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_min_s32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_add_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_add_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_add_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_sub_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_sub_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_sub_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_mul_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_mul_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_mul_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_max_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_max_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_max_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_vv_min_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_sv_min_u32<7>(dst, src0, src1);
  __tx_macro_elemwise_iv_min_u32<7>(dst, src0, src1);
}
