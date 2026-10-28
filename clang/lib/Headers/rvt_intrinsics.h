// NOLINTNEXTLINE(llvm-header-guard): guard style follows clang/lib/Headers.
#ifndef __RVT_INTRINSICS_H__
#define __RVT_INTRINSICS_H__

// RVT intrinsic-style wrappers with builtin-only implementation.
// The public wrappers mirror the canonical RVT ISA mnemonics. URAM operands
// are ordinary pointers or integer addresses; the compiler checks the
// call-site address space.

#if defined(__cplusplus)

// LSU TCSR offsets within one of the eight 32-entry configuration groups.
// The offsets and field widths follow Table 3 of the 2026-07-27 RVT ISA.
enum __rvt_lsu_tcsr_offset : long {
  __RVT_LSU_TCSR_BLOCK = 0,
  __RVT_LSU_TCSR_DIM0 = 1,
  __RVT_LSU_TCSR_DIM1 = 2,
  __RVT_LSU_TCSR_DIM2 = 3,
  __RVT_LSU_TCSR_IN_BLOCK_STRIDE = 4,
  __RVT_LSU_TCSR_IN_DIM0_STRIDE = 5,
  __RVT_LSU_TCSR_IN_DIM1_STRIDE = 6,
  __RVT_LSU_TCSR_IN_DIM2_STRIDE = 7,
  __RVT_LSU_TCSR_OUT_BLOCK_STRIDE = 8,
  __RVT_LSU_TCSR_OUT_DIM0_STRIDE = 9,
  __RVT_LSU_TCSR_OUT_DIM1_STRIDE = 10,
  __RVT_LSU_TCSR_OUT_DIM2_STRIDE = 11,
  __RVT_LSU_TCSR_BLOCK_TRANSPOSE = 12,
  __RVT_LSU_TCSR_BLOCK_PADDING = 13,
  __RVT_LSU_TCSR_REQUEST_READ_SIZE = 14, // 6 bits, valid bytes - 1.
  __RVT_LSU_TCSR_PADDING_VALUE = 15,
  __RVT_LSU_TCSR_URAM_WRITE_SWIZZLE_MBS = 16,
  __RVT_LSU_TCSR_URAM_READ_SWIZZLE_MBS = 17,
  __RVT_LSU_TCSR_REQUEST_WRITE_SIZE = 18, // 6 bits, valid bytes - 1.
  __RVT_LSU_TCSR_TABLE_SIZE = 19,         // 24 bits, subtasks - 1.
  __RVT_LSU_TCSR_TABLE_STRIDE = 20,       // 28-bit byte stride.
};

static constexpr long __RVT_LSU_TCSR_GROUP_SIZE = 32;

template <long GROUP>
static constexpr long __rvt_lsu_tcsr_addr(__rvt_lsu_tcsr_offset Offset) {
  static_assert(GROUP >= 0 && GROUP < 8,
                "LSU TCSR group must be in the range [0, 7]");
  return GROUP * __RVT_LSU_TCSR_GROUP_SIZE + static_cast<long>(Offset);
}

// URAM address space qualifier. Write it inside a type to get a pointer into
// the Core-private on-chip scratchpad, or on an object definition to place the
// object itself in URAM:
//
//   __uram__ int tile[64];          // the array lives in URAM
//   __uram__ void *p = ...;         // a pointer that points to URAM
//
// Hardware APIs below take ordinary T* / void* or integer URAM addresses.
// A URAM pointer converts implicitly to those generic pointers; an integer
// address converts implicitly to a URAM pointer. The compiler still requires
// the call-site argument to be a URAM pointer or integer and diagnoses a
// DRAM pointer mismatch.
#ifndef __uram__
#if defined(__clang__) && defined(__riscv)
#define __uram__ __attribute__((npu_uram))
#else
#define __uram__
#endif
#endif

#if defined(__clang__) && defined(__riscv)
#define __RVT_NPU_PTR_AS(...) __attribute__((npu_ptr_as(__VA_ARGS__)))
#else
#define __RVT_NPU_PTR_AS(...)
#endif

template <typename T> using __rvt_uram_ptr = __uram__ T *;
// Address tables reside in URAM and contain tightly packed 64-bit byte
// addresses. The ISA requires each table base to be aligned to 32 bytes.
using __rvt_uram_addr_table_ptr = __rvt_uram_ptr<const unsigned long long>;

template <typename T>
static __attribute__((always_inline)) inline long __rvt_as_long(T __value) {
  return (long)__value;
}

template <long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_lsu_tcsr_read(long &rd) {
  rd = __builtin_riscv_tx_lsu_tcsr_read(TCSR_ADDR);
}

template <long TCSR_ADDR>
static __attribute__((always_inline)) inline void
__tx_lsu_tcsr_write(long value) {
  __builtin_riscv_tx_lsu_tcsr_write(value, TCSR_ADDR);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_u2u(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_u2u(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_u2u_g(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_u2u_g(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_u2u_s(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_u2u_s(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_u2u_sg(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_u2u_sg(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "integer") void
__tx_lsu_dma_d2u(T rd, long rs1) {
  __builtin_riscv_tx_lsu_dma_d2u(__rvt_as_long(rd), rs1, GROUP);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_d2u_g(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_d2u_g(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "integer") void
__tx_lsu_dma_d2u_s(T rd, long rs1) {
  __builtin_riscv_tx_lsu_dma_d2u_s(__rvt_as_long(rd), rs1, GROUP);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_d2u_sg(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_d2u_sg(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("integer", "uram") void
__tx_lsu_dma_u2d(long rd, T rs1) {
  __builtin_riscv_tx_lsu_dma_u2d(rd, __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("integer", "uram") void
__tx_lsu_dma_u2d_g(long rd, T rs1) {
  __builtin_riscv_tx_lsu_dma_u2d_g(rd, __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_u2d_s(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_u2d_s(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_u2d_sg(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_u2d_sg(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_lsu_dma_d2d(long rd,
                                                                   long rs1) {
  __builtin_riscv_tx_lsu_dma_d2d(rd, rs1, GROUP);
}

template <long GROUP, typename T>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("integer", "uram") void
__tx_lsu_dma_d2d_g(long rd, T rs1) {
  __builtin_riscv_tx_lsu_dma_d2d_g(rd, __rvt_as_long(rs1), GROUP);
}

template <long GROUP, typename T>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "integer") void
__tx_lsu_dma_d2d_s(T rd, long rs1) {
  __builtin_riscv_tx_lsu_dma_d2d_s(__rvt_as_long(rd), rs1, GROUP);
}

template <long GROUP, typename T, typename U>
static __attribute__((always_inline)) inline __RVT_NPU_PTR_AS("uram", "uram") void
__tx_lsu_dma_d2d_sg(T rd, U rs1) {
  __builtin_riscv_tx_lsu_dma_d2d_sg(__rvt_as_long(rd), __rvt_as_long(rs1), GROUP);
}

template <long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_mat_tcsr_read(long &rd) {
  rd = __builtin_riscv_tx_mat_tcsr_read(TCSR_ADDR);
}

template <long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_mat_tcsr_write(long value) {
  __builtin_riscv_tx_mat_tcsr_write(value, TCSR_ADDR);
}

template <long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_vec_tcsr_read(long &rd) {
  rd = __builtin_riscv_tx_vec_tcsr_read(TCSR_ADDR);
}

template <long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_vec_tcsr_write(long value) {
  __builtin_riscv_tx_vec_tcsr_write(value, TCSR_ADDR);
}

template <long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_macro_tcsr_read(long &rd) {
  rd = __builtin_riscv_tx_macro_tcsr_read(TCSR_ADDR);
}

template <long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_macro_tcsr_write(long value) {
  __builtin_riscv_tx_macro_tcsr_write(value, TCSR_ADDR);
}

static __attribute__((always_inline)) inline void __tx_mat_sb_produce(long value) {
  __builtin_riscv_tx_mat_sb_produce(value);
}

static __attribute__((always_inline)) inline void __tx_mat_sb_bar(long &rd) {
  rd = __builtin_riscv_tx_mat_sb_bar();
}

static __attribute__((always_inline)) inline void __tx_vec_sb_produce(long value) {
  __builtin_riscv_tx_vec_sb_produce(value);
}

static __attribute__((always_inline)) inline void __tx_vec_sb_bar(long &rd) {
  rd = __builtin_riscv_tx_vec_sb_bar();
}

static __attribute__((always_inline)) inline void __tx_lsu_sb_produce(long value) {
  __builtin_riscv_tx_lsu_sb_produce(value);
}

static __attribute__((always_inline)) inline void __tx_lsu_sb_bar(long &rd) {
  rd = __builtin_riscv_tx_lsu_sb_bar();
}

static __attribute__((always_inline)) inline void __tx_macro_sb_produce(long value) {
  __builtin_riscv_tx_macro_sb_produce(value);
}

static __attribute__((always_inline)) inline void __tx_macro_sb_bar(long &rd) {
  rd = __builtin_riscv_tx_macro_sb_bar();
}

static __attribute__((always_inline)) inline void __tx_sb_get_any(long &rd) {
  rd = __builtin_riscv_tx_sb_get_any();
}

#define __RVT_VEC_LSU_RD_VB_SEL_WRAPPER(NAME)                                \
  template <long VB, long SEL>                                                \
  static __attribute__((always_inline)) inline void __tx_vec_lsu_##NAME(long rd) { \
    __builtin_riscv_tx_vec_lsu_##NAME(rd, VB, SEL);                           \
  }

__RVT_VEC_LSU_RD_VB_SEL_WRAPPER(u2v)
__RVT_VEC_LSU_RD_VB_SEL_WRAPPER(sm2v)
__RVT_VEC_LSU_RD_VB_SEL_WRAPPER(u2v_broadcast32)
__RVT_VEC_LSU_RD_VB_SEL_WRAPPER(sm2v_broadcast32)
__RVT_VEC_LSU_RD_VB_SEL_WRAPPER(g2v_broadcast32)

#undef __RVT_VEC_LSU_RD_VB_SEL_WRAPPER

template <long VB, long SEL, long S1>
static __attribute__((always_inline)) inline void
__tx_vec_lsu_s2v_broadcast32() {
  __builtin_riscv_tx_vec_lsu_s2v_broadcast32(VB, SEL, S1);
}

template <long SD, long SEL>
static __attribute__((always_inline)) inline void __tx_vec_lsu_u2s(long rd) {
  __builtin_riscv_tx_vec_lsu_u2s(rd, SD, SEL);
}

template <long SD, long SEL>
static __attribute__((always_inline)) inline void __tx_vec_lsu_sm2s(long rd) {
  __builtin_riscv_tx_vec_lsu_sm2s(rd, SD, SEL);
}

template <long SD>
static __attribute__((always_inline)) inline void __tx_vec_lsu_g2s(long rd) {
  __builtin_riscv_tx_vec_lsu_g2s(rd, SD);
}

#define __RVT_VEC_LSU_MASK_RD_WRAPPER(NAME)                                   \
  template <long VB, long VRS1, long S2>                                       \
  static __attribute__((always_inline)) inline void __tx_vec_lsu_##NAME(long rd) { \
    __builtin_riscv_tx_vec_lsu_##NAME(rd, VB, VRS1, S2);                      \
  }

__RVT_VEC_LSU_MASK_RD_WRAPPER(u2v_mask)
__RVT_VEC_LSU_MASK_RD_WRAPPER(sm2v_mask)
__RVT_VEC_LSU_MASK_RD_WRAPPER(u2v_mask_broadcast32)
__RVT_VEC_LSU_MASK_RD_WRAPPER(sm2v_mask_broadcast32)
__RVT_VEC_LSU_MASK_RD_WRAPPER(g2v_mask_broadcast32)

#undef __RVT_VEC_LSU_MASK_RD_WRAPPER

template <long VB, long VRS1, long S2, long S1>
static __attribute__((always_inline)) inline void
__tx_vec_lsu_s2v_mask_broadcast32() {
  __builtin_riscv_tx_vec_lsu_s2v_mask_broadcast32(VB, VRS1, S2, S1);
}

#define __RVT_VEC_LSU_STORE_WRAPPER(NAME)                                      \
  template <long VRS1, long SEL>                                               \
  static __attribute__((always_inline)) inline void __tx_vec_lsu_##NAME(long rd) { \
    __builtin_riscv_tx_vec_lsu_##NAME(rd, VRS1, SEL);                         \
  }

__RVT_VEC_LSU_STORE_WRAPPER(v2u)
__RVT_VEC_LSU_STORE_WRAPPER(v2sm)
__RVT_VEC_LSU_STORE_WRAPPER(s2u)
__RVT_VEC_LSU_STORE_WRAPPER(s2sm)

#undef __RVT_VEC_LSU_STORE_WRAPPER

template <long VRS1, long SEL, long S2>
static __attribute__((always_inline)) inline void __tx_vec_lsu_v2u_mask(long rd) {
  __builtin_riscv_tx_vec_lsu_v2u_mask(rd, VRS1, SEL, S2);
}

template <long VRS1, long SEL, long S2>
static __attribute__((always_inline)) inline void
__tx_vec_lsu_v2sm_mask(long rd) {
  __builtin_riscv_tx_vec_lsu_v2sm_mask(rd, VRS1, SEL, S2);
}

template <long VB, long VRS1, long VRS2, long S2>
static __attribute__((always_inline)) inline void __tx_vec_sm2v_sparse(long rd) {
  __builtin_riscv_tx_vec_sm2v_sparse(rd, VB, VRS1, VRS2, S2);
}

template <long VRS1, long VRS2, long S2>
static __attribute__((always_inline)) inline void __tx_vec_v2sm_sparse(long rd) {
  __builtin_riscv_tx_vec_v2sm_sparse(rd, VRS1, VRS2, S2);
}

#define __RVT_VEC_SPARSE_INT32_WRAPPER(NAME)                                  \
  template <long VB, long VRS1, long VRS2, long S2>                            \
  static __attribute__((always_inline)) inline void                            \
  __tx_vec_v2sm_sparse_int32_##NAME(long rd) {                                 \
    __builtin_riscv_tx_vec_v2sm_sparse_int32_##NAME(rd, VB, VRS1, VRS2, S2);   \
  }

__RVT_VEC_SPARSE_INT32_WRAPPER(add)
__RVT_VEC_SPARSE_INT32_WRAPPER(and)
__RVT_VEC_SPARSE_INT32_WRAPPER(or)
__RVT_VEC_SPARSE_INT32_WRAPPER(xor)

#undef __RVT_VEC_SPARSE_INT32_WRAPPER

template <long VB, long VRS1, long VRS2, long S2, long S1>
static __attribute__((always_inline)) inline void __tx_vec_sm2v_dyn() {
  __builtin_riscv_tx_vec_sm2v_dyn(VB, VRS1, VRS2, S2, S1);
}

template <long VRS1, long VRS2, long S2, long S1>
static __attribute__((always_inline)) inline void __tx_vec_v2sm_dyn() {
  __builtin_riscv_tx_vec_v2sm_dyn(VRS1, VRS2, S2, S1);
}

#define __RVT_VEC_DYN_INT32_WRAPPER(NAME)                                     \
  template <long VB, long VRS1, long VRS2, long S2, long S1>                  \
  static __attribute__((always_inline)) inline void                            \
  __tx_vec_v2sm_dyn_int32_##NAME() {                                           \
    __builtin_riscv_tx_vec_v2sm_dyn_int32_##NAME(VB, VRS1, VRS2, S2, S1);     \
  }

__RVT_VEC_DYN_INT32_WRAPPER(add)
__RVT_VEC_DYN_INT32_WRAPPER(and)
__RVT_VEC_DYN_INT32_WRAPPER(or)
__RVT_VEC_DYN_INT32_WRAPPER(xor)

#undef __RVT_VEC_DYN_INT32_WRAPPER

#define __RVT_VEC_SHUFFLE_RD_WRAPPER(NAME)                                    \
  template <long VB, long VRS1>                                                \
  static __attribute__((always_inline)) inline void __tx_vec_shuffle_##NAME(long rd) { \
    __builtin_riscv_tx_vec_shuffle_##NAME(rd, VB, VRS1);                      \
  }

__RVT_VEC_SHUFFLE_RD_WRAPPER(xor)
__RVT_VEC_SHUFFLE_RD_WRAPPER(ror)
__RVT_VEC_SHUFFLE_RD_WRAPPER(broadcast)
__RVT_VEC_SHUFFLE_RD_WRAPPER(compress)

#undef __RVT_VEC_SHUFFLE_RD_WRAPPER

#define __RVT_VEC_SHUFFLE_DYN_WRAPPER(NAME)                                   \
  template <long VB, long VRS1, long S1>                                       \
  static __attribute__((always_inline)) inline void __tx_vec_shuffle_##NAME##_dyn() { \
    __builtin_riscv_tx_vec_shuffle_##NAME##_dyn(VB, VRS1, S1);                \
  }

__RVT_VEC_SHUFFLE_DYN_WRAPPER(xor)
__RVT_VEC_SHUFFLE_DYN_WRAPPER(ror)
__RVT_VEC_SHUFFLE_DYN_WRAPPER(broadcast)
__RVT_VEC_SHUFFLE_DYN_WRAPPER(compress)

#undef __RVT_VEC_SHUFFLE_DYN_WRAPPER

template <long VB, long VRS1, long VRS2, long LUT>
static __attribute__((always_inline)) inline void __tx_vec_vv_int32_bitwise() {
  __builtin_riscv_tx_vec_vv_int32_bitwise(VB, VRS1, VRS2, LUT);
}

#define __RVT_VEC_VV_INT32_WRAPPER(NAME)                                      \
  template <long VB, long VRS1, long VRS2>                                    \
  static __attribute__((always_inline)) inline void __tx_vec_vv_int32_##NAME() { \
    __builtin_riscv_tx_vec_vv_int32_##NAME(VB, VRS1, VRS2);                   \
  }

__RVT_VEC_VV_INT32_WRAPPER(rol)
__RVT_VEC_VV_INT32_WRAPPER(sll)
__RVT_VEC_VV_INT32_WRAPPER(sra)
__RVT_VEC_VV_INT32_WRAPPER(srl)
__RVT_VEC_VV_INT32_WRAPPER(add)
__RVT_VEC_VV_INT32_WRAPPER(sub)
__RVT_VEC_VV_INT32_WRAPPER(mulu24)
__RVT_VEC_VV_INT32_WRAPPER(mul24)
__RVT_VEC_VV_INT32_WRAPPER(maxu)
__RVT_VEC_VV_INT32_WRAPPER(minu)
__RVT_VEC_VV_INT32_WRAPPER(max)
__RVT_VEC_VV_INT32_WRAPPER(min)

#undef __RVT_VEC_VV_INT32_WRAPPER

#define __RVT_VEC_VV_FP32_WRAPPER(NAME)                                       \
  template <long VB, long VRS1, long VRS2, long RM>                           \
  static __attribute__((always_inline)) inline void __tx_vec_vv_fp32_##NAME() { \
    __builtin_riscv_tx_vec_vv_fp32_##NAME(VB, VRS1, VRS2, RM);                \
  }

__RVT_VEC_VV_FP32_WRAPPER(add)
__RVT_VEC_VV_FP32_WRAPPER(sub)
__RVT_VEC_VV_FP32_WRAPPER(mul)
__RVT_VEC_VV_FP32_WRAPPER(max)
__RVT_VEC_VV_FP32_WRAPPER(min)

#undef __RVT_VEC_VV_FP32_WRAPPER

template <long VB, long VRS1, long VRS2, long S2>
static __attribute__((always_inline)) inline void __tx_vec_vv_mux2() {
  __builtin_riscv_tx_vec_vv_mux2(VB, VRS1, VRS2, S2);
}

template <long VRS1, long VRS2, long SD, long CMP>
static __attribute__((always_inline)) inline void __tx_vec_vv_int32_cmp() {
  __builtin_riscv_tx_vec_vv_int32_cmp(VRS1, VRS2, SD, CMP);
}

template <long VRS1, long VRS2, long SD, long CMP>
static __attribute__((always_inline)) inline void __tx_vec_vv_fp32_cmp() {
  __builtin_riscv_tx_vec_vv_fp32_cmp(VRS1, VRS2, SD, CMP);
}

#define __RVT_VEC_V_INT32_WRAPPER(NAME)                                       \
  template <long VB, long VRS1, long SFT>                                     \
  static __attribute__((always_inline)) inline void __tx_vec_v_int32_##NAME() { \
    __builtin_riscv_tx_vec_v_int32_##NAME(VB, VRS1, SFT);                    \
  }

__RVT_VEC_V_INT32_WRAPPER(sll)
__RVT_VEC_V_INT32_WRAPPER(sra)
__RVT_VEC_V_INT32_WRAPPER(srl)

#undef __RVT_VEC_V_INT32_WRAPPER

template <long VB, long VRS1, long OP>
static __attribute__((always_inline)) inline void __tx_vec_v_fp32_sfu() {
  __builtin_riscv_tx_vec_v_fp32_sfu(VB, VRS1, OP);
}

#define __RVT_VEC_CVT_WRAPPER(DST, SRC)                                       \
  template <long VB, long VRS1, long RM>                                      \
  static __attribute__((always_inline)) inline void __tx_vec_cvt_##DST##_##SRC() { \
    __builtin_riscv_tx_vec_cvt_##DST##_##SRC(VB, VRS1, RM);                  \
  }

__RVT_VEC_CVT_WRAPPER(fp32, int32)
__RVT_VEC_CVT_WRAPPER(s32, fp32)
__RVT_VEC_CVT_WRAPPER(fp32, fp16)
__RVT_VEC_CVT_WRAPPER(fp32, bf16)
__RVT_VEC_CVT_WRAPPER(fp16, fp32)
__RVT_VEC_CVT_WRAPPER(bf16, fp32)
__RVT_VEC_CVT_WRAPPER(mxfp8, bf16)
__RVT_VEC_CVT_WRAPPER(mxfp4, bf16)

#undef __RVT_VEC_CVT_WRAPPER

#define __RVT_VEC_S_INT32_WRAPPER(NAME)                                      \
  template <long SD, long S1, long S2>                                       \
  static __attribute__((always_inline)) inline void __tx_vec_s_int32_##NAME() { \
    __builtin_riscv_tx_vec_s_int32_##NAME(SD, S1, S2);                      \
  }

__RVT_VEC_S_INT32_WRAPPER(rol)
__RVT_VEC_S_INT32_WRAPPER(sll)
__RVT_VEC_S_INT32_WRAPPER(sra)
__RVT_VEC_S_INT32_WRAPPER(srl)
__RVT_VEC_S_INT32_WRAPPER(add)
__RVT_VEC_S_INT32_WRAPPER(sub)
__RVT_VEC_S_INT32_WRAPPER(mul)

#undef __RVT_VEC_S_INT32_WRAPPER

template <long SD, long DIM>
static __attribute__((always_inline)) inline void __tx_vec_s_iter_getdim() {
  __builtin_riscv_tx_vec_s_iter_getdim(SD, DIM);
}

template <long SD, long S1, long S2, long SEL>
static __attribute__((always_inline)) inline void __tx_vec_s_iter_gets() {
  __builtin_riscv_tx_vec_s_iter_gets(SD, S1, S2, SEL);
}

template <long SD, long S1, long S2, long SEL>
static __attribute__((always_inline)) inline void __tx_vec_s_iter_getv() {
  __builtin_riscv_tx_vec_s_iter_getv(SD, S1, S2, SEL);
}

template <long SD, long S1, long S2, long LUT>
static __attribute__((always_inline)) inline void __tx_vec_s_bitwise() {
  __builtin_riscv_tx_vec_s_bitwise(SD, S1, S2, LUT);
}

template <long TARGET, long S1, long S2>
static __attribute__((always_inline)) inline void __tx_vec_s_bitwise_beq() {
  __builtin_riscv_tx_vec_s_bitwise_beq(TARGET, S1, S2);
}

template <long TARGET, long S1, long S2>
static __attribute__((always_inline)) inline void __tx_vec_s_bitwise_bne() {
  __builtin_riscv_tx_vec_s_bitwise_bne(TARGET, S1, S2);
}

template <long TARGET, long S1, long S2>
static __attribute__((always_inline)) inline void __tx_vec_s_int32_blt() {
  __builtin_riscv_tx_vec_s_int32_blt(TARGET, S1, S2);
}

template <long TARGET, long S1, long S2>
static __attribute__((always_inline)) inline void __tx_vec_s_int32_bge() {
  __builtin_riscv_tx_vec_s_int32_bge(TARGET, S1, S2);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_mat_setptr(long rs1, long rs2) {
  __builtin_riscv_tx_mat_setptr(rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_mat_mm(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_mm(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_mat_mma(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_mma(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_mat_conv2d(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_conv2d(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_mat_conv2da(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_mat_conv2da(rd, rs1, rs2, GROUP);
}

template <long UNIT, long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_base_tcsr_read(long &rd) {
  rd = __builtin_riscv_tx_base_tcsr_read(UNIT, TCSR_ADDR);
}

template <long UNIT, long TCSR_ADDR>
static __attribute__((always_inline)) inline void __tx_base_tcsr_write(long value) {
  __builtin_riscv_tx_base_tcsr_write(value, UNIT, TCSR_ADDR);
}

// Elementwise operands: destination address, source 0, source 1.
// In iv mode source 0 is a scalar value supplied by a GPR.
template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_add_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_add_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_add_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_add_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_add_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_add_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_sub_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_sub_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_sub_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_sub_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_sub_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_sub_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_mul_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_mul_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_mul_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_mul_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_mul_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_mul_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_max_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_max_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_max_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_max_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_max_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_max_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_min_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_min_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_min_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_min_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_min_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_min_fp32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_add_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_add_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_add_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_add_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_add_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_add_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_sub_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_sub_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_sub_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_sub_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_sub_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_sub_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_mul_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_mul_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_mul_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_mul_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_mul_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_mul_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_max_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_max_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_max_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_max_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_max_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_max_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_min_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_min_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_min_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_min_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_min_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_min_s32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_add_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_add_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_add_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_add_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_add_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_add_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_sub_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_sub_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_sub_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_sub_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_sub_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_sub_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_mul_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_mul_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_mul_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_mul_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_mul_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_mul_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_max_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_max_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_max_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_max_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_max_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_max_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_vv_min_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_vv_min_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_sv_min_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_sv_min_u32(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_elemwise_iv_min_u32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_elemwise_iv_min_u32(rd, rs1, rs2, GROUP);
}

// Conversion suffixes are destination_source. FP8 and MXFP8 use E4M3.
template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_s32_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_s32_fp32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_s16_s32(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_s16_s32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_bf16_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_bf16_fp32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_s32_s16(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_s32_s16(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_mxfp8_bf16(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_mxfp8_bf16(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_s8_s16(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_s8_s16(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_s16_s8(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_s16_s8(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_bf16_s8(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_bf16_s8(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_bf16_u8(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_bf16_u8(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_fp8_s4(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp8_s4(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_fp8_u4(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp8_u4(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_s32_f32(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_s32_f32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_fp32_s32_round(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_s32_round(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_fp32_s32(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_s32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_fp32_fp8(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_fp8(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_fp32_fp16(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_fp16(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_convert_fp32_bf16(long rd, long rs1) {
  __builtin_riscv_tx_macro_convert_fp32_bf16(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_exp(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_exp(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_log2_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_log2_fp32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_exp2_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_exp2_fp32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_sin_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_sin_fp32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_cos_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_cos_fp32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_rsqrt_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_rsqrt_fp32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_recip_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_recip_fp32(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_reduce_add_s32(long rd, long rs1) {
  __builtin_riscv_tx_macro_reduce_add_s32(rd, rs1, GROUP);
}

template <long GROUP, long INDEX>
static __attribute__((always_inline)) inline void __tx_macro_reduce_max_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_reduce_max_fp32(rd, rs1, rs2, GROUP, INDEX);
}

template <long GROUP, long INDEX>
static __attribute__((always_inline)) inline void __tx_macro_reduce_min_fp32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_reduce_min_fp32(rd, rs1, rs2, GROUP, INDEX);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_reduce_add_fp32(long rd, long rs1) {
  __builtin_riscv_tx_macro_reduce_add_fp32(rd, rs1, GROUP);
}

template <long GROUP, long INDEX>
static __attribute__((always_inline)) inline void __tx_macro_reduce_max_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_reduce_max_s32(rd, rs1, rs2, GROUP, INDEX);
}

template <long GROUP, long INDEX>
static __attribute__((always_inline)) inline void __tx_macro_reduce_min_s32(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_reduce_min_s32(rd, rs1, rs2, GROUP, INDEX);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_shuffle_compress(long rd, long rs1, long rs2) {
  __builtin_riscv_tx_macro_shuffle_compress(rd, rs1, rs2, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_log2(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_log2(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_exp2(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_exp2(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_sin(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_sin(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_cos(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_cos(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_rsqrt(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_rsqrt(rd, rs1, GROUP);
}

template <long GROUP>
static __attribute__((always_inline)) inline void __tx_macro_sfu_recip(long rd, long rs1) {
  __builtin_riscv_tx_macro_sfu_recip(rd, rs1, GROUP);
}

#endif // defined(__cplusplus)

#endif // __RVT_INTRINSICS_H__
