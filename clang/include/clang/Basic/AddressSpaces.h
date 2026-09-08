//===- AddressSpaces.h - Language-specific address spaces -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Provides definitions for the various language-specific address
/// spaces.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_ADDRESSSPACES_H
#define LLVM_CLANG_BASIC_ADDRESSSPACES_H

#include <cassert>

namespace clang {

/// Defines the address space values used by the address space qualifier
/// of QualType.
///
enum class LangAS : unsigned {
  // The default value 0 is the value used in QualType for the situation
  // where there is no address space qualifier.
  Default = 0,

  // OpenCL specific address spaces.
  // In OpenCL each l-value must have certain non-default address space, each
  // r-value must have no address space (i.e. the default address space). The
  // pointee of a pointer must have non-default address space.
  opencl_global,
  opencl_local,
  opencl_constant,
  opencl_private,
  opencl_generic,
  opencl_global_device,
  opencl_global_host,

  // CUDA specific address spaces.
  cuda_device,
  cuda_constant,
  cuda_shared,

  // SYCL specific address spaces.
  sycl_global,
  sycl_global_device,
  sycl_global_host,
  sycl_local,
  sycl_private,

  // Pointer size and extension address spaces.
  ptr32_sptr,
  ptr32_uptr,
  ptr64,

  // HLSL specific address spaces.
  hlsl_groupshared,
  hlsl_constant,
  hlsl_private,
  hlsl_device,
  hlsl_input,
  hlsl_output,
  hlsl_push_constant,

  // Wasm specific address spaces.
  wasm_funcref,

  // This denotes the count of language-specific address spaces and also
  // the offset added to the target-specific address spaces, which are usually
  // specified by address space attributes __attribute__(address_space(n))).
  FirstTargetAddressSpace
};

/// The type of a lookup table which maps from language-specific address spaces
/// to target-specific ones.
using LangASMap = unsigned[(unsigned)LangAS::FirstTargetAddressSpace];

/// \return whether \p AS is a target-specific address space rather than a
/// clang AST address space
inline bool isTargetAddressSpace(LangAS AS) {
  return (unsigned)AS >= (unsigned)LangAS::FirstTargetAddressSpace;
}

inline unsigned toTargetAddressSpace(LangAS AS) {
  assert(isTargetAddressSpace(AS));
  return (unsigned)AS - (unsigned)LangAS::FirstTargetAddressSpace;
}

inline LangAS getLangASFromTargetAS(unsigned TargetAS) {
  return static_cast<LangAS>((TargetAS) +
                             (unsigned)LangAS::FirstTargetAddressSpace);
}

inline bool isPtrSizeAddressSpace(LangAS AS) {
  return (AS == LangAS::ptr32_sptr || AS == LangAS::ptr32_uptr ||
          AS == LangAS::ptr64);
}

/// Target address space that the npu_uram attribute maps to. URAM is the
/// per-Core on-chip scratchpad of the NPU; it shares the RISC-V byte-addressed
/// space, so casts to and from it are no-ops at the machine level and the
/// address space exists purely to let the front end tell URAM and DRAM apart.
inline constexpr unsigned NPUURAMTargetAddressSpace = 1;

/// Size in bytes of the URAM that belongs to one Core. A URAM address is an
/// offset inside that window, so this is also the exclusive upper bound of a
/// valid URAM address.
inline constexpr unsigned long long NPUURAMWindowByteSize = 0x180000; // 1.5 MiB

inline LangAS getNPUURAMAddressSpace() {
  return getLangASFromTargetAS(NPUURAMTargetAddressSpace);
}

/// \return whether \p T is an object living in URAM, as opposed to a pointer
/// to URAM held in an ordinary variable.
inline bool isNPUURAMAddressSpace(LangAS AS) {
  return AS == getNPUURAMAddressSpace();
}

} // namespace clang

#endif // LLVM_CLANG_BASIC_ADDRESSSPACES_H
