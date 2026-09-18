//==- BuiltinsRISCV.td - RISC-V Builtin function database ---*- tablegen -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the RISC-V-specific builtin function database.
//
//===----------------------------------------------------------------------===//

include "clang/Basic/BuiltinsBase.td"

class RISCVBuiltin<string prototype, string features = ""> : TargetBuiltin {
  let Spellings = ["__builtin_riscv_" # NAME];
  let Prototype = prototype;
  let Features = features;
}

let Attributes = [NoThrow, Const] in {
//===----------------------------------------------------------------------===//
// Zbb extension.
//===----------------------------------------------------------------------===//
def orc_b_32 : RISCVBuiltin<"unsigned int(unsigned int)", "zbb">;
def orc_b_64 : RISCVBuiltin<"uint64_t(uint64_t)", "zbb,64bit">;
def clz_32 : RISCVBuiltin<"unsigned int(unsigned int)", "zbb|xtheadbb">;
def clz_64 : RISCVBuiltin<"unsigned int(uint64_t)", "zbb|xtheadbb,64bit">;
def ctz_32 : RISCVBuiltin<"unsigned int(unsigned int)", "zbb">;
def ctz_64 : RISCVBuiltin<"unsigned int(uint64_t)", "zbb,64bit">;

//===----------------------------------------------------------------------===//
// Zbc or Zbkc extension.
//===----------------------------------------------------------------------===//
def clmul_32 : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)", "zbc|zbkc">;
def clmul_64 : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)", "zbc|zbkc,64bit">;
def clmulh_32 : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)", "zbc|zbkc,32bit">;
def clmulh_64 : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)", "zbc|zbkc,64bit">;
def clmulr_32 : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)", "zbc,32bit">;
def clmulr_64 : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)", "zbc,64bit">;

//===----------------------------------------------------------------------===//
// Zbkx extension.
//===----------------------------------------------------------------------===//
let Features = "zbkx,32bit" in {
def xperm4_32 : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)">;
def xperm8_32 : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)">;
} // Features = "zbkx,32bit"

let Features = "zbkx,64bit" in {
def xperm4_64 : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)">;
def xperm8_64 : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)">;
} // Features = "zbkx,64bit"

//===----------------------------------------------------------------------===//
// Zbkb extension.
//===----------------------------------------------------------------------===//
def brev8_32 : RISCVBuiltin<"unsigned int(unsigned int)", "zbkb">;
def brev8_64 : RISCVBuiltin<"uint64_t(uint64_t)", "zbkb,64bit">;
def zip_32 : RISCVBuiltin<"unsigned int(unsigned int)", "zbkb,32bit">;
def unzip_32 : RISCVBuiltin<"unsigned int(unsigned int)", "zbkb,32bit">;

//===----------------------------------------------------------------------===//
// Zknd extension.
//===----------------------------------------------------------------------===//
let Features = "zknd,32bit" in {
def aes32dsi : RISCVBuiltin<"unsigned int(unsigned int, unsigned int, _Constant unsigned int)">;
def aes32dsmi : RISCVBuiltin<"unsigned int(unsigned int, unsigned int, _Constant unsigned int)">;
} // Features = "zknd,32bit"

let Features = "zknd,64bit" in {
def aes64ds : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)">;
def aes64dsm : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)">;
def aes64im : RISCVBuiltin<"uint64_t(uint64_t)">;
} // Features = "zknd,64bit"

//===----------------------------------------------------------------------===//
// Zknd & Zkne extension.
//===----------------------------------------------------------------------===//
let Features = "zknd|zkne,64bit" in {
def aes64ks1i : RISCVBuiltin<"uint64_t(uint64_t, _Constant unsigned int)">;
def aes64ks2 : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)">;
} // Features = "zknd|zkne,64bit"

//===----------------------------------------------------------------------===//
// Zkne extension.
//===----------------------------------------------------------------------===//
let Features = "zkne,32bit" in {
def aes32esi : RISCVBuiltin<"unsigned int(unsigned int, unsigned int, _Constant unsigned int)">;
def aes32esmi : RISCVBuiltin<"unsigned int(unsigned int, unsigned int, _Constant unsigned int)">;
} // Features = "zkne,32bit"

let Features = "zkne,64bit" in {
def aes64es : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)">;
def aes64esm : RISCVBuiltin<"uint64_t(uint64_t, uint64_t)">;
} // Features = "zkne,64bit"

//===----------------------------------------------------------------------===//
// Zknh extension.
//===----------------------------------------------------------------------===//
let Features = "zknh" in {
def sha256sig0 : RISCVBuiltin<"unsigned int(unsigned int)">;
def sha256sig1 : RISCVBuiltin<"unsigned int(unsigned int)">;
def sha256sum0 : RISCVBuiltin<"unsigned int(unsigned int)">;
def sha256sum1 : RISCVBuiltin<"unsigned int(unsigned int)">;
} // Features = "zknh"

let Features = "zknh,32bit" in {
def sha512sig0h : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)">;
def sha512sig0l : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)">;
def sha512sig1h : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)">;
def sha512sig1l : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)">;
def sha512sum0r : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)">;
def sha512sum1r : RISCVBuiltin<"unsigned int(unsigned int, unsigned int)">;
} // Features = "zknh,32bit"

let Features = "zknh,64bit" in {
def sha512sig0 : RISCVBuiltin<"uint64_t(uint64_t)">;
def sha512sig1 : RISCVBuiltin<"uint64_t(uint64_t)">;
def sha512sum0 : RISCVBuiltin<"uint64_t(uint64_t)">;
def sha512sum1 : RISCVBuiltin<"uint64_t(uint64_t)">;
} // Features = "zknh,64bit"

//===----------------------------------------------------------------------===//
// Zksed extension.
//===----------------------------------------------------------------------===//
let Features = "zksed" in {
def sm4ed : RISCVBuiltin<"unsigned int(unsigned int, unsigned int, _Constant unsigned int )">;
def sm4ks : RISCVBuiltin<"unsigned int(unsigned int, unsigned int, _Constant unsigned int)">;
} // Features = "zksed"

//===----------------------------------------------------------------------===//
// Zksh extension.
//===----------------------------------------------------------------------===//
let Features = "zksh" in {
def sm3p0 : RISCVBuiltin<"unsigned int(unsigned int)">;
def sm3p1 : RISCVBuiltin<"unsigned int(unsigned int)">;
} // Features = "zksh"

} // Attributes = [Const, NoThrow]

//===----------------------------------------------------------------------===//
// Zihintntl extension.
//===----------------------------------------------------------------------===//
let Features = "zihintntl", Attributes = [CustomTypeChecking] in {
def ntl_load : RISCVBuiltin<"void(...)">;
def ntl_store : RISCVBuiltin<"void(...)">;
} // Features = "zihintntl", Attributes = [CustomTypeChecking]

//===----------------------------------------------------------------------===//
// Zihintpause extension.
//===----------------------------------------------------------------------===//
let Features = "zihintpause", Attributes = [NoThrow] in
def pause : RISCVBuiltin<"void()">;

//===----------------------------------------------------------------------===//
// RVT LSU extension.
//===----------------------------------------------------------------------===//
let Features = "txlsu|ztxlsu", Attributes = [NoThrow] in {
def tx_lsu_tcsr_read : RISCVBuiltin<"long int(_Constant unsigned int)">;
def tx_lsu_tcsr_write : RISCVBuiltin<"void(long int, _Constant unsigned int)">;
def tx_lsu_dma_u2u : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_u2u_g : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_u2u_s : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_u2u_sg : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_u2d : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_u2d_g : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_u2d_s : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_u2d_sg : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_d2u : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_d2u_g : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_d2u_s : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_d2u_sg : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_d2d : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_d2d_g : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_d2d_s : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_lsu_dma_d2d_sg : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
} // Features = "txlsu|ztxlsu", Attributes = [NoThrow]

//===----------------------------------------------------------------------===//
// RVT MAT extension.
//===----------------------------------------------------------------------===//
let Features = "txmat", Attributes = [NoThrow] in {
def tx_mat_tcsr_read : RISCVBuiltin<"long int(_Constant unsigned int)">;
def tx_mat_tcsr_write : RISCVBuiltin<"void(long int, _Constant unsigned int)">;
// setptr takes two extension pointers and a group; its rd field is reserved.
def tx_mat_setptr : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_mat_mm : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_mat_mma : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_mat_conv2d : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_mat_conv2da : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
} // Features = "txmat", Attributes = [NoThrow]


//===----------------------------------------------------------------------===//
// NPUURAM extension.
//===----------------------------------------------------------------------===//

let Features = "txbase", Attributes = [NoThrow, Const] in {
  def is_uram : RISCVBuiltin<"int(long int)">;
}


//===----------------------------------------------------------------------===//
// RVT BASE extension.
//===----------------------------------------------------------------------===//
let Features = "txbase", Attributes = [NoThrow] in {
def tx_base_tcsr_read : RISCVBuiltin<"long int(_Constant unsigned int, _Constant unsigned int)">;
def tx_base_tcsr_write : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_mat_sb_produce : RISCVBuiltin<"void(long int)">;
def tx_mat_sb_bar : RISCVBuiltin<"long int()">;
def tx_vec_sb_produce : RISCVBuiltin<"void(long int)">;
def tx_vec_sb_bar : RISCVBuiltin<"long int()">;
def tx_lsu_sb_produce : RISCVBuiltin<"void(long int)">;
def tx_lsu_sb_bar : RISCVBuiltin<"long int()">;
def tx_macro_sb_produce : RISCVBuiltin<"void(long int)">;
def tx_macro_sb_bar : RISCVBuiltin<"long int()">;
def tx_sb_get_any : RISCVBuiltin<"long int()">;
} // Features = "txbase", Attributes = [NoThrow]

//===----------------------------------------------------------------------===//
// RVT VEC extension.
//===----------------------------------------------------------------------===//
let Features = "txvec", Attributes = [NoThrow] in {
def tx_vec_tcsr_read : RISCVBuiltin<"long int(_Constant unsigned int)">;
def tx_vec_tcsr_write : RISCVBuiltin<"void(long int, _Constant unsigned int)">;
def tx_vec_lsu_u2v : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_sm2v : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_u2v_broadcast32 : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_sm2v_broadcast32 : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_s2v_broadcast32 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_g2v_broadcast32 : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_u2s : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_sm2s : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_g2s : RISCVBuiltin<"void(long int, _Constant unsigned int)">;
def tx_vec_lsu_u2v_mask : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_sm2v_mask : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_u2v_mask_broadcast32 : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_sm2v_mask_broadcast32 : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_s2v_mask_broadcast32 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_g2v_mask_broadcast32 : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_v2u : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_v2sm : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_s2u : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_s2sm : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_v2u_mask : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_lsu_v2sm_mask : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_sm2v_sparse : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_sparse : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_sparse_int32_add : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_sparse_int32_and : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_sparse_int32_or : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_sparse_int32_xor : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_sm2v_dyn : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_dyn : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_dyn_int32_add : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_dyn_int32_and : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_dyn_int32_or : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v2sm_dyn_int32_xor : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_shuffle_xor : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_shuffle_xor_dyn : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_shuffle_ror : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_shuffle_ror_dyn : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_shuffle_broadcast : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_shuffle_broadcast_dyn : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_shuffle_compress : RISCVBuiltin<"void(long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_shuffle_compress_dyn : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_bitwise : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_rol : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_sll : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_sra : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_srl : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_add : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_sub : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_mulu24 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_mul24 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_maxu : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_minu : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_max : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_min : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_fp32_add : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_fp32_sub : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_fp32_mul : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_fp32_max : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_fp32_min : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_mux2 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_int32_cmp : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_vv_fp32_cmp : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v_int32_sll : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v_int32_sra : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v_int32_srl : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_v_fp32_sfu : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_cvt_fp32_int32 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_cvt_s32_fp32 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_cvt_fp32_fp16 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_cvt_fp32_bf16 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_cvt_fp16_fp32 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_cvt_bf16_fp32 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_cvt_mxfp8_bf16 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_cvt_mxfp4_bf16 : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_rol : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_sll : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_sra : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_srl : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_add : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_sub : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_mul : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_iter_getdim : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_iter_gets : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_iter_getv : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_bitwise : RISCVBuiltin<"void(_Constant unsigned int, _Constant unsigned int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_bitwise_beq : RISCVBuiltin<"void(_Constant int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_bitwise_bne : RISCVBuiltin<"void(_Constant int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_blt : RISCVBuiltin<"void(_Constant int, _Constant unsigned int, _Constant unsigned int)">;
def tx_vec_s_int32_bge : RISCVBuiltin<"void(_Constant int, _Constant unsigned int, _Constant unsigned int)">;
} // Features = "txvec", Attributes = [NoThrow]

//===----------------------------------------------------------------------===//
// RVT MACRO extension.
//===----------------------------------------------------------------------===//
let Features = "txmacro", Attributes = [NoThrow] in {
def tx_macro_shuffle_compress : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_log2 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_exp2 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_sin : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_cos : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_rsqrt : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_recip : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_tcsr_read : RISCVBuiltin<"long int(_Constant unsigned int)">;
def tx_macro_tcsr_write : RISCVBuiltin<"void(long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_add_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_add_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_add_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_sub_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_sub_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_sub_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_mul_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_mul_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_mul_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_max_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_max_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_max_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_min_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_min_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_min_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_add_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_add_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_add_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_sub_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_sub_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_sub_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_mul_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_mul_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_mul_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_max_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_max_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_max_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_min_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_min_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_min_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_add_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_add_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_add_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_sub_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_sub_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_sub_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_mul_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_mul_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_mul_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_max_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_max_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_max_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_vv_min_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_sv_min_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;
def tx_macro_elemwise_iv_min_u32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int)">;

def tx_macro_convert_s32_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_s16_s32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_bf16_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_s32_s16 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_mxfp8_bf16 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_s8_s16 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_s16_s8 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_bf16_s8 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_bf16_u8 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_fp8_s4 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_fp8_u4 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_s32_f32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_fp32_s32_round : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_fp32_s32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_fp32_fp8 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_fp32_fp16 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_convert_fp32_bf16 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;

def tx_macro_sfu_exp : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_log2_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_exp2_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_sin_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_cos_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_rsqrt_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_sfu_recip_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;

def tx_macro_reduce_add_s32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_reduce_max_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_macro_reduce_min_fp32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_macro_reduce_add_fp32 : RISCVBuiltin<"void(long int, long int, _Constant unsigned int)">;
def tx_macro_reduce_max_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int, _Constant unsigned int)">;
def tx_macro_reduce_min_s32 : RISCVBuiltin<"void(long int, long int, long int, _Constant unsigned int, _Constant unsigned int)">;
} // Features = "txmacro", Attributes = [NoThrow]
// P extension.
//===----------------------------------------------------------------------===//
let Features = "experimental-p", Attributes = [NoThrow, Const] in {
// Packed Averaging Addition and Subtraction (32-bit)
def paadd_i8x4 : RISCVBuiltin<"_Vector<4, signed char>(_Vector<4, signed char>, _Vector<4, signed char>)">;
def paadd_i16x2 : RISCVBuiltin<"_Vector<2, short>(_Vector<2, short>, _Vector<2, short>)">;
def paaddu_u8x4 : RISCVBuiltin<"_Vector<4, unsigned char>(_Vector<4, unsigned char>, _Vector<4, unsigned char>)">;
def paaddu_u16x2 : RISCVBuiltin<"_Vector<2, unsigned short>(_Vector<2, unsigned short>, _Vector<2, unsigned short>)">;
def pasub_i8x4 : RISCVBuiltin<"_Vector<4, signed char>(_Vector<4, signed char>, _Vector<4, signed char>)">;
def pasub_i16x2 : RISCVBuiltin<"_Vector<2, short>(_Vector<2, short>, _Vector<2, short>)">;
def pasubu_u8x4 : RISCVBuiltin<"_Vector<4, unsigned char>(_Vector<4, unsigned char>, _Vector<4, unsigned char>)">;
def pasubu_u16x2 : RISCVBuiltin<"_Vector<2, unsigned short>(_Vector<2, unsigned short>, _Vector<2, unsigned short>)">;

// Packed Averaging Addition and Subtraction (64-bit)
def paadd_i8x8 : RISCVBuiltin<"_Vector<8, signed char>(_Vector<8, signed char>, _Vector<8, signed char>)">;
def paadd_i16x4 : RISCVBuiltin<"_Vector<4, short>(_Vector<4, short>, _Vector<4, short>)">;
def paadd_i32x2 : RISCVBuiltin<"_Vector<2, int>(_Vector<2, int>, _Vector<2, int>)">;
def paaddu_u8x8 : RISCVBuiltin<"_Vector<8, unsigned char>(_Vector<8, unsigned char>, _Vector<8, unsigned char>)">;
def paaddu_u16x4 : RISCVBuiltin<"_Vector<4, unsigned short>(_Vector<4, unsigned short>, _Vector<4, unsigned short>)">;
def paaddu_u32x2 : RISCVBuiltin<"_Vector<2, unsigned int>(_Vector<2, unsigned int>, _Vector<2, unsigned int>)">;
def pasub_i8x8 : RISCVBuiltin<"_Vector<8, signed char>(_Vector<8, signed char>, _Vector<8, signed char>)">;
def pasub_i16x4 : RISCVBuiltin<"_Vector<4, short>(_Vector<4, short>, _Vector<4, short>)">;
def pasub_i32x2 : RISCVBuiltin<"_Vector<2, int>(_Vector<2, int>, _Vector<2, int>)">;
def pasubu_u8x8 : RISCVBuiltin<"_Vector<8, unsigned char>(_Vector<8, unsigned char>, _Vector<8, unsigned char>)">;
def pasubu_u16x4 : RISCVBuiltin<"_Vector<4, unsigned short>(_Vector<4, unsigned short>, _Vector<4, unsigned short>)">;
def pasubu_u32x2 : RISCVBuiltin<"_Vector<2, unsigned int>(_Vector<2, unsigned int>, _Vector<2, unsigned int>)">;

// Packed Exchanged Addition and Subtraction (32-bit)
def pas_x_i16x2 : RISCVBuiltin<"_Vector<2, short>(_Vector<2, short>, _Vector<2, short>)">;
def psa_x_i16x2 : RISCVBuiltin<"_Vector<2, short>(_Vector<2, short>, _Vector<2, short>)">;
def psas_x_i16x2 : RISCVBuiltin<"_Vector<2, short>(_Vector<2, short>, _Vector<2, short>)">;
def pssa_x_i16x2 : RISCVBuiltin<"_Vector<2, short>(_Vector<2, short>, _Vector<2, short>)">;
def paas_x_i16x2 : RISCVBuiltin<"_Vector<2, short>(_Vector<2, short>, _Vector<2, short>)">;
def pasa_x_i16x2 : RISCVBuiltin<"_Vector<2, short>(_Vector<2, short>, _Vector<2, short>)">;

// Packed Exchanged Addition and Subtraction (64-bit)
def pas_x_i16x4 : RISCVBuiltin<"_Vector<4, short>(_Vector<4, short>, _Vector<4, short>)">;
def pas_x_i32x2 : RISCVBuiltin<"_Vector<2, int>(_Vector<2, int>, _Vector<2, int>)">;
def psa_x_i16x4 : RISCVBuiltin<"_Vector<4, short>(_Vector<4, short>, _Vector<4, short>)">;
def psa_x_i32x2 : RISCVBuiltin<"_Vector<2, int>(_Vector<2, int>, _Vector<2, int>)">;
def psas_x_i16x4 : RISCVBuiltin<"_Vector<4, short>(_Vector<4, short>, _Vector<4, short>)">;
def psas_x_i32x2 : RISCVBuiltin<"_Vector<2, int>(_Vector<2, int>, _Vector<2, int>)">;
def pssa_x_i16x4 : RISCVBuiltin<"_Vector<4, short>(_Vector<4, short>, _Vector<4, short>)">;
def pssa_x_i32x2 : RISCVBuiltin<"_Vector<2, int>(_Vector<2, int>, _Vector<2, int>)">;
def paas_x_i16x4 : RISCVBuiltin<"_Vector<4, short>(_Vector<4, short>, _Vector<4, short>)">;
def paas_x_i32x2 : RISCVBuiltin<"_Vector<2, int>(_Vector<2, int>, _Vector<2, int>)">;
def pasa_x_i16x4 : RISCVBuiltin<"_Vector<4, short>(_Vector<4, short>, _Vector<4, short>)">;
def pasa_x_i32x2 : RISCVBuiltin<"_Vector<2, int>(_Vector<2, int>, _Vector<2, int>)">;

// Packed Absolute Value and Absolute Difference (32-bit)
def pabd_i8x4 : RISCVBuiltin<"_Vector<4, unsigned char>(_Vector<4, signed char>, _Vector<4, signed char>)">;
def pabd_i16x2 : RISCVBuiltin<"_Vector<2, unsigned short>(_Vector<2, short>, _Vector<2, short>)">;
def pabdu_u8x4 : RISCVBuiltin<"_Vector<4, unsigned char>(_Vector<4, unsigned char>, _Vector<4, unsigned char>)">;
def pabdu_u16x2 : RISCVBuiltin<"_Vector<2, unsigned short>(_Vector<2, unsigned short>, _Vector<2, unsigned short>)">;

// Packed Absolute Value and Absolute Difference (64-bit)
def pabd_i8x8 : RISCVBuiltin<"_Vector<8, unsigned char>(_Vector<8, signed char>, _Vector<8, signed char>)">;
def pabd_i16x4 : RISCVBuiltin<"_Vector<4, unsigned short>(_Vector<4, short>, _Vector<4, short>)">;
def pabdu_u8x8 : RISCVBuiltin<"_Vector<8, unsigned char>(_Vector<8, unsigned char>, _Vector<8, unsigned char>)">;
def pabdu_u16x4 : RISCVBuiltin<"_Vector<4, unsigned short>(_Vector<4, unsigned short>, _Vector<4, unsigned short>)">;

} // Features = "experimental-p"

//===----------------------------------------------------------------------===//
// XCV extensions.
//===----------------------------------------------------------------------===//
include "clang/Basic/BuiltinsRISCVXCV.td"

//===----------------------------------------------------------------------===//
// XAndes extensions.
//===----------------------------------------------------------------------===//
include "clang/Basic/BuiltinsRISCVXAndes.td"

//===----------------------------------------------------------------------===//
// MIPS extensions.
//===----------------------------------------------------------------------===//
include "clang/Basic/BuiltinsRISCVXMIPS.td"
