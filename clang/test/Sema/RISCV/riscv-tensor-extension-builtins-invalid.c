// RUN: %clang_cc1 -triple riscv64 -target-feature +txlsu -fsyntax-only \
// RUN:   -verify %s

void test_lsu_dma_table_group_must_be_constant(long rd, long rs1,
                                                unsigned group) {
  __builtin_riscv_tx_lsu_dma_u2u_g(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_u2u_s(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_u2u_sg(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_u2d_g(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_u2d_s(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_u2d_sg(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_d2u_g(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_d2u_s(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_d2u_sg(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_d2d_g(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_d2d_s(rd, rs1, group); // expected-error {{must be a constant integer}}
  __builtin_riscv_tx_lsu_dma_d2d_sg(rd, rs1, group); // expected-error {{must be a constant integer}}
}
