// RUN: rm -rf %t && mkdir -p %t
// RUN: llvm-ar crs %t/libruntime.a
// RUN: env DYCC_RUNTIME_LIBRARY=%t/libruntime.a dycc --dayan-root %S/Inputs \
// RUN:   --device-cxx %clangxx --host-cxx %clangxx \
// RUN:   --device-linker ld.lld --objcopy llvm-objcopy \
// RUN:   --save-temps=%t %S/Inputs/device-link-helper.cpp \
// RUN:   %S/Inputs/device-link-kernel.cpp -o %t/linked.o -Wl,-r -nostdlib -no-pie
// RUN: llvm-readelf --file-header %t/linked.dayan.device.o | FileCheck %s --check-prefix=RISCV
// RUN: env DYCC_RUNTIME_LIBRARY=%t/libruntime.a dycc --dayan-root %S/Inputs \
// RUN:   --device-cxx %clangxx --host-cxx %clangxx \
// RUN:   --device-linker ld.lld --objcopy llvm-objcopy \
// RUN:   --save-temps=%t/single %S/Inputs/device-link-kernel.cpp \
// RUN:   -o %t/single-linked.o -Wl,-r -nostdlib -no-pie
// RUN: test -f %t/single/device-link-kernel.cpp.dycc.device.o
// RUN: test -f %t/device-link-helper.cpp.0.dycc.device.o
// RUN: test -f %t/device-link-kernel.cpp.1.dycc.device.o
// RUN: llvm-nm %t/linked.dayan.device.o | FileCheck %s --check-prefix=DEVICE
// RUN: llvm-nm %t/linked.o | FileCheck %s --check-prefix=HOST

// This file drives a multi-source dycc link test through RUN lines above.

// RISCV: Machine: RISC-V
// DEVICE-DAG: T _Z15cross_tu_helperi
// DEVICE-DAG: T _Z13linked_kernelPii
// HOST: R __kernel_obj__
