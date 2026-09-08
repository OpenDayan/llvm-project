// RUN: %clang_cc1 -triple riscv64 -fsyntax-only -verify %s

#define __uram__ __attribute__((npu_uram))

void init_out_of_range_upper(void) {
  __uram__ int *p = (__uram__ int *)(0x10000000); // expected-error {{NPU URAM pointer address 268435456 is outside the valid range [0, 1572863]}}
  (void)p;
}

void init_out_of_range_lower(void) {
  __uram__ int *p = (__uram__ int *)(-1); // expected-error {{NPU URAM pointer address -1 is outside the valid range [0, 1572863]}}
  (void)p;
}

void assign_out_of_range(void) {
  __uram__ int *p = (__uram__ int *)(0);
  p = (__uram__ int *)(0x180000); // expected-error {{NPU URAM pointer address 1572864 is outside the valid range [0, 1572863]}}
}

void init_ok(void) {
  __uram__ int *p = (__uram__ int *)(0);
  __uram__ int *q = (__uram__ int *)(0x10000);
  __uram__ int *r = (__uram__ int *)(0x17ffff);
  (void)p;
  (void)q;
  (void)r;
}

void implicit_integer_to_uram(unsigned long addr) {
  __uram__ int *p = addr;
  __uram__ int *q = 0x10000;
  (void)p;
  (void)q;
}

void implicit_integer_out_of_range(void) {
  __uram__ int *p = 0x10000000UL; // expected-error {{NPU URAM pointer address 268435456 is outside the valid range [0, 1572863]}}
  (void)p;
}

// URAM pointers implicitly convert to DRAM/generic pointers (like OpenCL
// named-to-generic). A DRAM pointer still requires an explicit cast to URAM.
// Integers convert implicitly to URAM pointers.
void implicit_uram_to_generic(int *dram, __uram__ int *uram) {
  __uram__ int *p = dram; // expected-error {{changes address space of pointer}}
  int *q = uram;
  (void)p;
  (void)q;
}

void explicit_conversion_is_allowed(int *dram, __uram__ int *uram) {
  __uram__ int *p = (__uram__ int *)dram;
  int *q = (int *)uram;
  (void)p;
  (void)q;
}

// A pointer into URAM is itself an ordinary variable; only an object declared
// in URAM is placed there.
__uram__ int uram_object;
__uram__ int *dram_pointer_to_uram;

_Static_assert(sizeof(dram_pointer_to_uram) == sizeof(void *), "");

/* Function return types may carry the qualifier via a typedef or directly. */
typedef __uram__ void *uram_void_ptr;
uram_void_ptr return_uram_typedef(void);
__uram__ void *return_uram_direct(void);
