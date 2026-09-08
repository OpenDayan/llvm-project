// RUN: rm -rf %t && mkdir -p %t
// RUN: dycc --dayan-root %S/Inputs --device-cxx %clangxx --host-cxx %clangxx \
// RUN:   --split-only --emit-host %t/host.cpp --emit-device %t/device.cpp \
// RUN:   %s
// RUN: FileCheck %s --check-prefix=HOST < %t/host.cpp
// RUN: FileCheck %s --check-prefix=DEVICE < %t/device.cpp

#include <dayan_kernel.h>

__host__ int host_value = 7;
__device__ int device_value = 9;

__kernel__ void add(int *output, int value);

__host__ int launch(npu_buffer_t *output,
                    const npu_kernel_launch_attrs_t *attrs) {
  return add(output, host_value, attrs);
}

__kernel__ void add(int *output, int value) {
  *output = value + device_value;
}

DAYAN_KERNEL_DESC(add, 1, 64, 32768, 1, 2, 3, 4);

// HOST: int add(::npu_buffer_t *output, int value,
// HOST: 1U, 0U, 0U, 1U, 2U, 3U, 4U, 0U, UINT64_C(64), UINT64_C(32768)
// HOST: npu_kernel_descriptor_create(
// HOST: "_Z3addPii"
// HOST: npu_kernel_descriptor_launch

// DEVICE: __device__ int device_value = 9;
// DEVICE: __kernel__ void add(int *output, int value)
// DEVICE: DAYAN_KERNEL_DESC(add, 1, 64, 32768, 1, 2, 3, 4);
