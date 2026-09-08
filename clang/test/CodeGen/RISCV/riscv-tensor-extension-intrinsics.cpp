// REQUIRES: riscv-registered-target
// RUN: %clang_cc1 -triple riscv64 -target-feature +txlsu \
// RUN:   -target-feature +txmat -target-feature +txbase -target-feature +txvec \
// RUN:   -target-feature +txmacro -std=c++14 \
// RUN:   -internal-isystem %S/../../../lib/Headers -emit-llvm -o - %s \
// RUN:   | FileCheck %s
// RUN: %clang_cc1 -triple riscv64 -target-feature +ztxlsu \
// RUN:   -target-feature +txmat -target-feature +txbase -target-feature +txvec \
// RUN:   -target-feature +txmacro -std=c++14 \
// RUN:   -internal-isystem %S/../../../lib/Headers -emit-llvm -o - %s \
// RUN:   | FileCheck %s

#include <rvt_intrinsics.h>

static_assert(__RVT_LSU_TCSR_REQUEST_READ_SIZE == 14, "RequestReadSize");
static_assert(__RVT_LSU_TCSR_REQUEST_WRITE_SIZE == 18, "RequestWriteSize");
static_assert(__RVT_LSU_TCSR_TABLE_SIZE == 19, "TableSize");
static_assert(__RVT_LSU_TCSR_TABLE_STRIDE == 20, "TableStride");
static_assert(sizeof(unsigned long long) == 8, "64-bit address table entry");
static_assert(__rvt_lsu_tcsr_addr<7>(__RVT_LSU_TCSR_TABLE_STRIDE) == 244,
              "last LSU group TableStride address");

void test_lsu_tcsr_extended_fields_intrinsics(long value) {
  __tx_lsu_tcsr_write<
      __rvt_lsu_tcsr_addr<2>(__RVT_LSU_TCSR_REQUEST_READ_SIZE)>(value);
  __tx_lsu_tcsr_write<
      __rvt_lsu_tcsr_addr<2>(__RVT_LSU_TCSR_REQUEST_WRITE_SIZE)>(value);
  __tx_lsu_tcsr_write<__rvt_lsu_tcsr_addr<2>(__RVT_LSU_TCSR_TABLE_SIZE)>(
      value);
  __tx_lsu_tcsr_write<__rvt_lsu_tcsr_addr<2>(__RVT_LSU_TCSR_TABLE_STRIDE)>(
      value);
}

// CHECK-LABEL: @{{.*}}test_lsu_tcsr_extended_fields_intrinsics
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 78)
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 82)
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 83)
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 84)

void test_lsu_dma_table_intrinsics(
    __rvt_uram_ptr<void> uram_dst, __rvt_uram_ptr<const void> uram_src,
    __rvt_uram_addr_table_ptr dst_table,
    __rvt_uram_addr_table_ptr src_table, long dram_dst, long dram_src) {
  __tx_lsu_dma_u2u<2>(uram_dst, uram_src);
  __tx_lsu_dma_u2d<2>(dram_dst, uram_src);
  __tx_lsu_dma_d2u<2>(uram_dst, dram_src);
  __tx_lsu_dma_d2d<2>(dram_dst, dram_src);
  __tx_lsu_dma_u2u_g<2>(uram_dst, src_table);
  __tx_lsu_dma_u2u_s<2>(dst_table, uram_src);
  __tx_lsu_dma_u2u_sg<2>(dst_table, src_table);
  __tx_lsu_dma_u2d_g<2>(dram_dst, src_table);
  __tx_lsu_dma_u2d_s<2>(dst_table, uram_src);
  __tx_lsu_dma_u2d_sg<2>(dst_table, src_table);
  __tx_lsu_dma_d2u_g<2>(uram_dst, src_table);
  __tx_lsu_dma_d2u_s<2>(dst_table, dram_src);
  __tx_lsu_dma_d2u_sg<2>(dst_table, src_table);
  __tx_lsu_dma_d2d_g<2>(dram_dst, src_table);
  __tx_lsu_dma_d2d_s<2>(dst_table, dram_src);
  __tx_lsu_dma_d2d_sg<2>(dst_table, src_table);
}

void test_lsu_dma_nontranspose_unaligned(
    __rvt_uram_ptr<char> uram_dst, __rvt_uram_ptr<const char> uram_src,
    long dram_dst, long dram_src) {
  __tx_lsu_tcsr_write<
      __rvt_lsu_tcsr_addr<0>(__RVT_LSU_TCSR_BLOCK_TRANSPOSE)>(6);
  __tx_lsu_tcsr_write<
      __rvt_lsu_tcsr_addr<0>(__RVT_LSU_TCSR_REQUEST_READ_SIZE)>(15);
  __tx_lsu_tcsr_write<
      __rvt_lsu_tcsr_addr<0>(__RVT_LSU_TCSR_REQUEST_WRITE_SIZE)>(7);
  __tx_lsu_dma_u2u<0>(uram_dst + 1, uram_src + 3);
  __tx_lsu_dma_u2d<0>(dram_dst + 5, uram_src + 7);
  __tx_lsu_dma_d2u<0>(uram_dst + 9, dram_src + 11);
}

// CHECK-LABEL: @{{.*}}test_lsu_dma_table_intrinsics
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2d $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u.g $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u.s $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u.sg $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d.g $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d.s $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d.sg $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u.g $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u.s $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u.sg $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2d.g $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2d.s $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2d.sg $0, $1, $2"
// CHECK-LABEL: @{{.*}}test_lsu_dma_nontranspose_unaligned
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 12)
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 14)
// CHECK: call void asm sideeffect "tx.lsu.tcsr.write $0, $1", "r,i"(i64 %{{.*}}, i32 18)
// CHECK: call void asm sideeffect "tx.lsu.dma.u2u $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.u2d $0, $1, $2"
// CHECK: call void asm sideeffect "tx.lsu.dma.d2u $0, $1, $2"

