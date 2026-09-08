// RUN: rm -rf %t && mkdir -p %t
// RUN: dycc --dayan-root %S/Inputs --device-cxx %clangxx --host-cxx %clangxx \
// RUN:   -c --emit-package %t/add.dypkg %s -o %t/add.o
// RUN: %python %S/Inputs/check-dypkg.py %t/add.dypkg
// RUN: not dycc --dayan-root %S/Inputs --device-cxx %clangxx \
// RUN:   --split-only --emit-package %t/add.dypkg %s 2>&1 | FileCheck %s --check-prefix=SPLIT
// RUN: not dycc --dayan-root %S/Inputs --device-cxx %clangxx \
// RUN:   -c --emit-package %t/add.dypkg %s %s 2>&1 | FileCheck %s --check-prefix=MULTI

#include <dayan_kernel.h>

__device__ int device_value = 9;

__kernel__ void add(int *output, int value) {
  *output = value + device_value;
}

DAYAN_KERNEL_DESC(add, 1, 64, 32768, 1, 2, 3, 4);

// SPLIT: error: --emit-package cannot be used with --split-only
// MULTI: error: --emit-package with -c currently requires exactly one input source
