//===----------------------------------------------------------------------===//
// RVT AsmParser direct implementation scaffold.
// Keep this file minimal and tablegen-first.
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringRef.h"

using namespace llvm;

namespace {

static unsigned getRVTDataTypeEncoding(StringRef TypeName) {
  // Keep this mapping aligned with npu-kernels/include/npu_instruction.hpp.
  if (TypeName == "fp32") return 0x00;
  if (TypeName == "bf16") return 0x02;
  if (TypeName == "fp16") return 0x03;
  if (TypeName == "e4m3") return 0x04;
  if (TypeName == "e5m2") return 0x05;
  if (TypeName == "s32")  return 0x20;
  if (TypeName == "u32")  return 0x21;
  if (TypeName == "s16")  return 0x22;
  if (TypeName == "u16")  return 0x23;
  if (TypeName == "s8")   return 0x24;
  if (TypeName == "u8")   return 0x25;
  return 0;
}

static bool isRVTTransferKind(StringRef Kind) {
  return Kind == "u2u" || Kind == "u2d" || Kind == "d2u" || Kind == "d2d";
}

} // namespace

// Integration checklist:
// 1) Prefer TableGen-generated matcher path for tx.* mnemonics.
// 2) Use this file only for parser extensions that cannot be expressed in TableGen.
// 3) Keep dtype/transfer mapping consistent with npu-kernels/include/npu_instruction.hpp.
