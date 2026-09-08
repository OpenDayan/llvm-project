// RUN: %clang_cc1 -triple riscv64 -fsyntax-only -verify %s

#define __uram__ __attribute__((npu_uram))

__attribute__((npu_ptr_as("uram", "integer")))
void dma_d2u(void *dst, long src);

__attribute__((npu_ptr_as("integer", "uram")))
void dma_u2d(long dst, const void *src);

__uram__ int uram_buf[8];
int dram_buf[8];

void ok_direct_object(void) {
  dma_d2u(uram_buf, 0);
  dma_u2d(0, uram_buf);
}

void ok_integer(unsigned long addr) {
  __uram__ void *p = addr;
  dma_d2u(p, 0);
  dma_u2d(0, p);
}

void bad_dram_to_uram(int *dram) {
  dma_d2u(dram, 0); // expected-error {{invalid pointer address space for 'dma_d2u', expected '__uram__'}}
  dma_u2d(0, dram); // expected-error {{invalid pointer address space for 'dma_u2d', expected '__uram__'}}
}

void stripped_generic_pointer(void) {
  int *p = uram_buf;
  dma_d2u(p, 0); // expected-error {{invalid pointer address space for 'dma_d2u', expected '__uram__'}}
}

void explicit_cast_strips(void) {
  dma_d2u((void *)uram_buf, 0); // expected-error {{invalid pointer address space for 'dma_d2u', expected '__uram__'}}
}

__attribute__((npu_ptr_as("uram", "integer")))
void wrapper(void *dst, long src) {
  dma_d2u(dst, src);
}

void wrapper_rejects_dram(int *dram) {
  wrapper(dram, 0); // expected-error {{invalid pointer address space for 'wrapper', expected '__uram__'}}
}

void wrapper_accepts_uram(__uram__ int *p) {
  wrapper(p, 0);
}

__attribute__((npu_ptr_as("uram"))) // expected-error {{'npu_ptr_as' attribute has 1 entries but 'too_few' has 2 parameters}}
void too_few(void *a, void *b);
