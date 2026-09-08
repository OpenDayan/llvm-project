// RUN: %clang_cc1 -triple riscv64 -fsyntax-only -verify -std=c++17 %s

#define __uram__ __attribute__((npu_uram))

template <long Group, typename T>
static __attribute__((always_inline, npu_ptr_as("uram", "integer"))) inline void
dma_d2u(T dst, long src) {}

__uram__ int uram_buf[8];
int dram_buf[8];

void ok_template() {
  dma_d2u<0>(uram_buf, 0);
  __uram__ int *p = uram_buf;
  dma_d2u<1>(p, 0);
}

void ok_integer(unsigned long addr) {
  __uram__ void *p = addr;
  dma_d2u<0>(p, 0);
  dma_d2u<1>(addr, 0);
}

void integer_not_to_dram(unsigned long addr) {
  int *p = addr; // expected-error {{cannot initialize a variable of type 'int *'}}
  (void)p;
}

void implicit_integer_out_of_range() {
  __uram__ int *p = 0x10000000UL; // expected-error {{NPU URAM pointer address 268435456 is outside the valid range [0, 1572863]}}
  (void)p;
}

void bad_template(int *dram) {
  dma_d2u<0>(dram, 0); // expected-error {{invalid pointer address space for 'dma_d2u<0L, int *>', expected '__uram__'}}
  dma_d2u<0>(dram_buf, 0); // expected-error {{invalid pointer address space for 'dma_d2u<0L, int *>', expected '__uram__'}}
}
