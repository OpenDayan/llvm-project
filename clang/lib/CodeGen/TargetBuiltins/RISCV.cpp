//===-------- RISCV.cpp - Emit LLVM Code for builtins ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Builtin calls as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "clang/Basic/AddressSpaces.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/IntrinsicsRISCV.h"
#include "llvm/TargetParser/RISCVISAInfo.h"
#include "llvm/TargetParser/RISCVTargetParser.h"

using namespace clang;
using namespace CodeGen;
using namespace llvm;

// The 0th bit simulates the `vta` of RVV
// The 1st bit simulates the `vma` of RVV
static constexpr unsigned RVV_VTA = 0x1;
static constexpr unsigned RVV_VMA = 0x2;

// RISC-V Vector builtin helper functions are marked NOINLINE to prevent
// excessive inlining in CodeGenFunction::EmitRISCVBuiltinExpr's large switch
// statement, which would significantly increase compilation time.
static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVVLEFFBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                    ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                    Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                    int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 3> IntrinsicTypes;
  if (IsMasked) {
    // Move mask to right before vl.
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
    if ((PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA))
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
    IntrinsicTypes = {ResultType, Ops[4]->getType(), Ops[2]->getType()};
  } else {
    if (PolicyAttrs & RVV_VTA)
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
    IntrinsicTypes = {ResultType, Ops[3]->getType(), Ops[1]->getType()};
  }
  Value *NewVL = Ops[2];
  Ops.erase(Ops.begin() + 2);
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  llvm::Value *LoadValue = Builder.CreateCall(F, Ops, "");
  llvm::Value *V = Builder.CreateExtractValue(LoadValue, {0});
  // Store new_vl.
  clang::CharUnits Align;
  if (IsMasked)
    Align = CGM.getNaturalPointeeTypeAlignment(
        E->getArg(E->getNumArgs() - 2)->getType());
  else
    Align = CGM.getNaturalPointeeTypeAlignment(E->getArg(1)->getType());
  llvm::Value *Val = Builder.CreateExtractValue(LoadValue, {1});
  Builder.CreateStore(Val, Address(NewVL, Val->getType(), Align));
  return V;
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVVSSEBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                   ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                   Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                   int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 3> IntrinsicTypes;
  if (IsMasked) {
    // Builtin: (mask, ptr, stride, value, vl). Intrinsic: (value, ptr, stride,
    // mask, vl)
    std::swap(Ops[0], Ops[3]);
  } else {
    // Builtin: (ptr, stride, value, vl). Intrinsic: (value, ptr, stride, vl)
    std::rotate(Ops.begin(), Ops.begin() + 2, Ops.begin() + 3);
  }
  if (IsMasked)
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[4]->getType()};
  else
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[3]->getType()};
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVIndexedStoreBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                           ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                           Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                           int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 4> IntrinsicTypes;
  if (IsMasked) {
    // Builtin: (mask, ptr, index, value, vl).
    // Intrinsic: (value, ptr, index, mask, vl)
    std::swap(Ops[0], Ops[3]);
  } else {
    // Builtin: (ptr, index, value, vl).
    // Intrinsic: (value, ptr, index, vl)
    std::rotate(Ops.begin(), Ops.begin() + 2, Ops.begin() + 3);
  }
  if (IsMasked)
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                      Ops[4]->getType()};
  else
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                      Ops[3]->getType()};
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVPseudoUnaryBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                          ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                          Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                          int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 3> IntrinsicTypes;
  if (IsMasked) {
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
    if ((PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA))
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  } else {
    if (PolicyAttrs & RVV_VTA)
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  }
  auto ElemTy = cast<llvm::VectorType>(ResultType)->getElementType();
  Ops.insert(Ops.begin() + 2, llvm::Constant::getNullValue(ElemTy));
  if (IsMasked) {
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
    // maskedoff, op1, op2, mask, vl, policy
    IntrinsicTypes = {ResultType, ElemTy, Ops[4]->getType()};
  } else {
    // passthru, op1, op2, vl
    IntrinsicTypes = {ResultType, ElemTy, Ops[3]->getType()};
  }
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVPseudoVNotBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                         ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                         Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                         int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 3> IntrinsicTypes;
  if (IsMasked) {
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
    if ((PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA))
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  } else {
    if (PolicyAttrs & RVV_VTA)
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  }
  auto ElemTy = cast<llvm::VectorType>(ResultType)->getElementType();
  Ops.insert(Ops.begin() + 2, llvm::Constant::getAllOnesValue(ElemTy));
  if (IsMasked) {
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
    // maskedoff, op1, po2, mask, vl, policy
    IntrinsicTypes = {ResultType, ElemTy, Ops[4]->getType()};
  } else {
    // passthru, op1, op2, vl
    IntrinsicTypes = {ResultType, ElemTy, Ops[3]->getType()};
  }
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVPseudoMaskBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                         ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                         Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                         int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 3> IntrinsicTypes;
  // op1, vl
  IntrinsicTypes = {ResultType, Ops[1]->getType()};
  Ops.insert(Ops.begin() + 1, Ops[0]);
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVPseudoVFUnaryBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                            ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                            Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                            int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 3> IntrinsicTypes;
  if (IsMasked) {
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
    if ((PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA))
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
    Ops.insert(Ops.begin() + 2, Ops[1]);
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
    // maskedoff, op1, op2, mask, vl
    IntrinsicTypes = {ResultType, Ops[2]->getType(), Ops.back()->getType()};
  } else {
    if (PolicyAttrs & RVV_VTA)
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
    // op1, po2, vl
    IntrinsicTypes = {ResultType, Ops[1]->getType(), Ops[2]->getType()};
    Ops.insert(Ops.begin() + 2, Ops[1]);
  }
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVPseudoVWCVTBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                          ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                          Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                          int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 4> IntrinsicTypes;
  if (IsMasked) {
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
    if ((PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA))
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  } else {
    if (PolicyAttrs & RVV_VTA)
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  }
  auto ElemTy = cast<llvm::VectorType>(Ops[1]->getType())->getElementType();
  Ops.insert(Ops.begin() + 2, llvm::Constant::getNullValue(ElemTy));
  if (IsMasked) {
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
    // maskedoff, op1, op2, mask, vl, policy
    IntrinsicTypes = {ResultType, Ops[1]->getType(), ElemTy, Ops[4]->getType()};
  } else {
    // passtru, op1, op2, vl
    IntrinsicTypes = {ResultType, Ops[1]->getType(), ElemTy, Ops[3]->getType()};
  }
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVPseudoVNCVTBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                          ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                          Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                          int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 4> IntrinsicTypes;
  if (IsMasked) {
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
    if ((PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA))
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  } else {
    if (PolicyAttrs & RVV_VTA)
      Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  }
  Ops.insert(Ops.begin() + 2,
             llvm::Constant::getNullValue(Ops.back()->getType()));
  if (IsMasked) {
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
    // maskedoff, op1, xlen, mask, vl
    IntrinsicTypes = {ResultType, Ops[1]->getType(), Ops[4]->getType(),
                      Ops[4]->getType()};
  } else {
    // passthru, op1, xlen, vl
    IntrinsicTypes = {ResultType, Ops[1]->getType(), Ops[3]->getType(),
                      Ops[3]->getType()};
  }
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVVlenbBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                    ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                    Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                    int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  LLVMContext &Context = CGM.getLLVMContext();
  llvm::MDBuilder MDHelper(Context);
  llvm::Metadata *OpsMD[] = {llvm::MDString::get(Context, "vlenb")};
  llvm::MDNode *RegName = llvm::MDNode::get(Context, OpsMD);
  llvm::Value *Metadata = llvm::MetadataAsValue::get(Context, RegName);
  llvm::Function *F =
      CGM.getIntrinsic(llvm::Intrinsic::read_register, {CGF->SizeTy});
  return Builder.CreateCall(F, Metadata);
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVVsetvliBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                      ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                      Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                      int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::Function *F = CGM.getIntrinsic(ID, {ResultType});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVVSEMaskBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                      ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                      Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                      int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 3> IntrinsicTypes;
  if (IsMasked) {
    // Builtin: (mask, ptr, value, vl).
    // Intrinsic: (value, ptr, mask, vl)
    std::swap(Ops[0], Ops[2]);
  } else {
    // Builtin: (ptr, value, vl).
    // Intrinsic: (value, ptr, vl)
    std::swap(Ops[0], Ops[1]);
  }
  if (IsMasked)
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[3]->getType()};
  else
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()};
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVUnitStridedSegLoadTupleBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked, unsigned SegInstSEW) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 4> IntrinsicTypes;
  bool NoPassthru =
      (IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) |
      (!IsMasked && (PolicyAttrs & RVV_VTA));
  unsigned Offset = IsMasked ? NoPassthru ? 1 : 2 : NoPassthru ? 0 : 1;
  if (IsMasked)
    IntrinsicTypes = {ResultType, Ops[Offset]->getType(), Ops[0]->getType(),
                      Ops.back()->getType()};
  else
    IntrinsicTypes = {ResultType, Ops[Offset]->getType(),
                      Ops.back()->getType()};
  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
  if (NoPassthru)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
  Ops.push_back(ConstantInt::get(Ops.back()->getType(), SegInstSEW));
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  llvm::Value *LoadValue = Builder.CreateCall(F, Ops, "");
  if (ReturnValue.isNull())
    return LoadValue;
  return Builder.CreateStore(LoadValue, ReturnValue.getValue());
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVUnitStridedSegStoreTupleBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked, unsigned SegInstSEW) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 4> IntrinsicTypes;
  // Masked
  // Builtin: (mask, ptr, v_tuple, vl)
  // Intrinsic: (tuple, ptr, mask, vl, SegInstSEW)
  // Unmasked
  // Builtin: (ptr, v_tuple, vl)
  // Intrinsic: (tuple, ptr, vl, SegInstSEW)
  if (IsMasked)
    std::swap(Ops[0], Ops[2]);
  else
    std::swap(Ops[0], Ops[1]);
  Ops.push_back(ConstantInt::get(Ops.back()->getType(), SegInstSEW));
  if (IsMasked)
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                      Ops[3]->getType()};
  else
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()};
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVUnitStridedSegLoadFFTupleBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked, unsigned SegInstSEW) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 4> IntrinsicTypes;
  bool NoPassthru =
      (IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) |
      (!IsMasked && (PolicyAttrs & RVV_VTA));
  unsigned Offset = IsMasked ? NoPassthru ? 1 : 2 : NoPassthru ? 0 : 1;
  if (IsMasked)
    IntrinsicTypes = {ResultType, Ops.back()->getType(), Ops[Offset]->getType(),
                      Ops[0]->getType()};
  else
    IntrinsicTypes = {ResultType, Ops.back()->getType(),
                      Ops[Offset]->getType()};
  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
  if (NoPassthru)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
  Ops.push_back(ConstantInt::get(Ops.back()->getType(), SegInstSEW));
  Value *NewVL = Ops[2];
  Ops.erase(Ops.begin() + 2);
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  llvm::Value *LoadValue = Builder.CreateCall(F, Ops, "");
  // Get alignment from the new vl operand
  clang::CharUnits Align =
      CGM.getNaturalPointeeTypeAlignment(E->getArg(Offset + 1)->getType());
  llvm::Value *ReturnTuple = Builder.CreateExtractValue(LoadValue, 0);
  // Store new_vl
  llvm::Value *V = Builder.CreateExtractValue(LoadValue, 1);
  Builder.CreateStore(V, Address(NewVL, V->getType(), Align));
  if (ReturnValue.isNull())
    return ReturnTuple;
  return Builder.CreateStore(ReturnTuple, ReturnValue.getValue());
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVStridedSegLoadTupleBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked, unsigned SegInstSEW) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 4> IntrinsicTypes;
  bool NoPassthru =
      (IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) |
      (!IsMasked && (PolicyAttrs & RVV_VTA));
  unsigned Offset = IsMasked ? NoPassthru ? 1 : 2 : NoPassthru ? 0 : 1;
  if (IsMasked)
    IntrinsicTypes = {ResultType, Ops[Offset]->getType(), Ops.back()->getType(),
                      Ops[0]->getType()};
  else
    IntrinsicTypes = {ResultType, Ops[Offset]->getType(),
                      Ops.back()->getType()};
  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
  if (NoPassthru)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));
  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
  Ops.push_back(ConstantInt::get(Ops.back()->getType(), SegInstSEW));
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  llvm::Value *LoadValue = Builder.CreateCall(F, Ops, "");
  if (ReturnValue.isNull())
    return LoadValue;
  return Builder.CreateStore(LoadValue, ReturnValue.getValue());
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVStridedSegStoreTupleBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked, unsigned SegInstSEW) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 4> IntrinsicTypes;
  // Masked
  // Builtin: (mask, ptr, stride, v_tuple, vl)
  // Intrinsic: (tuple, ptr, stride, mask, vl, SegInstSEW)
  // Unmasked
  // Builtin: (ptr, stride, v_tuple, vl)
  // Intrinsic: (tuple, ptr, stride, vl, SegInstSEW)
  if (IsMasked)
    std::swap(Ops[0], Ops[3]);
  else
    std::rotate(Ops.begin(), Ops.begin() + 2, Ops.begin() + 3);
  Ops.push_back(ConstantInt::get(Ops.back()->getType(), SegInstSEW));
  if (IsMasked)
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[4]->getType(),
                      Ops[3]->getType()};
  else
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[3]->getType()};
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVAveragingBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                        ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                        Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                        int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  // LLVM intrinsic
  // Unmasked: (passthru, op0, op1, round_mode, vl)
  // Masked:   (passthru, vector_in, vector_in/scalar_in, mask, vxrm, vl,
  // policy)

  bool HasMaskedOff =
      !((IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) ||
        (!IsMasked && PolicyAttrs & RVV_VTA));

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 2);

  if (!HasMaskedOff)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));

  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));

  llvm::Function *F = CGM.getIntrinsic(
      ID, {ResultType, Ops[2]->getType(), Ops.back()->getType()});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVNarrowingClipBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                            ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                            Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                            int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  // LLVM intrinsic
  // Unmasked: (passthru, op0, op1, round_mode, vl)
  // Masked:   (passthru, vector_in, vector_in/scalar_in, mask, vxrm, vl,
  // policy)

  bool HasMaskedOff =
      !((IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) ||
        (!IsMasked && PolicyAttrs & RVV_VTA));

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 2);

  if (!HasMaskedOff)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));

  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));

  llvm::Function *F =
      CGM.getIntrinsic(ID, {ResultType, Ops[1]->getType(), Ops[2]->getType(),
                            Ops.back()->getType()});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVFloatingPointBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                            ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                            Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                            int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  // LLVM intrinsic
  // Unmasked: (passthru, op0, op1, round_mode, vl)
  // Masked:   (passthru, vector_in, vector_in/scalar_in, mask, frm, vl, policy)

  bool HasMaskedOff =
      !((IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) ||
        (!IsMasked && PolicyAttrs & RVV_VTA));
  bool HasRoundModeOp =
      IsMasked ? (HasMaskedOff ? Ops.size() == 6 : Ops.size() == 5)
               : (HasMaskedOff ? Ops.size() == 5 : Ops.size() == 4);

  if (!HasRoundModeOp)
    Ops.insert(Ops.end() - 1,
               ConstantInt::get(Ops.back()->getType(), 7)); // frm

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 2);

  if (!HasMaskedOff)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));

  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));

  llvm::Function *F = CGM.getIntrinsic(
      ID, {ResultType, Ops[2]->getType(), Ops.back()->getType()});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVWideningFloatingPointBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  // LLVM intrinsic
  // Unmasked: (passthru, op0, op1, round_mode, vl)
  // Masked:   (passthru, vector_in, vector_in/scalar_in, mask, frm, vl, policy)

  bool HasMaskedOff =
      !((IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) ||
        (!IsMasked && PolicyAttrs & RVV_VTA));
  bool HasRoundModeOp =
      IsMasked ? (HasMaskedOff ? Ops.size() == 6 : Ops.size() == 5)
               : (HasMaskedOff ? Ops.size() == 5 : Ops.size() == 4);

  if (!HasRoundModeOp)
    Ops.insert(Ops.end() - 1,
               ConstantInt::get(Ops.back()->getType(), 7)); // frm

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 2);

  if (!HasMaskedOff)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));

  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));

  llvm::Function *F =
      CGM.getIntrinsic(ID, {ResultType, Ops[1]->getType(), Ops[2]->getType(),
                            Ops.back()->getType()});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVIndexedSegLoadTupleBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked, unsigned SegInstSEW) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 5> IntrinsicTypes;

  bool NoPassthru =
      (IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) |
      (!IsMasked && (PolicyAttrs & RVV_VTA));

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 1);
  if (NoPassthru)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));

  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));
  Ops.push_back(ConstantInt::get(Ops.back()->getType(), SegInstSEW));

  if (IsMasked)
    IntrinsicTypes = {ResultType, Ops[1]->getType(), Ops[2]->getType(),
                      Ops[3]->getType(), Ops[4]->getType()};
  else
    IntrinsicTypes = {ResultType, Ops[1]->getType(), Ops[2]->getType(),
                      Ops[3]->getType()};
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  llvm::Value *LoadValue = Builder.CreateCall(F, Ops, "");

  if (ReturnValue.isNull())
    return LoadValue;
  return Builder.CreateStore(LoadValue, ReturnValue.getValue());
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVIndexedSegStoreTupleBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked, unsigned SegInstSEW) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 5> IntrinsicTypes;
  // Masked
  // Builtin: (mask, ptr, index, v_tuple, vl)
  // Intrinsic: (tuple, ptr, index, mask, vl, SegInstSEW)
  // Unmasked
  // Builtin: (ptr, index, v_tuple, vl)
  // Intrinsic: (tuple, ptr, index, vl, SegInstSEW)

  if (IsMasked)
    std::swap(Ops[0], Ops[3]);
  else
    std::rotate(Ops.begin(), Ops.begin() + 2, Ops.begin() + 3);

  Ops.push_back(ConstantInt::get(Ops.back()->getType(), SegInstSEW));

  if (IsMasked)
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                      Ops[3]->getType(), Ops[4]->getType()};
  else
    IntrinsicTypes = {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                      Ops[3]->getType()};
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVFMABuiltin(CodeGenFunction *CGF, const CallExpr *E,
                  ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                  Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                  int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  // LLVM intrinsic
  // Unmasked: (vector_in, vector_in/scalar_in, vector_in, round_mode,
  //            vl, policy)
  // Masked:   (vector_in, vector_in/scalar_in, vector_in, mask, frm,
  //            vl, policy)

  bool HasRoundModeOp = IsMasked ? Ops.size() == 6 : Ops.size() == 5;

  if (!HasRoundModeOp)
    Ops.insert(Ops.end() - 1,
               ConstantInt::get(Ops.back()->getType(), 7)); // frm

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 2);

  Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));

  llvm::Function *F = CGM.getIntrinsic(
      ID, {ResultType, Ops[1]->getType(), Ops.back()->getType()});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVWideningFMABuiltin(CodeGenFunction *CGF, const CallExpr *E,
                          ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                          Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                          int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  // LLVM intrinsic
  // Unmasked: (vector_in, vector_in/scalar_in, vector_in, round_mode, vl,
  // policy) Masked:   (vector_in, vector_in/scalar_in, vector_in, mask, frm,
  // vl, policy)

  bool HasRoundModeOp = IsMasked ? Ops.size() == 6 : Ops.size() == 5;

  if (!HasRoundModeOp)
    Ops.insert(Ops.end() - 1,
               ConstantInt::get(Ops.back()->getType(), 7)); // frm

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.begin() + 4);

  Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));

  llvm::Function *F =
      CGM.getIntrinsic(ID, {ResultType, Ops[1]->getType(), Ops[2]->getType(),
                            Ops.back()->getType()});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVFloatingUnaryBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                            ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                            Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                            int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  llvm::SmallVector<llvm::Type *, 3> IntrinsicTypes;
  // LLVM intrinsic
  // Unmasked: (passthru, op0, round_mode, vl)
  // Masked:   (passthru, op0, mask, frm, vl, policy)

  bool HasMaskedOff =
      !((IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) ||
        (!IsMasked && PolicyAttrs & RVV_VTA));
  bool HasRoundModeOp =
      IsMasked ? (HasMaskedOff ? Ops.size() == 5 : Ops.size() == 4)
               : (HasMaskedOff ? Ops.size() == 4 : Ops.size() == 3);

  if (!HasRoundModeOp)
    Ops.insert(Ops.end() - 1,
               ConstantInt::get(Ops.back()->getType(), 7)); // frm

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 2);

  if (!HasMaskedOff)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));

  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));

  IntrinsicTypes = {ResultType, Ops.back()->getType()};
  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVFloatingConvBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                           ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                           Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                           int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  // LLVM intrinsic
  // Unmasked: (passthru, op0, frm, vl)
  // Masked:   (passthru, op0, mask, frm, vl, policy)
  bool HasMaskedOff =
      !((IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) ||
        (!IsMasked && PolicyAttrs & RVV_VTA));
  bool HasRoundModeOp =
      IsMasked ? (HasMaskedOff ? Ops.size() == 5 : Ops.size() == 4)
               : (HasMaskedOff ? Ops.size() == 4 : Ops.size() == 3);

  if (!HasRoundModeOp)
    Ops.insert(Ops.end() - 1,
               ConstantInt::get(Ops.back()->getType(), 7)); // frm

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 2);

  if (!HasMaskedOff)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));

  if (IsMasked)
    Ops.push_back(ConstantInt::get(Ops.back()->getType(), PolicyAttrs));

  llvm::Function *F = CGM.getIntrinsic(
      ID, {ResultType, Ops[1]->getType(), Ops.back()->getType()});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *emitRVVFloatingReductionBuiltin(
    CodeGenFunction *CGF, const CallExpr *E, ReturnValueSlot ReturnValue,
    llvm::Type *ResultType, Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
    int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;
  // LLVM intrinsic
  // Unmasked: (passthru, op0, op1, round_mode, vl)
  // Masked:   (passthru, vector_in, vector_in/scalar_in, mask, frm, vl, policy)

  bool HasMaskedOff =
      !((IsMasked && (PolicyAttrs & RVV_VTA) && (PolicyAttrs & RVV_VMA)) ||
        (!IsMasked && PolicyAttrs & RVV_VTA));
  bool HasRoundModeOp =
      IsMasked ? (HasMaskedOff ? Ops.size() == 6 : Ops.size() == 5)
               : (HasMaskedOff ? Ops.size() == 5 : Ops.size() == 4);

  if (!HasRoundModeOp)
    Ops.insert(Ops.end() - 1,
               ConstantInt::get(Ops.back()->getType(), 7)); // frm

  if (IsMasked)
    std::rotate(Ops.begin(), Ops.begin() + 1, Ops.end() - 2);

  if (!HasMaskedOff)
    Ops.insert(Ops.begin(), llvm::PoisonValue::get(ResultType));

  llvm::Function *F = CGM.getIntrinsic(
      ID, {ResultType, Ops[1]->getType(), Ops.back()->getType()});
  return Builder.CreateCall(F, Ops, "");
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVReinterpretBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                          ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                          Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                          int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto &CGM = CGF->CGM;

  if (ResultType->isIntOrIntVectorTy(1) ||
      Ops[0]->getType()->isIntOrIntVectorTy(1)) {
    assert(isa<ScalableVectorType>(ResultType) &&
           isa<ScalableVectorType>(Ops[0]->getType()));

    LLVMContext &Context = CGM.getLLVMContext();
    ScalableVectorType *Boolean64Ty =
        ScalableVectorType::get(llvm::Type::getInt1Ty(Context), 64);

    if (ResultType->isIntOrIntVectorTy(1)) {
      // Casting from m1 vector integer -> vector boolean
      // Ex: <vscale x 8 x i8>
      //     --(bitcast)--------> <vscale x 64 x i1>
      //     --(vector_extract)-> <vscale x  8 x i1>
      llvm::Value *BitCast = Builder.CreateBitCast(Ops[0], Boolean64Ty);
      return Builder.CreateExtractVector(ResultType, BitCast,
                                         ConstantInt::get(CGF->Int64Ty, 0));
    } else {
      // Casting from vector boolean -> m1 vector integer
      // Ex: <vscale x  1 x i1>
      //       --(vector_insert)-> <vscale x 64 x i1>
      //       --(bitcast)-------> <vscale x  8 x i8>
      llvm::Value *Boolean64Val = Builder.CreateInsertVector(
          Boolean64Ty, llvm::PoisonValue::get(Boolean64Ty), Ops[0],
          ConstantInt::get(CGF->Int64Ty, 0));
      return Builder.CreateBitCast(Boolean64Val, ResultType);
    }
  }
  return Builder.CreateBitCast(Ops[0], ResultType);
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVGetBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                  ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                  Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                  int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  auto *VecTy = cast<ScalableVectorType>(ResultType);
  if (auto *OpVecTy = dyn_cast<ScalableVectorType>(Ops[0]->getType())) {
    unsigned MaxIndex =
        OpVecTy->getMinNumElements() / VecTy->getMinNumElements();
    assert(isPowerOf2_32(MaxIndex));
    // Mask to only valid indices.
    Ops[1] = Builder.CreateZExt(Ops[1], Builder.getInt64Ty());
    Ops[1] = Builder.CreateAnd(Ops[1], MaxIndex - 1);
    Ops[1] =
        Builder.CreateMul(Ops[1], ConstantInt::get(Ops[1]->getType(),
                                                   VecTy->getMinNumElements()));
    return Builder.CreateExtractVector(ResultType, Ops[0], Ops[1]);
  }

  return Builder.CreateIntrinsic(
      Intrinsic::riscv_tuple_extract, {ResultType, Ops[0]->getType()},
      {Ops[0], Builder.CreateTrunc(Ops[1], Builder.getInt32Ty())});
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVSetBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                  ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                  Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                  int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  if (auto *ResVecTy = dyn_cast<ScalableVectorType>(ResultType)) {
    auto *VecTy = cast<ScalableVectorType>(Ops[2]->getType());
    unsigned MaxIndex =
        ResVecTy->getMinNumElements() / VecTy->getMinNumElements();
    assert(isPowerOf2_32(MaxIndex));
    // Mask to only valid indices.
    Ops[1] = Builder.CreateZExt(Ops[1], Builder.getInt64Ty());
    Ops[1] = Builder.CreateAnd(Ops[1], MaxIndex - 1);
    Ops[1] =
        Builder.CreateMul(Ops[1], ConstantInt::get(Ops[1]->getType(),
                                                   VecTy->getMinNumElements()));
    return Builder.CreateInsertVector(ResultType, Ops[0], Ops[2], Ops[1]);
  }

  return Builder.CreateIntrinsic(
      Intrinsic::riscv_tuple_insert, {ResultType, Ops[2]->getType()},
      {Ops[0], Ops[2], Builder.CreateTrunc(Ops[1], Builder.getInt32Ty())});
}

static LLVM_ATTRIBUTE_NOINLINE Value *
emitRVVCreateBuiltin(CodeGenFunction *CGF, const CallExpr *E,
                     ReturnValueSlot ReturnValue, llvm::Type *ResultType,
                     Intrinsic::ID ID, SmallVectorImpl<Value *> &Ops,
                     int PolicyAttrs, bool IsMasked) {
  auto &Builder = CGF->Builder;
  llvm::Value *ReturnVector = llvm::PoisonValue::get(ResultType);
  auto *VecTy = cast<ScalableVectorType>(Ops[0]->getType());
  for (unsigned I = 0, N = Ops.size(); I < N; ++I) {
    if (isa<ScalableVectorType>(ResultType)) {
      llvm::Value *Idx = ConstantInt::get(Builder.getInt64Ty(),
                                          VecTy->getMinNumElements() * I);
      ReturnVector =
          Builder.CreateInsertVector(ResultType, ReturnVector, Ops[I], Idx);
    } else {
      llvm::Value *Idx = ConstantInt::get(Builder.getInt32Ty(), I);
      ReturnVector = Builder.CreateIntrinsic(Intrinsic::riscv_tuple_insert,
                                             {ResultType, Ops[I]->getType()},
                                             {ReturnVector, Ops[I], Idx});
    }
  }
  return ReturnVector;
}

Value *CodeGenFunction::EmitRISCVCpuInit() {
  llvm::FunctionType *FTy = llvm::FunctionType::get(VoidTy, {VoidPtrTy}, false);
  llvm::FunctionCallee Func =
      CGM.CreateRuntimeFunction(FTy, "__init_riscv_feature_bits");
  auto *CalleeGV = cast<llvm::GlobalValue>(Func.getCallee());
  CalleeGV->setDSOLocal(true);
  CalleeGV->setDLLStorageClass(llvm::GlobalValue::DefaultStorageClass);
  return Builder.CreateCall(Func, {llvm::ConstantPointerNull::get(VoidPtrTy)});
}

Value *CodeGenFunction::EmitRISCVCpuSupports(const CallExpr *E) {

  const Expr *FeatureExpr = E->getArg(0)->IgnoreParenCasts();
  StringRef FeatureStr = cast<StringLiteral>(FeatureExpr)->getString();
  if (!getContext().getTargetInfo().validateCpuSupports(FeatureStr))
    return Builder.getFalse();

  return EmitRISCVCpuSupports(ArrayRef<StringRef>(FeatureStr));
}

static Value *loadRISCVFeatureBits(unsigned Index, CGBuilderTy &Builder,
                                   CodeGenModule &CGM) {
  llvm::Type *Int32Ty = Builder.getInt32Ty();
  llvm::Type *Int64Ty = Builder.getInt64Ty();
  llvm::ArrayType *ArrayOfInt64Ty =
      llvm::ArrayType::get(Int64Ty, llvm::RISCVISAInfo::FeatureBitSize);
  llvm::Type *StructTy = llvm::StructType::get(Int32Ty, ArrayOfInt64Ty);
  llvm::Constant *RISCVFeaturesBits =
      CGM.CreateRuntimeVariable(StructTy, "__riscv_feature_bits");
  cast<llvm::GlobalValue>(RISCVFeaturesBits)->setDSOLocal(true);
  Value *IndexVal = llvm::ConstantInt::get(Int32Ty, Index);
  llvm::Value *GEPIndices[] = {Builder.getInt32(0), Builder.getInt32(1),
                               IndexVal};
  Value *Ptr =
      Builder.CreateInBoundsGEP(StructTy, RISCVFeaturesBits, GEPIndices);
  Value *FeaturesBit =
      Builder.CreateAlignedLoad(Int64Ty, Ptr, CharUnits::fromQuantity(8));
  return FeaturesBit;
}

Value *CodeGenFunction::EmitRISCVCpuSupports(ArrayRef<StringRef> FeaturesStrs) {
  const unsigned RISCVFeatureLength = llvm::RISCVISAInfo::FeatureBitSize;
  uint64_t RequireBitMasks[RISCVFeatureLength] = {0};

  for (auto Feat : FeaturesStrs) {
    auto [GroupID, BitPos] = RISCVISAInfo::getRISCVFeaturesBitsInfo(Feat);

    // If there isn't BitPos for this feature, skip this version.
    // It also report the warning to user during compilation.
    if (BitPos == -1)
      return Builder.getFalse();

    RequireBitMasks[GroupID] |= (1ULL << BitPos);
  }

  Value *Result = nullptr;
  for (unsigned Idx = 0; Idx < RISCVFeatureLength; Idx++) {
    if (RequireBitMasks[Idx] == 0)
      continue;

    Value *Mask = Builder.getInt64(RequireBitMasks[Idx]);
    Value *Bitset =
        Builder.CreateAnd(loadRISCVFeatureBits(Idx, Builder, CGM), Mask);
    Value *CmpV = Builder.CreateICmpEQ(Bitset, Mask);
    Result = (!Result) ? CmpV : Builder.CreateAnd(Result, CmpV);
  }

  assert(Result && "Should have value here.");

  return Result;
}

Value *CodeGenFunction::EmitRISCVCpuIs(const CallExpr *E) {
  const Expr *CPUExpr = E->getArg(0)->IgnoreParenCasts();
  StringRef CPUStr = cast<clang::StringLiteral>(CPUExpr)->getString();
  return EmitRISCVCpuIs(CPUStr);
}

Value *CodeGenFunction::EmitRISCVCpuIs(StringRef CPUStr) {
  llvm::Type *Int32Ty = Builder.getInt32Ty();
  llvm::Type *Int64Ty = Builder.getInt64Ty();
  llvm::StructType *StructTy = llvm::StructType::get(Int32Ty, Int64Ty, Int64Ty);
  llvm::Constant *RISCVCPUModel =
      CGM.CreateRuntimeVariable(StructTy, "__riscv_cpu_model");
  cast<llvm::GlobalValue>(RISCVCPUModel)->setDSOLocal(true);

  auto loadRISCVCPUID = [&](unsigned Index) {
    Value *Ptr = Builder.CreateStructGEP(StructTy, RISCVCPUModel, Index);
    Value *CPUID = Builder.CreateAlignedLoad(StructTy->getTypeAtIndex(Index),
                                             Ptr, llvm::MaybeAlign());
    return CPUID;
  };

  const llvm::RISCV::CPUModel Model = llvm::RISCV::getCPUModel(CPUStr);

  // Compare mvendorid.
  Value *VendorID = loadRISCVCPUID(0);
  Value *Result =
      Builder.CreateICmpEQ(VendorID, Builder.getInt32(Model.MVendorID));

  // Compare marchid.
  Value *ArchID = loadRISCVCPUID(1);
  Result = Builder.CreateAnd(
      Result, Builder.CreateICmpEQ(ArchID, Builder.getInt64(Model.MArchID)));

  // Compare mimpid.
  Value *ImpID = loadRISCVCPUID(2);
  Result = Builder.CreateAnd(
      Result, Builder.CreateICmpEQ(ImpID, Builder.getInt64(Model.MImpID)));

  return Result;
}

Value *CodeGenFunction::EmitRISCVBuiltinExpr(unsigned BuiltinID,
                                             const CallExpr *E,
                                             ReturnValueSlot ReturnValue) {

  if (BuiltinID == Builtin::BI__builtin_cpu_supports)
    return EmitRISCVCpuSupports(E);
  if (BuiltinID == Builtin::BI__builtin_cpu_init)
    return EmitRISCVCpuInit();
  if (BuiltinID == Builtin::BI__builtin_cpu_is)
    return EmitRISCVCpuIs(E);

  SmallVector<Value *, 4> Ops;
  llvm::Type *ResultType = ConvertType(E->getType());

  // Find out if any arguments are required to be integer constant expressions.
  unsigned ICEArguments = 0;
  ASTContext::GetBuiltinTypeError Error;
  getContext().GetBuiltinType(BuiltinID, Error, &ICEArguments);
  if (Error == ASTContext::GE_Missing_type) {
    // Vector intrinsics don't have a type string.
    assert(BuiltinID >= clang::RISCV::FirstRVVBuiltin &&
           BuiltinID <= clang::RISCV::LastRVVBuiltin);
    ICEArguments = 0;
    if (BuiltinID == RISCVVector::BI__builtin_rvv_vget_v ||
        BuiltinID == RISCVVector::BI__builtin_rvv_vset_v)
      ICEArguments = 1 << 1;
  } else {
    assert(Error == ASTContext::GE_None && "Unexpected error");
  }

  if (BuiltinID == RISCV::BI__builtin_riscv_ntl_load)
    ICEArguments |= (1 << 1);
  if (BuiltinID == RISCV::BI__builtin_riscv_ntl_store)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_lsu_tcsr_read)
    ICEArguments |= (1 << 0);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_lsu_tcsr_write)
    ICEArguments |= (1 << 1);
  switch (BuiltinID) {
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2u:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2u_g:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2u_s:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2u_sg:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2d:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2d_g:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2d_s:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2d_sg:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2u:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2u_g:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2u_s:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2u_sg:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2d:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2d_g:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2d_s:
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2d_sg:
    ICEArguments |= (1 << 2);
    break;
  default:
    break;
  }
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_mat_tcsr_read)
    ICEArguments |= (1 << 0);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_mat_tcsr_write)
    ICEArguments |= (1 << 1);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_mat_setptr)
    ICEArguments |= (1 << 3);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_mat_mm)
    ICEArguments |= (1 << 3);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_mat_mma)
    ICEArguments |= (1 << 3);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_mat_conv2d)
    ICEArguments |= (1 << 3);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_mat_conv2da)
    ICEArguments |= (1 << 3);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_vec_tcsr_read)
    ICEArguments |= (1 << 0);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_vec_tcsr_write)
    ICEArguments |= (1 << 1);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_vec_v_load)
    ICEArguments |= (1 << 1) | (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_vec_s_load)
    ICEArguments |= (1 << 1) | (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_vec_const_load)
    ICEArguments |= (1 << 1);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_vec_v_store)
    ICEArguments |= (1 << 1) | (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_vec_s_store)
    ICEArguments |= (1 << 1) | (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_vec_vv_int32_bitwise)
    ICEArguments |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
  switch (BuiltinID) {
  default:
    break;
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2v:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2v:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2v_broadcast32:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2v_broadcast32:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_g2v_broadcast32:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2s:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2s:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_v2u:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_v2sm:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_s2u:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_s2sm:
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_xor:
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_ror:
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_broadcast:
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_compress:
    ICEArguments |= (1 << 1) | (1 << 2);
    break;
  case RISCV::BI__builtin_riscv_tx_vec_lsu_g2s:
    ICEArguments |= (1 << 1);
    break;
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_rol:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_sll:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_sra:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_srl:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_add:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_sub:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_mulu24:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_mul24:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_maxu:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_minu:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_max:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_min:
  case RISCV::BI__builtin_riscv_tx_vec_v_int32_sll:
  case RISCV::BI__builtin_riscv_tx_vec_v_int32_sra:
  case RISCV::BI__builtin_riscv_tx_vec_v_int32_srl:
  case RISCV::BI__builtin_riscv_tx_vec_v_fp32_sfu:
  case RISCV::BI__builtin_riscv_tx_vec_cvt_fp32_int32:
  case RISCV::BI__builtin_riscv_tx_vec_cvt_s32_fp32:
  case RISCV::BI__builtin_riscv_tx_vec_cvt_fp32_fp16:
  case RISCV::BI__builtin_riscv_tx_vec_cvt_fp32_bf16:
  case RISCV::BI__builtin_riscv_tx_vec_cvt_fp16_fp32:
  case RISCV::BI__builtin_riscv_tx_vec_cvt_bf16_fp32:
  case RISCV::BI__builtin_riscv_tx_vec_cvt_mxfp8_bf16:
  case RISCV::BI__builtin_riscv_tx_vec_cvt_mxfp4_bf16:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_rol:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_sll:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_sra:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_srl:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_add:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_sub:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_mul:
  case RISCV::BI__builtin_riscv_tx_vec_s_iter_getdim:
  case RISCV::BI__builtin_riscv_tx_vec_s_bitwise_beq:
  case RISCV::BI__builtin_riscv_tx_vec_s_bitwise_bne:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_blt:
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_bge:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_s2v_broadcast32:
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_xor_dyn:
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_ror_dyn:
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_broadcast_dyn:
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_compress_dyn:
    ICEArguments |= (1 << 0) | (1 << 1) | (1 << 2);
    break;
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2v_mask:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2v_mask:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2v_mask_broadcast32:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2v_mask_broadcast32:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_g2v_mask_broadcast32:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_v2u_mask:
  case RISCV::BI__builtin_riscv_tx_vec_lsu_v2sm_mask:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse:
    ICEArguments |= (1 << 1) | (1 << 2) | (1 << 3);
    break;
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_add:
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_sub:
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_mul:
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_max:
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_min:
  case RISCV::BI__builtin_riscv_tx_vec_vv_mux2:
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_cmp:
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_cmp:
  case RISCV::BI__builtin_riscv_tx_vec_s_iter_gets:
  case RISCV::BI__builtin_riscv_tx_vec_s_iter_getv:
  case RISCV::BI__builtin_riscv_tx_vec_s_bitwise:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn:
    ICEArguments |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
    break;
  case RISCV::BI__builtin_riscv_tx_vec_sm2v_sparse:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse_int32_add:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse_int32_and:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse_int32_or:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse_int32_xor:
    ICEArguments |= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
    break;
  case RISCV::BI__builtin_riscv_tx_vec_lsu_s2v_mask_broadcast32:
    ICEArguments |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
    break;
  case RISCV::BI__builtin_riscv_tx_vec_sm2v_dyn:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn_int32_add:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn_int32_and:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn_int32_or:
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn_int32_xor:
    ICEArguments |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
    break;
  }
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_base_tcsr_read)
    ICEArguments |= (1 << 0) | (1 << 1);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_base_tcsr_write)
    ICEArguments |= (1 << 1) | (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_tcsr_read)
    ICEArguments |= (1 << 0);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_tcsr_write)
    ICEArguments |= (1 << 1);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_vv_add_s32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_sv_add_s32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_iv_add_s32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_add_s32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_sub_s32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_mul_s32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_add_fp32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_sub_fp32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_mul_fp32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_max_fp32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_elemwise_min_fp32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_convert_s32_f32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_convert_fp32_s32_round)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_convert_fp32_s32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_convert_fp32_e4m3)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_convert_fp32_fp16)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_convert_fp32_bf16)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_sfu_exp)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_sfu_log2_fp32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_sfu_exp2_fp32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_sfu_sin_fp32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_sfu_cos_fp32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_sfu_rsqrt_fp32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_sfu_recip_fp32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_reduce_add_s32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_reduce_max_fp32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_reduce_min_fp32)
    ICEArguments |= (1 << 3) | (1 << 4);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_reduce_add_fp32)
    ICEArguments |= (1 << 2);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_reduce_max_s32)
    ICEArguments |= (1 << 2) | (1 << 3);
  if (BuiltinID == RISCV::BI__builtin_riscv_tx_macro_reduce_min_s32)
    ICEArguments |= (1 << 2) | (1 << 3);
  for (unsigned i = 0, e = E->getNumArgs(); i != e; i++) {
    // Handle aggregate argument, namely RVV tuple types in segment load/store
    if (hasAggregateEvaluationKind(E->getArg(i)->getType())) {
      LValue L = EmitAggExprToLValue(E->getArg(i));
      llvm::Value *AggValue = Builder.CreateLoad(L.getAddress());
      Ops.push_back(AggValue);
      continue;
    }
    Ops.push_back(EmitScalarOrConstFoldImmArg(ICEArguments, i, E));
  }

  Intrinsic::ID ID = Intrinsic::not_intrinsic;
  int PolicyAttrs = 0;
  bool IsMasked = false;
  // This is used by segment load/store to determine it's llvm type.
  unsigned SegInstSEW = 8;
  // This is used by XSfmm.
  unsigned TWiden = 0;

  // Required for overloaded intrinsics.
  llvm::SmallVector<llvm::Type *, 2> IntrinsicTypes;
  auto EmitRVTLSUDMA = [&](llvm::StringRef AsmString) -> llvm::Value * {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, AsmString, "r,r,i,~{memory}", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  };
  auto EmitRVTInlineAsm = [&](StringRef AsmString,
                              StringRef Constraints) -> llvm::Value * {
    llvm::SmallVector<llvm::Type *, 8> ArgTypes;
    for (llvm::Value *Op : Ops)
      ArgTypes.push_back(Op->getType());
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(VoidTy, ArgTypes, false);
    llvm::InlineAsm *IA =
        llvm::InlineAsm::get(FTy, AsmString, Constraints,
                             /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, Ops);
  };
  switch (BuiltinID) {
  default: llvm_unreachable("unexpected builtin ID");
  case RISCV::BI__builtin_riscv_is_uram: {
    llvm::Value *Addr = Ops[0];
    llvm::Value *Limit =
        llvm::ConstantInt::get(Addr->getType(), NPUURAMWindowByteSize);
    llvm::Value *InRange = Builder.CreateICmpULT(Addr, Limit);
    return Builder.CreateZExt(InRange, ResultType);
  }
  case RISCV::BI__builtin_riscv_orc_b_32:
  case RISCV::BI__builtin_riscv_orc_b_64:
  case RISCV::BI__builtin_riscv_clmul_32:
  case RISCV::BI__builtin_riscv_clmul_64:
  case RISCV::BI__builtin_riscv_clmulh_32:
  case RISCV::BI__builtin_riscv_clmulh_64:
  case RISCV::BI__builtin_riscv_clmulr_32:
  case RISCV::BI__builtin_riscv_clmulr_64:
  case RISCV::BI__builtin_riscv_xperm4_32:
  case RISCV::BI__builtin_riscv_xperm4_64:
  case RISCV::BI__builtin_riscv_xperm8_32:
  case RISCV::BI__builtin_riscv_xperm8_64:
  case RISCV::BI__builtin_riscv_brev8_32:
  case RISCV::BI__builtin_riscv_brev8_64:
  case RISCV::BI__builtin_riscv_zip_32:
  case RISCV::BI__builtin_riscv_unzip_32: {
    switch (BuiltinID) {
    default: llvm_unreachable("unexpected builtin ID");
    // Zbb
    case RISCV::BI__builtin_riscv_orc_b_32:
    case RISCV::BI__builtin_riscv_orc_b_64:
      ID = Intrinsic::riscv_orc_b;
      break;

    // Zbc
    case RISCV::BI__builtin_riscv_clmul_32:
    case RISCV::BI__builtin_riscv_clmul_64:
      ID = Intrinsic::clmul;
      break;
    case RISCV::BI__builtin_riscv_clmulh_32:
    case RISCV::BI__builtin_riscv_clmulh_64:
      ID = Intrinsic::riscv_clmulh;
      break;
    case RISCV::BI__builtin_riscv_clmulr_32:
    case RISCV::BI__builtin_riscv_clmulr_64:
      ID = Intrinsic::riscv_clmulr;
      break;

    // Zbkx
    case RISCV::BI__builtin_riscv_xperm8_32:
    case RISCV::BI__builtin_riscv_xperm8_64:
      ID = Intrinsic::riscv_xperm8;
      break;
    case RISCV::BI__builtin_riscv_xperm4_32:
    case RISCV::BI__builtin_riscv_xperm4_64:
      ID = Intrinsic::riscv_xperm4;
      break;

    // Zbkb
    case RISCV::BI__builtin_riscv_brev8_32:
    case RISCV::BI__builtin_riscv_brev8_64:
      ID = Intrinsic::riscv_brev8;
      break;
    case RISCV::BI__builtin_riscv_zip_32:
      ID = Intrinsic::riscv_zip;
      break;
    case RISCV::BI__builtin_riscv_unzip_32:
      ID = Intrinsic::riscv_unzip;
      break;
    }

    IntrinsicTypes = {ResultType};
    break;
  }

  // Packed Averaging Addition and Subtraction
  case RISCV::BI__builtin_riscv_paadd_i8x4:
  case RISCV::BI__builtin_riscv_paadd_i16x2:
  case RISCV::BI__builtin_riscv_paadd_i8x8:
  case RISCV::BI__builtin_riscv_paadd_i16x4:
  case RISCV::BI__builtin_riscv_paadd_i32x2:
  case RISCV::BI__builtin_riscv_paaddu_u8x4:
  case RISCV::BI__builtin_riscv_paaddu_u16x2:
  case RISCV::BI__builtin_riscv_paaddu_u8x8:
  case RISCV::BI__builtin_riscv_paaddu_u16x4:
  case RISCV::BI__builtin_riscv_paaddu_u32x2:
  case RISCV::BI__builtin_riscv_pasub_i8x4:
  case RISCV::BI__builtin_riscv_pasub_i16x2:
  case RISCV::BI__builtin_riscv_pasub_i8x8:
  case RISCV::BI__builtin_riscv_pasub_i16x4:
  case RISCV::BI__builtin_riscv_pasub_i32x2:
  case RISCV::BI__builtin_riscv_pasubu_u8x4:
  case RISCV::BI__builtin_riscv_pasubu_u16x2:
  case RISCV::BI__builtin_riscv_pasubu_u8x8:
  case RISCV::BI__builtin_riscv_pasubu_u16x4:
  case RISCV::BI__builtin_riscv_pasubu_u32x2:
  // Packed Exchanged Addition and Subtraction
  case RISCV::BI__builtin_riscv_pas_x_i16x2:
  case RISCV::BI__builtin_riscv_pas_x_i16x4:
  case RISCV::BI__builtin_riscv_pas_x_i32x2:
  case RISCV::BI__builtin_riscv_psa_x_i16x2:
  case RISCV::BI__builtin_riscv_psa_x_i16x4:
  case RISCV::BI__builtin_riscv_psa_x_i32x2:
  case RISCV::BI__builtin_riscv_psas_x_i16x2:
  case RISCV::BI__builtin_riscv_psas_x_i16x4:
  case RISCV::BI__builtin_riscv_psas_x_i32x2:
  case RISCV::BI__builtin_riscv_pssa_x_i16x2:
  case RISCV::BI__builtin_riscv_pssa_x_i16x4:
  case RISCV::BI__builtin_riscv_pssa_x_i32x2:
  case RISCV::BI__builtin_riscv_paas_x_i16x2:
  case RISCV::BI__builtin_riscv_paas_x_i16x4:
  case RISCV::BI__builtin_riscv_paas_x_i32x2:
  case RISCV::BI__builtin_riscv_pasa_x_i16x2:
  case RISCV::BI__builtin_riscv_pasa_x_i16x4:
  case RISCV::BI__builtin_riscv_pasa_x_i32x2:
  // Packed Absolute Value and Absolute Difference
  case RISCV::BI__builtin_riscv_pabd_i8x4:
  case RISCV::BI__builtin_riscv_pabd_i16x2:
  case RISCV::BI__builtin_riscv_pabd_i8x8:
  case RISCV::BI__builtin_riscv_pabd_i16x4:
  case RISCV::BI__builtin_riscv_pabdu_u8x4:
  case RISCV::BI__builtin_riscv_pabdu_u16x2:
  case RISCV::BI__builtin_riscv_pabdu_u8x8:
  case RISCV::BI__builtin_riscv_pabdu_u16x4: {
    switch (BuiltinID) {
    default:
      llvm_unreachable("unexpected builtin ID");
    case RISCV::BI__builtin_riscv_paadd_i8x4:
    case RISCV::BI__builtin_riscv_paadd_i16x2:
    case RISCV::BI__builtin_riscv_paadd_i8x8:
    case RISCV::BI__builtin_riscv_paadd_i16x4:
    case RISCV::BI__builtin_riscv_paadd_i32x2:
      ID = Intrinsic::riscv_paadd;
      break;
    case RISCV::BI__builtin_riscv_paaddu_u8x4:
    case RISCV::BI__builtin_riscv_paaddu_u16x2:
    case RISCV::BI__builtin_riscv_paaddu_u8x8:
    case RISCV::BI__builtin_riscv_paaddu_u16x4:
    case RISCV::BI__builtin_riscv_paaddu_u32x2:
      ID = Intrinsic::riscv_paaddu;
      break;
    case RISCV::BI__builtin_riscv_pasub_i8x4:
    case RISCV::BI__builtin_riscv_pasub_i16x2:
    case RISCV::BI__builtin_riscv_pasub_i8x8:
    case RISCV::BI__builtin_riscv_pasub_i16x4:
    case RISCV::BI__builtin_riscv_pasub_i32x2:
      ID = Intrinsic::riscv_pasub;
      break;
    case RISCV::BI__builtin_riscv_pasubu_u8x4:
    case RISCV::BI__builtin_riscv_pasubu_u16x2:
    case RISCV::BI__builtin_riscv_pasubu_u8x8:
    case RISCV::BI__builtin_riscv_pasubu_u16x4:
    case RISCV::BI__builtin_riscv_pasubu_u32x2:
      ID = Intrinsic::riscv_pasubu;
      break;
    case RISCV::BI__builtin_riscv_pas_x_i16x2:
    case RISCV::BI__builtin_riscv_pas_x_i16x4:
    case RISCV::BI__builtin_riscv_pas_x_i32x2:
      ID = Intrinsic::riscv_pas;
      break;
    case RISCV::BI__builtin_riscv_psa_x_i16x2:
    case RISCV::BI__builtin_riscv_psa_x_i16x4:
    case RISCV::BI__builtin_riscv_psa_x_i32x2:
      ID = Intrinsic::riscv_psa;
      break;
    case RISCV::BI__builtin_riscv_psas_x_i16x2:
    case RISCV::BI__builtin_riscv_psas_x_i16x4:
    case RISCV::BI__builtin_riscv_psas_x_i32x2:
      ID = Intrinsic::riscv_psas;
      break;
    case RISCV::BI__builtin_riscv_pssa_x_i16x2:
    case RISCV::BI__builtin_riscv_pssa_x_i16x4:
    case RISCV::BI__builtin_riscv_pssa_x_i32x2:
      ID = Intrinsic::riscv_pssa;
      break;
    case RISCV::BI__builtin_riscv_paas_x_i16x2:
    case RISCV::BI__builtin_riscv_paas_x_i16x4:
    case RISCV::BI__builtin_riscv_paas_x_i32x2:
      ID = Intrinsic::riscv_paas;
      break;
    case RISCV::BI__builtin_riscv_pasa_x_i16x2:
    case RISCV::BI__builtin_riscv_pasa_x_i16x4:
    case RISCV::BI__builtin_riscv_pasa_x_i32x2:
      ID = Intrinsic::riscv_pasa;
      break;
    case RISCV::BI__builtin_riscv_pabd_i8x4:
    case RISCV::BI__builtin_riscv_pabd_i16x2:
    case RISCV::BI__builtin_riscv_pabd_i8x8:
    case RISCV::BI__builtin_riscv_pabd_i16x4:
      ID = Intrinsic::riscv_pabd;
      break;
    case RISCV::BI__builtin_riscv_pabdu_u8x4:
    case RISCV::BI__builtin_riscv_pabdu_u16x2:
    case RISCV::BI__builtin_riscv_pabdu_u8x8:
    case RISCV::BI__builtin_riscv_pabdu_u16x4:
      ID = Intrinsic::riscv_pabdu;
      break;
    }

    IntrinsicTypes = {ResultType};
    break;
  }

  // Zk builtins

  // Zknh
  case RISCV::BI__builtin_riscv_sha256sig0:
    ID = Intrinsic::riscv_sha256sig0;
    break;
  case RISCV::BI__builtin_riscv_sha256sig1:
    ID = Intrinsic::riscv_sha256sig1;
    break;
  case RISCV::BI__builtin_riscv_sha256sum0:
    ID = Intrinsic::riscv_sha256sum0;
    break;
  case RISCV::BI__builtin_riscv_sha256sum1:
    ID = Intrinsic::riscv_sha256sum1;
    break;

  // Zksed
  case RISCV::BI__builtin_riscv_sm4ks:
    ID = Intrinsic::riscv_sm4ks;
    break;
  case RISCV::BI__builtin_riscv_sm4ed:
    ID = Intrinsic::riscv_sm4ed;
    break;

  // Zksh
  case RISCV::BI__builtin_riscv_sm3p0:
    ID = Intrinsic::riscv_sm3p0;
    break;
  case RISCV::BI__builtin_riscv_sm3p1:
    ID = Intrinsic::riscv_sm3p1;
    break;

  case RISCV::BI__builtin_riscv_clz_32:
  case RISCV::BI__builtin_riscv_clz_64: {
    Function *F = CGM.getIntrinsic(Intrinsic::ctlz, Ops[0]->getType());
    Value *Result = Builder.CreateCall(F, {Ops[0], Builder.getInt1(false)});
    if (Result->getType() != ResultType)
      Result =
          Builder.CreateIntCast(Result, ResultType, /*isSigned*/ false, "cast");
    return Result;
  }
  case RISCV::BI__builtin_riscv_ctz_32:
  case RISCV::BI__builtin_riscv_ctz_64: {
    Function *F = CGM.getIntrinsic(Intrinsic::cttz, Ops[0]->getType());
    Value *Result = Builder.CreateCall(F, {Ops[0], Builder.getInt1(false)});
    if (Result->getType() != ResultType)
      Result =
          Builder.CreateIntCast(Result, ResultType, /*isSigned*/ false, "cast");
    return Result;
  }

  // Zihintntl
  case RISCV::BI__builtin_riscv_ntl_load: {
    llvm::Type *ResTy = ConvertType(E->getType());
    unsigned DomainVal = 5; // Default __RISCV_NTLH_ALL
    if (Ops.size() == 2)
      DomainVal = cast<ConstantInt>(Ops[1])->getZExtValue();

    llvm::MDNode *RISCVDomainNode = llvm::MDNode::get(
        getLLVMContext(),
        llvm::ConstantAsMetadata::get(Builder.getInt32(DomainVal)));
    llvm::MDNode *NontemporalNode = llvm::MDNode::get(
        getLLVMContext(), llvm::ConstantAsMetadata::get(Builder.getInt32(1)));

    int Width;
    if(ResTy->isScalableTy()) {
      const ScalableVectorType *SVTy = cast<ScalableVectorType>(ResTy);
      llvm::Type *ScalarTy = ResTy->getScalarType();
      Width = ScalarTy->getPrimitiveSizeInBits() *
              SVTy->getElementCount().getKnownMinValue();
    } else
      Width = ResTy->getPrimitiveSizeInBits();
    LoadInst *Load = Builder.CreateLoad(
        Address(Ops[0], ResTy, CharUnits::fromQuantity(Width / 8)));

    Load->setMetadata(llvm::LLVMContext::MD_nontemporal, NontemporalNode);
    Load->setMetadata(CGM.getModule().getMDKindID("riscv-nontemporal-domain"),
                      RISCVDomainNode);

    return Load;
  }
  case RISCV::BI__builtin_riscv_ntl_store: {
    unsigned DomainVal = 5; // Default __RISCV_NTLH_ALL
    if (Ops.size() == 3)
      DomainVal = cast<ConstantInt>(Ops[2])->getZExtValue();

    llvm::MDNode *RISCVDomainNode = llvm::MDNode::get(
        getLLVMContext(),
        llvm::ConstantAsMetadata::get(Builder.getInt32(DomainVal)));
    llvm::MDNode *NontemporalNode = llvm::MDNode::get(
        getLLVMContext(), llvm::ConstantAsMetadata::get(Builder.getInt32(1)));

    StoreInst *Store = Builder.CreateDefaultAlignedStore(Ops[1], Ops[0]);
    Store->setMetadata(llvm::LLVMContext::MD_nontemporal, NontemporalNode);
    Store->setMetadata(CGM.getModule().getMDKindID("riscv-nontemporal-domain"),
                       RISCVDomainNode);

    return Store;
  }
  // Zihintpause
  case RISCV::BI__builtin_riscv_pause: {
    llvm::Function *Fn = CGM.getIntrinsic(llvm::Intrinsic::riscv_pause);
    return Builder.CreateCall(Fn, {});
  }

  // RVT
  case RISCV::BI__builtin_riscv_tx_lsu_tcsr_read: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(ResultType, {Ops[0]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.lsu.tcsr.read $0, $1", "=r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0]});
  }
  case RISCV::BI__builtin_riscv_tx_lsu_tcsr_write: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.lsu.tcsr.write $0, $1", "r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1]});
  }
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2u:
    return EmitRVTLSUDMA("tx.lsu.dma.u2u $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2u_g:
    return EmitRVTLSUDMA("tx.lsu.dma.u2u.g $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2u_s:
    return EmitRVTLSUDMA("tx.lsu.dma.u2u.s $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2u_sg:
    return EmitRVTLSUDMA("tx.lsu.dma.u2u.sg $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2d:
    return EmitRVTLSUDMA("tx.lsu.dma.u2d $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2d_g:
    return EmitRVTLSUDMA("tx.lsu.dma.u2d.g $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2d_s:
    return EmitRVTLSUDMA("tx.lsu.dma.u2d.s $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_u2d_sg:
    return EmitRVTLSUDMA("tx.lsu.dma.u2d.sg $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2u:
    return EmitRVTLSUDMA("tx.lsu.dma.d2u $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2u_g:
    return EmitRVTLSUDMA("tx.lsu.dma.d2u.g $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2u_s:
    return EmitRVTLSUDMA("tx.lsu.dma.d2u.s $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2u_sg:
    return EmitRVTLSUDMA("tx.lsu.dma.d2u.sg $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2d:
    return EmitRVTLSUDMA("tx.lsu.dma.d2d $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2d_g:
    return EmitRVTLSUDMA("tx.lsu.dma.d2d.g $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2d_s:
    return EmitRVTLSUDMA("tx.lsu.dma.d2d.s $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_lsu_dma_d2d_sg:
    return EmitRVTLSUDMA("tx.lsu.dma.d2d.sg $0, $1, $2");
  case RISCV::BI__builtin_riscv_tx_mat_tcsr_read: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(ResultType, {Ops[0]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.mat.tcsr.read $0, $1", "=r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0]});
  }
  case RISCV::BI__builtin_riscv_tx_mat_tcsr_write: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.mat.tcsr.write $0, $1", "r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1]});
  }
  case RISCV::BI__builtin_riscv_tx_mat_setptr: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
         Ops[3]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        // Match npu-kernels MAT format: tx.mat.setptr rd, rs1, rs2, group.
        FTy, "tx.mat.setptr $0, $1, $2, $3", "r,r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3]});
  }
  case RISCV::BI__builtin_riscv_tx_mat_mm: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
         Ops[3]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.mat.mm $0, $1, $2, $3", "r,r,r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3]});
  }
  case RISCV::BI__builtin_riscv_tx_mat_mma: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
         Ops[3]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.mat.mma $0, $1, $2, $3", "r,r,r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3]});
  }
  case RISCV::BI__builtin_riscv_tx_mat_conv2d: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
         Ops[3]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.mat.conv2d $0, $1, $2, $3", "r,r,r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3]});
  }
  case RISCV::BI__builtin_riscv_tx_mat_conv2da: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
         Ops[3]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.mat.conv2da $0, $1, $2, $3", "r,r,r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_tcsr_read: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(ResultType, {Ops[0]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.tcsr.read $0, $1", "=r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_tcsr_write: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.tcsr.write $0, $1", "r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_tcsr_read: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(ResultType, {Ops[0]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.tcsr.read $0, $1", "=r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_tcsr_write: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.tcsr.write $0, $1", "r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1]});
  }
  case RISCV::BI__builtin_riscv_tx_mat_sb_produce: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(VoidTy, {Ops[0]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.mat.sb.produce $0", "r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0]});
  }
  case RISCV::BI__builtin_riscv_tx_mat_sb_bar: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(ResultType, {}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.mat.sb.bar $0", "=r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {});
  }
  case RISCV::BI__builtin_riscv_tx_vec_sb_produce: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(VoidTy, {Ops[0]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.sb.produce $0", "r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_sb_bar: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(ResultType, {}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.sb.bar $0", "=r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {});
  }
  case RISCV::BI__builtin_riscv_tx_lsu_sb_produce: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(VoidTy, {Ops[0]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.lsu.sb.produce $0", "r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0]});
  }
  case RISCV::BI__builtin_riscv_tx_lsu_sb_bar: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(ResultType, {}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.lsu.sb.bar $0", "=r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sb_produce: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(VoidTy, {Ops[0]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sb.produce $0", "r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sb_bar: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(ResultType, {}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sb.bar $0", "=r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {});
  }
  case RISCV::BI__builtin_riscv_tx_sb_get_any: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(ResultType, {}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.sb.get.any $0", "=r", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {});
  }
  case RISCV::BI__builtin_riscv_tx_vec_v_load: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.v.load $0, $1, $2", "r,i,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_s_load: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.s.load $0, $1, $2", "r,i,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_const_load: {
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(VoidTy, {Ops[0]->getType(), Ops[1]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.const.load $0, $1", "r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_v_store: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.v.store $0, $1, $2", "r,i,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_s_store: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.s.store $0, $1, $2", "r,i,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_bitwise: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
         Ops[3]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.vec.vv.int32.bitwise $0, $1, $2, $3", "i,i,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3]});
  }
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_rol:
    return EmitRVTInlineAsm("tx.vec.vv.int32.rol $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_sll:
    return EmitRVTInlineAsm("tx.vec.vv.int32.sll $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_sra:
    return EmitRVTInlineAsm("tx.vec.vv.int32.sra $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_srl:
    return EmitRVTInlineAsm("tx.vec.vv.int32.srl $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_add:
    return EmitRVTInlineAsm("tx.vec.vv.int32.add $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_sub:
    return EmitRVTInlineAsm("tx.vec.vv.int32.sub $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_mulu24:
    return EmitRVTInlineAsm("tx.vec.vv.int32.mulu24 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_mul24:
    return EmitRVTInlineAsm("tx.vec.vv.int32.mul24 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_maxu:
    return EmitRVTInlineAsm("tx.vec.vv.int32.maxu $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_minu:
    return EmitRVTInlineAsm("tx.vec.vv.int32.minu $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_max:
    return EmitRVTInlineAsm("tx.vec.vv.int32.max $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_min:
    return EmitRVTInlineAsm("tx.vec.vv.int32.min $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_add:
    return EmitRVTInlineAsm("tx.vec.vv.fp32.add $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_sub:
    return EmitRVTInlineAsm("tx.vec.vv.fp32.sub $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_mul:
    return EmitRVTInlineAsm("tx.vec.vv.fp32.mul $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_max:
    return EmitRVTInlineAsm("tx.vec.vv.fp32.max $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_min:
    return EmitRVTInlineAsm("tx.vec.vv.fp32.min $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_mux2:
    return EmitRVTInlineAsm("tx.vec.vv.mux2 $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_int32_cmp:
    return EmitRVTInlineAsm("tx.vec.vv.int32.cmp $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_vv_fp32_cmp:
    return EmitRVTInlineAsm("tx.vec.vv.fp32.cmp $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v_int32_sll:
    return EmitRVTInlineAsm("tx.vec.v.int32.sll $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v_int32_sra:
    return EmitRVTInlineAsm("tx.vec.v.int32.sra $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v_int32_srl:
    return EmitRVTInlineAsm("tx.vec.v.int32.srl $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v_fp32_sfu:
    return EmitRVTInlineAsm("tx.vec.v.float32.sfu $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_cvt_fp32_int32:
    return EmitRVTInlineAsm("tx.vec.cvt.fp32.int32 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_cvt_s32_fp32:
    return EmitRVTInlineAsm("tx.vec.cvt.s32.fp32 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_cvt_fp32_fp16:
    return EmitRVTInlineAsm("tx.vec.cvt.fp32.fp16 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_cvt_fp32_bf16:
    return EmitRVTInlineAsm("tx.vec.cvt.fp32.bf16 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_cvt_fp16_fp32:
    return EmitRVTInlineAsm("tx.vec.cvt.fp16.fp32 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_cvt_bf16_fp32:
    return EmitRVTInlineAsm("tx.vec.cvt.bf16.fp32 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_cvt_mxfp8_bf16:
    return EmitRVTInlineAsm("tx.vec.cvt.mxfp8.bf16 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_cvt_mxfp4_bf16:
    return EmitRVTInlineAsm("tx.vec.cvt.mxfp4.bf16 $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_rol:
    return EmitRVTInlineAsm("tx.vec.s.int32.rol $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_sll:
    return EmitRVTInlineAsm("tx.vec.s.int32.sll $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_sra:
    return EmitRVTInlineAsm("tx.vec.s.int32.sra $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_srl:
    return EmitRVTInlineAsm("tx.vec.s.int32.srl $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_add:
    return EmitRVTInlineAsm("tx.vec.s.int32.add $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_sub:
    return EmitRVTInlineAsm("tx.vec.s.int32.sub $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_mul:
    return EmitRVTInlineAsm("tx.vec.s.int32.mul $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_iter_getdim:
    return EmitRVTInlineAsm("tx.vec.s.iter.getdim $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_iter_gets:
    return EmitRVTInlineAsm("tx.vec.s.iter.gets $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_iter_getv:
    return EmitRVTInlineAsm("tx.vec.s.iter.getv $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_bitwise:
    return EmitRVTInlineAsm("tx.vec.s.bitwise $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_bitwise_beq:
    return EmitRVTInlineAsm("tx.vec.s.bitwise.beq $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_bitwise_bne:
    return EmitRVTInlineAsm("tx.vec.s.bitwise.bne $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_blt:
    return EmitRVTInlineAsm("tx.vec.s.int32.blt $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_s_int32_bge:
    return EmitRVTInlineAsm("tx.vec.s.int32.bge $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2v:
    return EmitRVTInlineAsm("tx.vec.lsu.u2v $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2v:
    return EmitRVTInlineAsm("tx.vec.lsu.sm2v $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2v_broadcast32:
    return EmitRVTInlineAsm("tx.vec.lsu.u2v.broadcast32 $0, $1, $2",
                            "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2v_broadcast32:
    return EmitRVTInlineAsm("tx.vec.lsu.sm2v.broadcast32 $0, $1, $2",
                            "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_s2v_broadcast32:
    return EmitRVTInlineAsm("tx.vec.lsu.s2v.broadcast32 $0, $1, $2",
                            "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_g2v_broadcast32:
    return EmitRVTInlineAsm("tx.vec.lsu.g2v.broadcast32 $0, $1, $2",
                            "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2s:
    return EmitRVTInlineAsm("tx.vec.lsu.u2s $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2s:
    return EmitRVTInlineAsm("tx.vec.lsu.sm2s $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_g2s:
    return EmitRVTInlineAsm("tx.vec.lsu.g2s $0, $1", "r,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2v_mask:
    return EmitRVTInlineAsm("tx.vec.lsu.u2v.mask $0, $1, $2, $3",
                            "r,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2v_mask:
    return EmitRVTInlineAsm("tx.vec.lsu.sm2v.mask $0, $1, $2, $3",
                            "r,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_u2v_mask_broadcast32:
    return EmitRVTInlineAsm("tx.vec.lsu.u2v.mask.broadcast32 $0, $1, $2, $3",
                            "r,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_sm2v_mask_broadcast32:
    return EmitRVTInlineAsm("tx.vec.lsu.sm2v.mask.broadcast32 $0, $1, $2, $3",
                            "r,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_s2v_mask_broadcast32:
    return EmitRVTInlineAsm("tx.vec.lsu.s2v.mask.broadcast32 $0, $1, $2, $3",
                            "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_g2v_mask_broadcast32:
    return EmitRVTInlineAsm("tx.vec.lsu.g2v.mask.broadcast32 $0, $1, $2, $3",
                            "r,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_v2u:
    return EmitRVTInlineAsm("tx.vec.lsu.v2u $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_v2sm:
    return EmitRVTInlineAsm("tx.vec.lsu.v2sm $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_s2u:
    return EmitRVTInlineAsm("tx.vec.lsu.s2u $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_s2sm:
    return EmitRVTInlineAsm("tx.vec.lsu.s2sm $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_v2u_mask:
    return EmitRVTInlineAsm("tx.vec.lsu.v2u.mask $0, $1, $2, $3",
                            "r,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_lsu_v2sm_mask:
    return EmitRVTInlineAsm("tx.vec.lsu.v2sm.mask $0, $1, $2, $3",
                            "r,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_sm2v_sparse:
    return EmitRVTInlineAsm("tx.vec.sm2v.sparse $0, $1, $2, $3, $4",
                            "r,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse:
    return EmitRVTInlineAsm("tx.vec.v2sm.sparse $0, $1, $2, $3",
                            "r,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse_int32_add:
    return EmitRVTInlineAsm("tx.vec.v2sm.sparse.int32.add $0, $1, $2, $3, $4",
                            "r,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse_int32_and:
    return EmitRVTInlineAsm("tx.vec.v2sm.sparse.int32.and $0, $1, $2, $3, $4",
                            "r,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse_int32_or:
    return EmitRVTInlineAsm("tx.vec.v2sm.sparse.int32.or $0, $1, $2, $3, $4",
                            "r,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_sparse_int32_xor:
    return EmitRVTInlineAsm("tx.vec.v2sm.sparse.int32.xor $0, $1, $2, $3, $4",
                            "r,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_sm2v_dyn:
    return EmitRVTInlineAsm("tx.vec.sm2v.dyn $0, $1, $2, $3, $4",
                            "i,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn:
    return EmitRVTInlineAsm("tx.vec.v2sm.dyn $0, $1, $2, $3", "i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn_int32_add:
    return EmitRVTInlineAsm("tx.vec.v2sm.dyn.int32.add $0, $1, $2, $3, $4",
                            "i,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn_int32_and:
    return EmitRVTInlineAsm("tx.vec.v2sm.dyn.int32.and $0, $1, $2, $3, $4",
                            "i,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn_int32_or:
    return EmitRVTInlineAsm("tx.vec.v2sm.dyn.int32.or $0, $1, $2, $3, $4",
                            "i,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_v2sm_dyn_int32_xor:
    return EmitRVTInlineAsm("tx.vec.v2sm.dyn.int32.xor $0, $1, $2, $3, $4",
                            "i,i,i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_xor:
    return EmitRVTInlineAsm("tx.vec.shuffle.xor $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_xor_dyn:
    return EmitRVTInlineAsm("tx.vec.shuffle.xor.dyn $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_ror:
    return EmitRVTInlineAsm("tx.vec.shuffle.ror $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_ror_dyn:
    return EmitRVTInlineAsm("tx.vec.shuffle.ror.dyn $0, $1, $2", "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_broadcast:
    return EmitRVTInlineAsm("tx.vec.shuffle.broadcast $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_broadcast_dyn:
    return EmitRVTInlineAsm("tx.vec.shuffle.broadcast.dyn $0, $1, $2",
                            "i,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_compress:
    return EmitRVTInlineAsm("tx.vec.shuffle.compress $0, $1, $2", "r,i,i");
  case RISCV::BI__builtin_riscv_tx_vec_shuffle_compress_dyn:
    return EmitRVTInlineAsm("tx.vec.shuffle.compress.dyn $0, $1, $2",
                            "i,i,i");
  case RISCV::BI__builtin_riscv_tx_base_tcsr_read: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        ResultType, {Ops[0]->getType(), Ops[1]->getType()}, false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.base.tcsr.read $0, $1, $2", "=r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1]});
  }
  case RISCV::BI__builtin_riscv_tx_base_tcsr_write: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.base.tcsr.write $0, $1, $2", "r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_vv_add_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.vv.add.s32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_sv_add_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.sv.add.s32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_iv_add_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy,
        {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.iv.add.s32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_add_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.add.s32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_sub_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.sub.s32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_mul_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.mul.s32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_add_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.add.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_sub_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.sub.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_mul_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.mul.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_max_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.max.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_elemwise_min_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.elemwise.min.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_convert_s32_f32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.convert.s32.f32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_convert_fp32_s32_round: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.convert.fp32.s32.round $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_convert_fp32_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.convert.fp32.s32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_convert_fp32_e4m3: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.convert.fp32.e4m3 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_convert_fp32_fp16: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.convert.fp32.fp16 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_convert_fp32_bf16: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.convert.fp32.bf16 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sfu_exp: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sfu.exp $0, $1, $2", "r,r,i", /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sfu_log2_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sfu.log2.fp32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sfu_exp2_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sfu.exp2.fp32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sfu_sin_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sfu.sin.fp32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sfu_cos_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sfu.cos.fp32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sfu_rsqrt_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sfu.rsqrt.fp32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_sfu_recip_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.sfu.recip.fp32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_reduce_add_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.reduce.add.s32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_reduce_max_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.reduce.max.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_reduce_min_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType(), Ops[4]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.reduce.min.fp32 $0, $1, $2, $3, $4", "r,r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3], Ops[4]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_reduce_add_fp32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.reduce.add.fp32 $0, $1, $2", "r,r,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_reduce_max_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.reduce.max.s32 $0, $1, $2, $3", "r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3]});
  }
  case RISCV::BI__builtin_riscv_tx_macro_reduce_min_s32: {
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        VoidTy, {Ops[0]->getType(), Ops[1]->getType(), Ops[2]->getType(),
                 Ops[3]->getType()},
        false);
    llvm::InlineAsm *IA = llvm::InlineAsm::get(
        FTy, "tx.macro.reduce.min.s32 $0, $1, $2, $3", "r,r,i,i",
        /*hasSideEffects=*/true);
    return Builder.CreateCall(IA, {Ops[0], Ops[1], Ops[2], Ops[3]});
  }

  // XCValu
  case RISCV::BI__builtin_riscv_cv_alu_addN:
    ID = Intrinsic::riscv_cv_alu_addN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_addRN:
    ID = Intrinsic::riscv_cv_alu_addRN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_adduN:
    ID = Intrinsic::riscv_cv_alu_adduN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_adduRN:
    ID = Intrinsic::riscv_cv_alu_adduRN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_clip:
    ID = Intrinsic::riscv_cv_alu_clip;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_clipu:
    ID = Intrinsic::riscv_cv_alu_clipu;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_extbs:
    return Builder.CreateSExt(Builder.CreateTrunc(Ops[0], Int8Ty), Int32Ty,
                              "extbs");
  case RISCV::BI__builtin_riscv_cv_alu_extbz:
    return Builder.CreateZExt(Builder.CreateTrunc(Ops[0], Int8Ty), Int32Ty,
                              "extbz");
  case RISCV::BI__builtin_riscv_cv_alu_exths:
    return Builder.CreateSExt(Builder.CreateTrunc(Ops[0], Int16Ty), Int32Ty,
                              "exths");
  case RISCV::BI__builtin_riscv_cv_alu_exthz:
    return Builder.CreateZExt(Builder.CreateTrunc(Ops[0], Int16Ty), Int32Ty,
                              "exthz");
  case RISCV::BI__builtin_riscv_cv_alu_sle:
    return Builder.CreateZExt(Builder.CreateICmpSLE(Ops[0], Ops[1]), Int32Ty,
                              "sle");
  case RISCV::BI__builtin_riscv_cv_alu_sleu:
    return Builder.CreateZExt(Builder.CreateICmpULE(Ops[0], Ops[1]), Int32Ty,
                              "sleu");
  case RISCV::BI__builtin_riscv_cv_alu_subN:
    ID = Intrinsic::riscv_cv_alu_subN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_subRN:
    ID = Intrinsic::riscv_cv_alu_subRN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_subuN:
    ID = Intrinsic::riscv_cv_alu_subuN;
    break;
  case RISCV::BI__builtin_riscv_cv_alu_subuRN:
    ID = Intrinsic::riscv_cv_alu_subuRN;
    break;

  // XAndesPerf
  case RISCV::BI__builtin_riscv_nds_ffb_32:
  case RISCV::BI__builtin_riscv_nds_ffb_64:
    IntrinsicTypes = {ResultType};
    ID = Intrinsic::riscv_nds_ffb;
    break;
  case RISCV::BI__builtin_riscv_nds_ffzmism_32:
  case RISCV::BI__builtin_riscv_nds_ffzmism_64:
    IntrinsicTypes = {ResultType};
    ID = Intrinsic::riscv_nds_ffzmism;
    break;
  case RISCV::BI__builtin_riscv_nds_ffmism_32:
  case RISCV::BI__builtin_riscv_nds_ffmism_64:
    IntrinsicTypes = {ResultType};
    ID = Intrinsic::riscv_nds_ffmism;
    break;
  case RISCV::BI__builtin_riscv_nds_flmism_32:
  case RISCV::BI__builtin_riscv_nds_flmism_64:
    IntrinsicTypes = {ResultType};
    ID = Intrinsic::riscv_nds_flmism;
    break;

  // XAndesBFHCvt
  case RISCV::BI__builtin_riscv_nds_fcvt_s_bf16:
    return Builder.CreateFPExt(Ops[0], FloatTy);
  case RISCV::BI__builtin_riscv_nds_fcvt_bf16_s:
    return Builder.CreateFPTrunc(Ops[0], BFloatTy);

    // Vector builtins are handled from here.
#include "clang/Basic/riscv_vector_builtin_cg.inc"

    // SiFive Vector builtins are handled from here.
#include "clang/Basic/riscv_sifive_vector_builtin_cg.inc"

    // Andes Vector builtins are handled from here.
#include "clang/Basic/riscv_andes_vector_builtin_cg.inc"
  }

  assert(ID != Intrinsic::not_intrinsic);

  llvm::Function *F = CGM.getIntrinsic(ID, IntrinsicTypes);
  return Builder.CreateCall(F, Ops, "");
}
