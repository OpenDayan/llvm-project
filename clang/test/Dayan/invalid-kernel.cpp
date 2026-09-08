// RUN: not dycc --dayan-root %S/Inputs --device-cxx %clangxx \
// RUN:   --split-only --emit-host %t.host.cpp --emit-device %t.device.cpp \
// RUN:   %s 2>&1 | FileCheck %s

#include <dayan_kernel.h>

__kernel__ int invalid_kernel(int value) { return value; }
DAYAN_KERNEL_DESC(invalid_kernel, 0, 0, 0, 0, 0, 0, 0);

// CHECK: error: __kernel__ functions must return void
