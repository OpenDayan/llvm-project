// RUN: %clang_cc1 -triple riscv64 -target-feature +txmacro -target-feature +txbase -target-feature +txmat -target-feature +txvec -target-feature +txlsu -internal-isystem %S/../../../lib/Headers -emit-obj -o %t %s
#include <rvt_intrinsics.h>
void test(long d, long a, long b) {
  __tx_mat_setptr<7>(a, b);
  __tx_vec_s_iter_getdim<2, 3>();
  __tx_vec_s_bitwise<2, 1, 3, 6>();
  __tx_vec_s_bitwise_beq<-128, 1, 3>();
  __tx_vec_s_bitwise_bne<-1, 1, 3>();
  __tx_vec_s_int32_blt<-128, 1, 3>();
  __tx_vec_s_int32_bge<127, 1, 3>();
  __tx_macro_reduce_max_s32<7, 1>(d, a, b);
  __tx_macro_reduce_min_s32<7, 0>(d, a, b);
  __tx_macro_shuffle_compress<7>(d, a, b);
  __tx_macro_sfu_log2<7>(d, a);
  __tx_macro_sfu_exp2<7>(d, a);
  __tx_macro_sfu_sin<7>(d, a);
  __tx_macro_sfu_cos<7>(d, a);
  __tx_macro_sfu_rsqrt<7>(d, a);
  __tx_macro_sfu_recip<7>(d, a);
}
