//===- Dayan.h - Dayan source splitting support -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DAYAN_DAYAN_H
#define LLVM_CLANG_DAYAN_DAYAN_H

#include <cstdint>
#include <string>
#include <vector>

namespace clang::dayan {

struct SplitOptions {
  std::string InputPath;
  std::string WorkingDirectory;
  std::string CompilerResourceDir;
  std::string CompilerAttributesHeader;
  std::string DeviceTriple;
  std::string DeviceArch;
  std::string DeviceAPIInclude;
  std::string HostInclude;
  std::vector<std::string> CommonArgs;
  std::vector<std::string> DeviceArgs;
};

struct PackageArgumentInfo {
  uint32_t Kind = 0;
  uint32_t Size = 0;
  uint32_t Alignment = 0;
  uint32_t Access = 0;
};

struct PackageKernelInfo {
  std::string Name;
  std::string EntrySymbol;
  uint32_t MaxChildThreads = 0;
  uint32_t MaxTasksPerCore = 0;
  uint32_t Flags = 0;
  uint32_t LsuTcsrCount = 0;
  uint32_t MatTcsrCount = 0;
  uint32_t VecTcsrCount = 0;
  uint32_t MacroTcsrCount = 0;
  uint64_t AtomicCacheSize = 0;
  uint64_t UramSize = 0;
  std::vector<PackageArgumentInfo> Arguments;
};

struct SplitOutput {
  std::string Host;
  std::string Device;
  std::vector<PackageKernelInfo> Kernels;
};

/// Parse and split one Dayan translation unit into Host and Device sources.
/// Diagnostics are emitted through Clang's normal diagnostic machinery.
bool splitSource(const SplitOptions &Options, SplitOutput &Output);

} // namespace clang::dayan

#endif // LLVM_CLANG_DAYAN_DAYAN_H
