//===----------------------------------------------------------------------===//
// RVT instruction definitions for direct in-repo implementation.
// Compile-first baseline with RVT 32/64-bit encodings.
//===----------------------------------------------------------------------===//

// Custom opcode from current RVT 32-bit proposal.
def OPC_RVT32 : RISCVOpcode<"OPC_RVT32", 0b0001011>;
def OPC_RVT64 : RISCVOpcode<"OPC_RVT64", 0b0111111>;

// Local immediate operands for TCSR address fields.
def rvt_uimm10 : RISCVUImmLeafOp<10>;
def rvt_lsu_uimm8 : RISCVUImmLeafOp<8>;
def rvt_simm8 : RISCVSImmLeafOp<8>;

// Feature gates.
//
// Use RISCVExtension (not plain SubtargetFeature) so Clang's RISC-V frontend
// can validate and forward +tx* / +ztx* via RISCVISAInfo::parseFeatures.
// We preserve existing feature spellings and Subtarget field names.
def FeatureVendorTXBase  : RISCVExtension<1, 0, "RVT base instructions", [], "HasTXBase">;
def FeatureVendorTXLSU   : RISCVExtension<1, 0, "RVT LSU instructions",  [], "HasTXLSU">;
def FeatureVendorTXMat   : RISCVExtension<1, 0, "RVT MAT instructions",  [], "HasTXMat">;
def FeatureVendorTXVec   : RISCVExtension<1, 0, "RVT VEC instructions",  [], "HasTXVec">;
def FeatureVendorTXMacro : RISCVExtension<1, 0, "RVT MACRO instructions",[], "HasTXMacro">;

// Spec-aligned aliases. Keep both tx* and ztx* accepted in -mattr/-target-feature.
def FeatureVendorZTXBase  : RISCVExtension<1, 0, "RVT base instructions (spec alias)", [], "HasZTXBase">;
def FeatureVendorZTXLSU   : RISCVExtension<1, 0, "RVT LSU instructions (spec alias)",  [], "HasZTXLSU">;
def FeatureVendorZTXMat   : RISCVExtension<1, 0, "RVT MAT instructions (spec alias)",  [], "HasZTXMat">;
def FeatureVendorZTXVec   : RISCVExtension<1, 0, "RVT VEC instructions (spec alias)",  [], "HasZTXVec">;
def FeatureVendorZTXMacro : RISCVExtension<1, 0, "RVT MACRO instructions (spec alias)",[], "HasZTXMacro">;

def HasTXBaseOrZTXBase
    : Predicate<"Subtarget->hasTXBase() || Subtarget->hasZTXBase()">,
      AssemblerPredicate<(any_of FeatureVendorTXBase, FeatureVendorZTXBase),
                         "'txbase' or 'ztxbase' (RVT base instructions)">;

def HasTXLSUOrZTXLSU
    : Predicate<"Subtarget->hasTXLSU() || Subtarget->hasZTXLSU()">,
      AssemblerPredicate<(any_of FeatureVendorTXLSU, FeatureVendorZTXLSU),
                         "'txlsu' or 'ztxlsu' (RVT LSU instructions)">;

def HasTXMatOrZTXMat
    : Predicate<"Subtarget->hasTXMat() || Subtarget->hasZTXMat()">,
      AssemblerPredicate<(any_of FeatureVendorTXMat, FeatureVendorZTXMat),
                         "'txmat' or 'ztxmat' (RVT MAT instructions)">;

def HasTXVecOrZTXVec
    : Predicate<"Subtarget->hasTXVec() || Subtarget->hasZTXVec()">,
      AssemblerPredicate<(any_of FeatureVendorTXVec, FeatureVendorZTXVec),
                         "'txvec' or 'ztxvec' (RVT VEC instructions)">;

def HasTXMacroOrZTXMacro
    : Predicate<"Subtarget->hasTXMacro() || Subtarget->hasZTXMacro()">,
      AssemblerPredicate<(any_of FeatureVendorTXMacro, FeatureVendorZTXMacro),
                         "'txmacro' or 'ztxmacro' (RVT MACRO instructions)">;

// -------------------------------
// 32-bit families (base/lsu/mat)
// -------------------------------

class RVTBaseTCSRRead<string asmstr>
  : RVInst<(outs GPR:$rd), (ins uimm4:$unit, rvt_uimm10:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<4> unit;
  bits<10> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = unit;
  let Inst{22-13} = tcsr_addr;
  let Inst{12} = 0b1;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTBaseTCSRWrite<string asmstr>
  : RVInst<(outs), (ins GPR:$src, uimm4:$unit, rvt_uimm10:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> src;
  bits<4> unit;
  bits<10> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = unit;
  let Inst{22-13} = tcsr_addr;
  let Inst{12} = 0b0;
  let Inst{11-7} = src;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTDMABase<bits<1> src_v, bits<1> dst_v, bits<1> scatter_v,
                 bits<1> gather_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, GPR:$rs1, uimm3:$group),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> rs1;
  bits<3> group;
  let hasSideEffects = 1;
  // DMA reads source data, writes destination data, and may also read gather
  // and/or scatter address tables from URAM.
  let mayLoad = 1;
  let mayStore = 1;
  // LSU DMA encoding from the 2026-07-27 RVT specification:
  // [31:25]=funct7, [24]=reserved, [23]=gather, [22:20]=group,
  // [19:15]=rs1, [14]=scatter, [13]=src, [12]=dst, [11:7]=rd.
  let Inst{31-25} = 0b0000100;
  let Inst{24} = 0b0;
  let Inst{23} = gather_v;
  let Inst{22-20} = group;
  let Inst{19-15} = rs1;
  let Inst{14} = scatter_v;
  let Inst{13} = src_v;
  let Inst{12} = dst_v;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTLSUTCSRRead<string asmstr>
  : RVInst<(outs GPR:$rd), (ins rvt_lsu_uimm8:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<8> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  // RVT section 3.1 LSU TCSR layout. LSU owns addresses 0x000-0x0ff,
  // so address bits [9:8] are fixed to zero.
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0001; // LSU
  let Inst{22-21} = 0b00;
  let Inst{20-13} = tcsr_addr;
  let Inst{12} = 0b1;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTLSUTCSRWrite<string asmstr>
  : RVInst<(outs), (ins GPR:$src, rvt_lsu_uimm8:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> src;
  bits<8> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0001; // LSU
  let Inst{22-21} = 0b00;
  let Inst{20-13} = tcsr_addr;
  let Inst{12} = 0b0;
  let Inst{11-7} = src;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTMAT<bits<3> funct3_v, bits<2> mode_high, bits<1> acc_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, GPR:$rs1, GPR:$rs2, uimm3:$group),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> rs1;
  bits<5> rs2;
  bits<3> group;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  // Align with npu-kernels MAT encoding:
  //   encoding = (MAT_FUNCT << 29) | (mode << 12) | OPC_RVT32 | ((group & 7) << 25)
  // Here mode is split as mode_high[14:13] + acc_v[12]:
  //   mm=000, mma=001, conv2d=010, conv2da=011.
  let Inst{31-29} = funct3_v;
  let Inst{28} = 0b0;
  let Inst{27-25} = group;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-13} = mode_high;
  let Inst{12} = acc_v;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTMATSetPtr<string asmstr>
  : RVInst<(outs), (ins GPR:$rs1, GPR:$rs2, uimm3:$group),
           asmstr, "", [], InstFormatOther> {
  bits<5> rs1;
  bits<5> rs2;
  bits<3> group;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-29} = 0b001;
  let Inst{28} = 0b0;
  let Inst{27-25} = group;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  // The specification reserves rd and fixes mode to 0b100.
  let Inst{14-12} = 0b100;
  let Inst{11-7} = 0;
  let Inst{6-0} = OPC_RVT32.Value;
}

// 64-bit RVT encodings. Hardware fetches these as a single 8-byte word, so the
// assembler and compiler must emit them on an 8-byte boundary.
class RVTInst64<dag outs, dag ins, string opcodestr, string argstr,
                list<dag> pattern, InstFormat format>
    : RVInst64<outs, ins, opcodestr, argstr, pattern, format> {
  let Requires64BitAlignment = 1;
}

class RVTMacroElemwise<bits<2> bcast_v, bits<6> op_v, bits<8> dtype_v, string asmstr>
  : RVTInst64<(outs), (ins GPR:$rd, GPR:$rs1, GPR:$rs2, uimm3:$group),
             asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> rs1;
  bits<5> rs2;
  bits<3> group;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{63-48} = 0;
  let Inst{47-46} = bcast_v;
  let Inst{45-40} = op_v;
  let Inst{39-32} = dtype_v;
  let Inst{31-25} = 0b0000000;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = group;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT64.Value;
}

// The specification defines three broadcast modes with independent operands.
multiclass RVTMacroElemwiseForms<bits<6> op, bits<8> dtype, string suffix> {
  def _VV : RVTMacroElemwise<0, op, dtype,
    "tx.macro.elemwise.vv." # suffix # " $rd, $rs1, $rs2, $group">;
  def _SV : RVTMacroElemwise<1, op, dtype,
    "tx.macro.elemwise.sv." # suffix # " $rd, $rs1, $rs2, $group">;
  def _IV : RVTMacroElemwise<2, op, dtype,
    "tx.macro.elemwise.iv." # suffix # " $rd, $rs1, $rs2, $group">;
}

class RVTMacroConvert<bits<8> out_t, bits<8> in_t, string asmstr>
  : RVTInst64<(outs), (ins GPR:$rd, GPR:$rs1, uimm3:$group),
             asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> rs1;
  bits<3> group;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{63-48} = 0;
  // convert.<destination>.<source>: [47:40]=destination, [39:32]=source.
  // rd supplies an address; this instruction does not define a GPR.
  let Inst{47-40} = out_t;
  let Inst{39-32} = in_t;
  let Inst{31-25} = 0b0000001;
  let Inst{24-20} = 0b00000;
  let Inst{19-15} = rs1;
  let Inst{14-12} = group;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT64.Value;
}

class RVTMacroSFU<bits<8> op_v, string asmstr>
  : RVTInst64<(outs), (ins GPR:$rd, GPR:$rs1, uimm3:$group),
             asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> rs1;
  bits<3> group;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{63-48} = 0;
  // npu-kernels SFU: [47:40]=0, [39:32]=op.
  let Inst{47-40} = 0b00000000;
  let Inst{39-32} = op_v;
  let Inst{31-25} = 0b0000010;
  let Inst{24-20} = 0b00000;
  let Inst{19-15} = rs1;
  let Inst{14-12} = group;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT64.Value;
}

class RVTMacroReduce<bits<1> index_v, bits<6> op_v, bits<8> dtype_v, string asmstr>
  : RVTInst64<(outs), (ins GPR:$rd, GPR:$rs1, uimm3:$group),
             asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> rs1;
  bits<3> group;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{63-48} = 0;
  let Inst{47} = 0b0;
  let Inst{46} = index_v;
  let Inst{45-40} = op_v;
  let Inst{39-32} = dtype_v;
  let Inst{31-25} = 0b0000011;
  let Inst{24-20} = 0b00000;
  let Inst{19-15} = rs1;
  let Inst{14-12} = group;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT64.Value;
}

class RVTMacroReduce3P<bits<6> op_v, bits<8> dtype_v, string asmstr>
  : RVTInst64<(outs), (ins GPR:$rd, GPR:$rs1, GPR:$rs2, uimm3:$group, uimm1:$index),
             asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> rs1;
  bits<5> rs2;
  bits<3> group;
  bits<1> index;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{63-48} = 0;
  let Inst{47} = 0b0;
  let Inst{46} = index;
  let Inst{45-40} = op_v;
  let Inst{39-32} = dtype_v;
  let Inst{31-25} = 0b0000011;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = group;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT64.Value;
}

class RVTMacroShuffleCompress<string asmstr>
  : RVTInst64<(outs), (ins GPR:$rd, GPR:$rs1, GPR:$rs2, uimm3:$group),
              asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> rs1;
  bits<5> rs2;
  bits<3> group;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{63-36} = 0;
  let Inst{35-32} = 1; // Compress.
  let Inst{31-25} = 0b0000101;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = group;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT64.Value;
}

// -------------------------------
// Phase-1 instruction definitions
// -------------------------------

// Base/TCSR.
let Predicates = [HasTXBaseOrZTXBase] in {
def RVT_BASE_TCSR_READ  : RVTBaseTCSRRead<"tx.base.tcsr.read $rd, $unit, $tcsr_addr">;
def RVT_BASE_TCSR_WRITE : RVTBaseTCSRWrite<"tx.base.tcsr.write $src, $unit, $tcsr_addr">;
} // Predicates = [HasTXBaseOrZTXBase]

let Predicates = [HasTXLSUOrZTXLSU] in {
// LSU DMA. The suffix selects the source gather table (.g), destination
// scatter table (.s), or both (.sg).
def RVT_LSU_DMA_U2U    : RVTDMABase<0, 0, 0, 0, "tx.lsu.dma.u2u $rd, $rs1, $group">;
def RVT_LSU_DMA_U2U_G  : RVTDMABase<0, 0, 0, 1, "tx.lsu.dma.u2u.g $rd, $rs1, $group">;
def RVT_LSU_DMA_U2U_S  : RVTDMABase<0, 0, 1, 0, "tx.lsu.dma.u2u.s $rd, $rs1, $group">;
def RVT_LSU_DMA_U2U_SG : RVTDMABase<0, 0, 1, 1, "tx.lsu.dma.u2u.sg $rd, $rs1, $group">;
def RVT_LSU_DMA_U2D    : RVTDMABase<0, 1, 0, 0, "tx.lsu.dma.u2d $rd, $rs1, $group">;
def RVT_LSU_DMA_U2D_G  : RVTDMABase<0, 1, 0, 1, "tx.lsu.dma.u2d.g $rd, $rs1, $group">;
def RVT_LSU_DMA_U2D_S  : RVTDMABase<0, 1, 1, 0, "tx.lsu.dma.u2d.s $rd, $rs1, $group">;
def RVT_LSU_DMA_U2D_SG : RVTDMABase<0, 1, 1, 1, "tx.lsu.dma.u2d.sg $rd, $rs1, $group">;
def RVT_LSU_DMA_D2U    : RVTDMABase<1, 0, 0, 0, "tx.lsu.dma.d2u $rd, $rs1, $group">;
def RVT_LSU_DMA_D2U_G  : RVTDMABase<1, 0, 0, 1, "tx.lsu.dma.d2u.g $rd, $rs1, $group">;
def RVT_LSU_DMA_D2U_S  : RVTDMABase<1, 0, 1, 0, "tx.lsu.dma.d2u.s $rd, $rs1, $group">;
def RVT_LSU_DMA_D2U_SG : RVTDMABase<1, 0, 1, 1, "tx.lsu.dma.d2u.sg $rd, $rs1, $group">;
def RVT_LSU_DMA_D2D    : RVTDMABase<1, 1, 0, 0, "tx.lsu.dma.d2d $rd, $rs1, $group">;
def RVT_LSU_DMA_D2D_G  : RVTDMABase<1, 1, 0, 1, "tx.lsu.dma.d2d.g $rd, $rs1, $group">;
def RVT_LSU_DMA_D2D_S  : RVTDMABase<1, 1, 1, 0, "tx.lsu.dma.d2d.s $rd, $rs1, $group">;
def RVT_LSU_DMA_D2D_SG : RVTDMABase<1, 1, 1, 1, "tx.lsu.dma.d2d.sg $rd, $rs1, $group">;
def RVT_LSU_TCSR_READ  : RVTLSUTCSRRead<"tx.lsu.tcsr.read $rd, $tcsr_addr">;
def RVT_LSU_TCSR_WRITE : RVTLSUTCSRWrite<"tx.lsu.tcsr.write $src, $tcsr_addr">;
} // Predicates = [HasTXLSUOrZTXLSU]

// MAT.
let Predicates = [HasTXMatOrZTXMat] in {
def RVT_MAT_MM     : RVTMAT<0b001, 0b00, 0b0, "tx.mat.mm $rd, $rs1, $rs2, $group">;
def RVT_MAT_MMA    : RVTMAT<0b001, 0b00, 0b1, "tx.mat.mma $rd, $rs1, $rs2, $group">;
// conv2d/conv2da use mode=0b010/0b011 with the same MAT_FUNCT family.
def RVT_MAT_CONV2D : RVTMAT<0b001, 0b01, 0b0, "tx.mat.conv2d $rd, $rs1, $rs2, $group">;
def RVT_MAT_CONV2DA: RVTMAT<0b001, 0b01, 0b1, "tx.mat.conv2da $rd, $rs1, $rs2, $group">;
def RVT_MAT_SETPTR : RVTMATSetPtr<"tx.mat.setptr $rs1, $rs2, $group">;
} // Predicates = [HasTXMatOrZTXMat]

class RVTMatTCSRRead<string asmstr>
  : RVInst<(outs GPR:$rd), (ins rvt_uimm10:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<10> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0010; // MAT
  let Inst{22-13} = tcsr_addr;
  let Inst{12} = 0b1;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTMatTCSRWrite<string asmstr>
  : RVInst<(outs), (ins GPR:$src, rvt_uimm10:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> src;
  bits<10> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0010; // MAT
  let Inst{22-13} = tcsr_addr;
  let Inst{12} = 0b0;
  let Inst{11-7} = src;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecTCSRRead<string asmstr>
  : RVInst<(outs GPR:$rd), (ins rvt_uimm10:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<10> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0011; // VEC
  let Inst{22-13} = tcsr_addr;
  let Inst{12} = 0b1;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecTCSRWrite<string asmstr>
  : RVInst<(outs), (ins GPR:$src, rvt_uimm10:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> src;
  bits<10> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0011; // VEC
  let Inst{22-13} = tcsr_addr;
  let Inst{12} = 0b0;
  let Inst{11-7} = src;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTMacroTCSRRead<string asmstr>
  : RVInst<(outs GPR:$rd), (ins rvt_uimm10:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<10> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0100; // MACRO
  let Inst{22-13} = tcsr_addr;
  let Inst{12} = 0b1;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTMacroTCSRWrite<string asmstr>
  : RVInst<(outs), (ins GPR:$src, rvt_uimm10:$tcsr_addr),
           asmstr, "", [], InstFormatOther> {
  bits<5> src;
  bits<10> tcsr_addr;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0100; // MACRO
  let Inst{22-13} = tcsr_addr;
  let Inst{12} = 0b0;
  let Inst{11-7} = src;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTSBProduce<bits<10> sb_id, string asmstr>
  : RVInst<(outs), (ins GPR:$src), asmstr, "", [], InstFormatOther> {
  bits<5> src;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0000;
  let Inst{22-13} = sb_id;
  let Inst{12} = 0b0;
  let Inst{11-7} = src;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTSBBar<bits<10> sb_id, string asmstr>
  : RVInst<(outs GPR:$rd), (ins), asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0000;
  let Inst{22-13} = sb_id;
  let Inst{12} = 0b1;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTSBGetAny<string asmstr>
  : RVInst<(outs GPR:$rd), (ins), asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-27} = 0b00000;
  let Inst{26-23} = 0b0000;
  let Inst{22-13} = 0b0000000000;
  let Inst{12} = 0b1;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}
class RVTVecVVInt32Bitwise<string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm5:$vrs2, uimm4:$lut),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<5> vrs2;
  bits<4> lut;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b100001;
  let Inst{25} = vb;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = 0b000;
  let Inst{11} = 0b1;
  let Inst{10-7} = lut;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecVVInt32<bits<8> op_v, string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm5:$vrs2),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<5> vrs2;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b100001;
  let Inst{25} = vb;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-7} = op_v;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecVVFP32<bits<5> func5, string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm5:$vrs2, uimm3:$rm),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<5> vrs2;
  bits<3> rm;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b100010;
  let Inst{25} = vb;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-10} = func5;
  let Inst{9-7} = rm;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecVVMux2<string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm5:$vrs2, uimm3:$s2),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<5> vrs2;
  bits<3> s2;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b100011;
  let Inst{25} = vb;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = s2;
  let Inst{11-7} = 0b00000;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecVVCmp<bits<2> dtype_v, string asmstr>
  : RVInst<(outs), (ins uimm5:$vrs1, uimm5:$vrs2, uimm3:$sd, uimm3:$cmp),
           asmstr, "", [], InstFormatOther> {
  bits<5> vrs1;
  bits<5> vrs2;
  bits<3> sd;
  bits<3> cmp;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b100011;
  let Inst{25} = 0b0;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = sd;
  let Inst{11-10} = dtype_v;
  let Inst{9-7} = cmp;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecVInt32<bits<2> op_v, string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm5:$sft),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<5> sft;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b100000;
  let Inst{25} = vb;
  let Inst{24-20} = 0b00001;
  let Inst{19} = 0b0;
  let Inst{18-17} = op_v;
  let Inst{16-12} = sft;
  let Inst{11-7} = vrs1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecVSFU<string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm6:$op),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<6> op;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b100000;
  let Inst{25} = vb;
  let Inst{24-20} = 0b00010;
  let Inst{19-18} = 0b00;
  let Inst{17-12} = op;
  let Inst{11-7} = vrs1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecCvt<bits<3> cvt_v, string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm3:$rm),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<3> rm;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  // Convert encoding from the RVT spec:
  //   [31:26]=100000 prefix, [25]=vb, [24:20]=00010 unit,
  //   [19:18]=01 class, [17:15]=cvt op, [14:12]=rm, [11:7]=vrs1.
  let Inst{31-26} = 0b100000;
  let Inst{25} = vb;
  let Inst{24-20} = 0b00010;
  let Inst{19-18} = 0b01;
  let Inst{17-15} = cvt_v;
  let Inst{14-12} = rm;
  let Inst{11-7} = vrs1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecSInt32Like<bits<2> op_hi, bits<2> op_lo, string asmstr>
  : RVInst<(outs), (ins uimm3:$sd, uimm3:$s1, uimm3:$s2),
           asmstr, "", [], InstFormatOther> {
  bits<3> sd;
  bits<3> s1;
  bits<3> s2;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b111110;
  let Inst{25-23} = sd;
  let Inst{22-16} = 0b0000000;
  let Inst{15} = op_hi{0};
  let Inst{14-12} = s2;
  let Inst{11-10} = op_lo;
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecSGetDim<string asmstr>
  : RVInst<(outs), (ins uimm3:$sd, uimm2:$dim),
           asmstr, "", [], InstFormatOther> {
  bits<3> sd;
  bits<2> dim;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b111110;
  let Inst{25-23} = sd;
  let Inst{22-15} = 0b00000001;
  let Inst{14-12} = 0; // Reserved s2.
  let Inst{11-10} = 0b11;
  let Inst{9} = 0; // Reserved s1[2].
  let Inst{8-7} = dim;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecSIter<bits<2> op_v, string asmstr>
  : RVInst<(outs), (ins uimm3:$sd, uimm3:$s1, uimm3:$s2, uimm2:$sel),
           asmstr, "", [], InstFormatOther> {
  bits<3> sd;
  bits<3> s1;
  bits<3> s2;
  bits<2> sel;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b111110;
  let Inst{25-23} = sd;
  let Inst{22-17} = 0b000000;
  let Inst{16-15} = op_v;
  let Inst{14-12} = s2;
  let Inst{11-10} = sel;
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecSBitwise<string asmstr>
  : RVInst<(outs), (ins uimm3:$sd, uimm3:$s1, uimm3:$s2, uimm4:$lut),
           asmstr, "", [], InstFormatOther> {
  bits<3> sd;
  bits<3> s1;
  bits<3> s2;
  bits<4> lut;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b111110;
  let Inst{25-23} = sd;
  let Inst{22-18} = 0b00000;
  let Inst{17} = 0b1;
  let Inst{16-15} = lut{3-2};
  let Inst{14-12} = s2;
  let Inst{11-10} = lut{1-0};
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecSBranch<bits<2> op_v, string asmstr>
  : RVInst<(outs), (ins rvt_simm8:$target, uimm3:$s1, uimm3:$s2),
           asmstr, "", [], InstFormatOther> {
  bits<8> target;
  bits<3> s1;
  bits<3> s2;
  let hasSideEffects = 1;
  // This branches within a queued VEC block, not the host instruction stream.
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b111111;
  let Inst{25-23} = 0b000;
  let Inst{22-15} = target;
  let Inst{14-12} = s2;
  let Inst{11-10} = op_v;
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecLSULoadRd<bits<3> op_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm1:$vb, uimm2:$sel),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<1> vb;
  bits<2> sel;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 0;
  let Inst{31-26} = 0b010000;
  let Inst{25} = vb;
  let Inst{24-17} = 0;
  let Inst{16-15} = sel;
  let Inst{14-12} = op_v;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecLSULoadS1<string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm2:$sel, uimm3:$s1),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<2> sel;
  bits<3> s1;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 0;
  let Inst{31-26} = 0b010000;
  let Inst{25} = vb;
  let Inst{24-17} = 0;
  let Inst{16-15} = sel;
  let Inst{14-12} = 0b100;
  let Inst{11-10} = 0b00;
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecLSULoadSd<bits<3> op_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm3:$sd, uimm2:$sel),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<3> sd;
  bits<2> sel;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 0;
  let Inst{31-26} = 0b010000;
  let Inst{25-23} = sd;
  let Inst{22-17} = 0;
  let Inst{16-15} = sel;
  let Inst{14-12} = op_v;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecLSUG2S<string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm3:$sd),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<3> sd;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 0;
  let Inst{31-26} = 0b010000;
  let Inst{25-23} = sd;
  let Inst{22-18} = 0;
  let Inst{17} = 0b1;
  let Inst{16-12} = 0;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecLSUMaskedRd<bits<5> op_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm1:$vb, uimm5:$vrs1, uimm3:$s2),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<1> vb;
  bits<5> vrs1;
  bits<3> s2;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 0;
  let Inst{31-26} = 0b010000;
  let Inst{25} = vb;
  let Inst{24-20} = op_v;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = s2;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecLSUMaskedS1<string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm3:$s2, uimm3:$s1),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<3> s2;
  bits<3> s1;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 0;
  let Inst{31-26} = 0b010000;
  let Inst{25} = vb;
  let Inst{24-20} = 0b01010;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = s2;
  let Inst{11-10} = 0b00;
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecLSUStore<bits<3> op_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm5:$vrs1, uimm2:$sel),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> vrs1;
  bits<2> sel;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 1;
  let Inst{31-26} = 0b010001;
  let Inst{25-22} = 0;
  let Inst{21-20} = sel;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = op_v;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecLSUStoreMasked<bits<3> op_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm5:$vrs1, uimm2:$sel, uimm3:$s2),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> vrs1;
  bits<2> sel;
  bits<3> s2;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 1;
  let Inst{31-26} = 0b010001;
  let Inst{25} = 0;
  let Inst{24-22} = op_v;
  let Inst{21-20} = sel;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = s2;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecSparse<bits<6> prefix_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm1:$vb, uimm5:$vrs1, uimm5:$vrs2,
                    uimm3:$s2),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<1> vb;
  bits<5> vrs1;
  bits<5> vrs2;
  bits<3> s2;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{31-26} = prefix_v;
  let Inst{25} = vb;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = s2;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecSparseNoVB<bits<6> prefix_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm5:$vrs1, uimm5:$vrs2, uimm3:$s2),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<5> vrs1;
  bits<5> vrs2;
  bits<3> s2;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{31-26} = prefix_v;
  let Inst{25} = 0;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = s2;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecDyn<bits<6> prefix_v, bits<2> op_v, string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm5:$vrs2, uimm3:$s2,
                    uimm3:$s1),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<5> vrs2;
  bits<3> s2;
  bits<3> s1;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{31-26} = prefix_v;
  let Inst{25} = vb;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = s2;
  let Inst{11-10} = op_v;
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecDynNoVB<bits<6> prefix_v, bits<2> op_v, string asmstr>
  : RVInst<(outs), (ins uimm5:$vrs1, uimm5:$vrs2, uimm3:$s2, uimm3:$s1),
           asmstr, "", [], InstFormatOther> {
  bits<5> vrs1;
  bits<5> vrs2;
  bits<3> s2;
  bits<3> s1;
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let Inst{31-26} = prefix_v;
  let Inst{25} = 0;
  let Inst{24-20} = vrs2;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = s2;
  let Inst{11-10} = op_v;
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecShuffleRd<bits<3> op_v, string asmstr>
  : RVInst<(outs), (ins GPR:$rd, uimm1:$vb, uimm5:$vrs1),
           asmstr, "", [], InstFormatOther> {
  bits<5> rd;
  bits<1> vb;
  bits<5> vrs1;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b011111;
  let Inst{25} = vb;
  let Inst{24-20} = 0;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = op_v;
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_RVT32.Value;
}

class RVTVecShuffleDyn<bits<3> op_v, string asmstr>
  : RVInst<(outs), (ins uimm1:$vb, uimm5:$vrs1, uimm3:$s1),
           asmstr, "", [], InstFormatOther> {
  bits<1> vb;
  bits<5> vrs1;
  bits<3> s1;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
  let Inst{31-26} = 0b011111;
  let Inst{25} = vb;
  let Inst{24-20} = 0;
  let Inst{19-15} = vrs1;
  let Inst{14-12} = op_v;
  let Inst{11-10} = 0b00;
  let Inst{9-7} = s1;
  let Inst{6-0} = OPC_RVT32.Value;
}

let Predicates = [HasTXBaseOrZTXBase] in {
def RVT_SB_GET_ANY : RVTSBGetAny<"tx.sb.get.any $rd">;
} // Predicates = [HasTXBaseOrZTXBase]

let Predicates = [HasTXMatOrZTXMat] in {
def RVT_MAT_TCSR_READ  : RVTMatTCSRRead<"tx.mat.tcsr.read $rd, $tcsr_addr">;
def RVT_MAT_TCSR_WRITE : RVTMatTCSRWrite<"tx.mat.tcsr.write $src, $tcsr_addr">;
def RVT_MAT_SB_PRODUCE : RVTSBProduce<0x002, "tx.mat.sb.produce $src">;
def RVT_MAT_SB_BAR     : RVTSBBar<0x002, "tx.mat.sb.bar $rd">;
} // Predicates = [HasTXMatOrZTXMat]

let Predicates = [HasTXLSUOrZTXLSU] in {
def RVT_LSU_SB_PRODUCE : RVTSBProduce<0x001, "tx.lsu.sb.produce $src">;
def RVT_LSU_SB_BAR     : RVTSBBar<0x001, "tx.lsu.sb.bar $rd">;
} // Predicates = [HasTXLSUOrZTXLSU]

let Predicates = [HasTXMacroOrZTXMacro] in {
def RVT_MACRO_TCSR_READ  : RVTMacroTCSRRead<"tx.macro.tcsr.read $rd, $tcsr_addr">;
def RVT_MACRO_TCSR_WRITE : RVTMacroTCSRWrite<"tx.macro.tcsr.write $src, $tcsr_addr">;
def RVT_MACRO_SB_PRODUCE : RVTSBProduce<0x004, "tx.macro.sb.produce $src">;
def RVT_MACRO_SB_BAR     : RVTSBBar<0x004, "tx.macro.sb.bar $rd">;
} // Predicates = [HasTXMacroOrZTXMacro]

let Predicates = [HasTXVecOrZTXVec] in {
def RVT_VEC_TCSR_READ  : RVTVecTCSRRead<"tx.vec.tcsr.read $rd, $tcsr_addr">;
def RVT_VEC_TCSR_WRITE : RVTVecTCSRWrite<"tx.vec.tcsr.write $src, $tcsr_addr">;
def RVT_VEC_SB_PRODUCE : RVTSBProduce<0x003, "tx.vec.sb.produce $src">;
def RVT_VEC_SB_BAR     : RVTSBBar<0x003, "tx.vec.sb.bar $rd">;
def RVT_VEC_VV_INT32_BITWISE
  : RVTVecVVInt32Bitwise<"tx.vec.vv.int32.bitwise $vb, $vrs1, $vrs2, $lut">;
def RVT_VEC_VV_INT32_ROL : RVTVecVVInt32<0b00000000,
  "tx.vec.vv.int32.rol $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_SLL : RVTVecVVInt32<0b00000001,
  "tx.vec.vv.int32.sll $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_SRA : RVTVecVVInt32<0b00000010,
  "tx.vec.vv.int32.sra $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_SRL : RVTVecVVInt32<0b00000011,
  "tx.vec.vv.int32.srl $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_ADD : RVTVecVVInt32<0b00000100,
  "tx.vec.vv.int32.add $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_SUB : RVTVecVVInt32<0b00000101,
  "tx.vec.vv.int32.sub $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_MULU24 : RVTVecVVInt32<0b00000110,
  "tx.vec.vv.int32.mulu24 $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_MUL24 : RVTVecVVInt32<0b00000111,
  "tx.vec.vv.int32.mul24 $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_MAXU : RVTVecVVInt32<0b00001000,
  "tx.vec.vv.int32.maxu $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_MINU : RVTVecVVInt32<0b00001001,
  "tx.vec.vv.int32.minu $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_MAX : RVTVecVVInt32<0b00001010,
  "tx.vec.vv.int32.max $vb, $vrs1, $vrs2">;
def RVT_VEC_VV_INT32_MIN : RVTVecVVInt32<0b00001011,
  "tx.vec.vv.int32.min $vb, $vrs1, $vrs2">;

def RVT_VEC_VV_FP32_ADD : RVTVecVVFP32<0b00000,
  "tx.vec.vv.fp32.add $vb, $vrs1, $vrs2, $rm">;
def RVT_VEC_VV_FP32_SUB : RVTVecVVFP32<0b00001,
  "tx.vec.vv.fp32.sub $vb, $vrs1, $vrs2, $rm">;
def RVT_VEC_VV_FP32_MUL : RVTVecVVFP32<0b00010,
  "tx.vec.vv.fp32.mul $vb, $vrs1, $vrs2, $rm">;
def RVT_VEC_VV_FP32_MAX : RVTVecVVFP32<0b00100,
  "tx.vec.vv.fp32.max $vb, $vrs1, $vrs2, $rm">;
def RVT_VEC_VV_FP32_MIN : RVTVecVVFP32<0b00101,
  "tx.vec.vv.fp32.min $vb, $vrs1, $vrs2, $rm">;
def RVT_VEC_VV_MUX2 : RVTVecVVMux2<
  "tx.vec.vv.mux2 $vb, $vrs1, $vrs2, $s2">;
def RVT_VEC_VV_INT32_CMP : RVTVecVVCmp<0b01,
  "tx.vec.vv.int32.cmp $vrs1, $vrs2, $sd, $cmp">;
def RVT_VEC_VV_FP32_CMP : RVTVecVVCmp<0b10,
  "tx.vec.vv.fp32.cmp $vrs1, $vrs2, $sd, $cmp">;

def RVT_VEC_V_INT32_SLL : RVTVecVInt32<0b01,
  "tx.vec.v.int32.sll $vb, $vrs1, $sft">;
def RVT_VEC_V_INT32_SRA : RVTVecVInt32<0b10,
  "tx.vec.v.int32.sra $vb, $vrs1, $sft">;
def RVT_VEC_V_INT32_SRL : RVTVecVInt32<0b11,
  "tx.vec.v.int32.srl $vb, $vrs1, $sft">;
def RVT_VEC_V_FLOAT32_SFU : RVTVecVSFU<
  "tx.vec.v.float32.sfu $vb, $vrs1, $op">;

def RVT_VEC_CVT_FP32_INT32 : RVTVecCvt<0b000,
  "tx.vec.cvt.fp32.int32 $vb, $vrs1, $rm">;
def RVT_VEC_CVT_S32_FP32 : RVTVecCvt<0b001,
  "tx.vec.cvt.s32.fp32 $vb, $vrs1, $rm">;
def RVT_VEC_CVT_FP32_FP16 : RVTVecCvt<0b010,
  "tx.vec.cvt.fp32.fp16 $vb, $vrs1, $rm">;
def RVT_VEC_CVT_FP32_BF16 : RVTVecCvt<0b011,
  "tx.vec.cvt.fp32.bf16 $vb, $vrs1, $rm">;
def RVT_VEC_CVT_FP16_FP32 : RVTVecCvt<0b100,
  "tx.vec.cvt.fp16.fp32 $vb, $vrs1, $rm">;
def RVT_VEC_CVT_BF16_FP32 : RVTVecCvt<0b101,
  "tx.vec.cvt.bf16.fp32 $vb, $vrs1, $rm">;
def RVT_VEC_CVT_MXFP8_BF16 : RVTVecCvt<0b110,
  "tx.vec.cvt.mxfp8.bf16 $vb, $vrs1, $rm">;
def RVT_VEC_CVT_MXFP4_BF16 : RVTVecCvt<0b111,
  "tx.vec.cvt.mxfp4.bf16 $vb, $vrs1, $rm">;

def RVT_VEC_S_INT32_ROL : RVTVecSInt32Like<0b00, 0b00,
  "tx.vec.s.int32.rol $sd, $s1, $s2">;
def RVT_VEC_S_INT32_SLL : RVTVecSInt32Like<0b00, 0b01,
  "tx.vec.s.int32.sll $sd, $s1, $s2">;
def RVT_VEC_S_INT32_SRA : RVTVecSInt32Like<0b00, 0b10,
  "tx.vec.s.int32.sra $sd, $s1, $s2">;
def RVT_VEC_S_INT32_SRL : RVTVecSInt32Like<0b00, 0b11,
  "tx.vec.s.int32.srl $sd, $s1, $s2">;
def RVT_VEC_S_INT32_ADD : RVTVecSInt32Like<0b01, 0b00,
  "tx.vec.s.int32.add $sd, $s1, $s2">;
def RVT_VEC_S_INT32_SUB : RVTVecSInt32Like<0b01, 0b01,
  "tx.vec.s.int32.sub $sd, $s1, $s2">;
def RVT_VEC_S_INT32_MUL : RVTVecSInt32Like<0b01, 0b10,
  "tx.vec.s.int32.mul $sd, $s1, $s2">;
def RVT_VEC_S_ITER_GETDIM : RVTVecSGetDim<
  "tx.vec.s.iter.getdim $sd, $dim">;
def RVT_VEC_S_ITER_GETS : RVTVecSIter<0b10,
  "tx.vec.s.iter.gets $sd, $s1, $s2, $sel">;
def RVT_VEC_S_ITER_GETV : RVTVecSIter<0b11,
  "tx.vec.s.iter.getv $sd, $s1, $s2, $sel">;
def RVT_VEC_S_BITWISE : RVTVecSBitwise<
  "tx.vec.s.bitwise $sd, $s1, $s2, $lut">;
def RVT_VEC_S_BITWISE_BEQ : RVTVecSBranch<0b00,
  "tx.vec.s.bitwise.beq $target, $s1, $s2">;
def RVT_VEC_S_BITWISE_BNE : RVTVecSBranch<0b01,
  "tx.vec.s.bitwise.bne $target, $s1, $s2">;
def RVT_VEC_S_INT32_BLT : RVTVecSBranch<0b10,
  "tx.vec.s.int32.blt $target, $s1, $s2">;
def RVT_VEC_S_INT32_BGE : RVTVecSBranch<0b11,
  "tx.vec.s.int32.bge $target, $s1, $s2">;

def RVT_VEC_LSU_U2V : RVTVecLSULoadRd<0b000,
  "tx.vec.lsu.u2v $rd, $vb, $sel">;
def RVT_VEC_LSU_SM2V : RVTVecLSULoadRd<0b001,
  "tx.vec.lsu.sm2v $rd, $vb, $sel">;
def RVT_VEC_LSU_U2V_BROADCAST32 : RVTVecLSULoadRd<0b010,
  "tx.vec.lsu.u2v.broadcast32 $rd, $vb, $sel">;
def RVT_VEC_LSU_SM2V_BROADCAST32 : RVTVecLSULoadRd<0b011,
  "tx.vec.lsu.sm2v.broadcast32 $rd, $vb, $sel">;
def RVT_VEC_LSU_S2V_BROADCAST32 : RVTVecLSULoadS1<
  "tx.vec.lsu.s2v.broadcast32 $vb, $sel, $s1">;
def RVT_VEC_LSU_G2V_BROADCAST32 : RVTVecLSULoadRd<0b101,
  "tx.vec.lsu.g2v.broadcast32 $rd, $vb, $sel">;
def RVT_VEC_LSU_U2S : RVTVecLSULoadSd<0b110,
  "tx.vec.lsu.u2s $rd, $sd, $sel">;
def RVT_VEC_LSU_SM2S : RVTVecLSULoadSd<0b111,
  "tx.vec.lsu.sm2s $rd, $sd, $sel">;
def RVT_VEC_LSU_G2S : RVTVecLSUG2S<"tx.vec.lsu.g2s $rd, $sd">;

def RVT_VEC_LSU_U2V_MASK : RVTVecLSUMaskedRd<0b00001,
  "tx.vec.lsu.u2v.mask $rd, $vb, $vrs1, $s2">;
def RVT_VEC_LSU_SM2V_MASK : RVTVecLSUMaskedRd<0b00011,
  "tx.vec.lsu.sm2v.mask $rd, $vb, $vrs1, $s2">;
def RVT_VEC_LSU_U2V_MASK_BROADCAST32 : RVTVecLSUMaskedRd<0b00101,
  "tx.vec.lsu.u2v.mask.broadcast32 $rd, $vb, $vrs1, $s2">;
def RVT_VEC_LSU_SM2V_MASK_BROADCAST32 : RVTVecLSUMaskedRd<0b00111,
  "tx.vec.lsu.sm2v.mask.broadcast32 $rd, $vb, $vrs1, $s2">;
def RVT_VEC_LSU_S2V_MASK_BROADCAST32 : RVTVecLSUMaskedS1<
  "tx.vec.lsu.s2v.mask.broadcast32 $vb, $vrs1, $s2, $s1">;
def RVT_VEC_LSU_G2V_MASK_BROADCAST32 : RVTVecLSUMaskedRd<0b01001,
  "tx.vec.lsu.g2v.mask.broadcast32 $rd, $vb, $vrs1, $s2">;

def RVT_VEC_LSU_V2U : RVTVecLSUStore<0b000,
  "tx.vec.lsu.v2u $rd, $vrs1, $sel">;
def RVT_VEC_LSU_V2SM : RVTVecLSUStore<0b001,
  "tx.vec.lsu.v2sm $rd, $vrs1, $sel">;
def RVT_VEC_LSU_S2U : RVTVecLSUStore<0b010,
  "tx.vec.lsu.s2u $rd, $vrs1, $sel">;
def RVT_VEC_LSU_S2SM : RVTVecLSUStore<0b011,
  "tx.vec.lsu.s2sm $rd, $vrs1, $sel">;
def RVT_VEC_LSU_V2U_MASK : RVTVecLSUStoreMasked<0b001,
  "tx.vec.lsu.v2u.mask $rd, $vrs1, $sel, $s2">;
def RVT_VEC_LSU_V2SM_MASK : RVTVecLSUStoreMasked<0b010,
  "tx.vec.lsu.v2sm.mask $rd, $vrs1, $sel, $s2">;

def RVT_VEC_SM2V_SPARSE : RVTVecSparse<0b010010,
  "tx.vec.sm2v.sparse $rd, $vb, $vrs1, $vrs2, $s2">;
def RVT_VEC_V2SM_SPARSE : RVTVecSparseNoVB<0b010011,
  "tx.vec.v2sm.sparse $rd, $vrs1, $vrs2, $s2">;
def RVT_VEC_V2SM_SPARSE_INT32_ADD : RVTVecSparse<0b010100,
  "tx.vec.v2sm.sparse.int32.add $rd, $vb, $vrs1, $vrs2, $s2">;
def RVT_VEC_V2SM_SPARSE_INT32_AND : RVTVecSparse<0b010101,
  "tx.vec.v2sm.sparse.int32.and $rd, $vb, $vrs1, $vrs2, $s2">;
def RVT_VEC_V2SM_SPARSE_INT32_OR : RVTVecSparse<0b010110,
  "tx.vec.v2sm.sparse.int32.or $rd, $vb, $vrs1, $vrs2, $s2">;
def RVT_VEC_V2SM_SPARSE_INT32_XOR : RVTVecSparse<0b010111,
  "tx.vec.v2sm.sparse.int32.xor $rd, $vb, $vrs1, $vrs2, $s2">;

def RVT_VEC_SM2V_DYN : RVTVecDyn<0b011000, 0b00,
  "tx.vec.sm2v.dyn $vb, $vrs1, $vrs2, $s2, $s1">;
def RVT_VEC_V2SM_DYN : RVTVecDynNoVB<0b011000, 0b01,
  "tx.vec.v2sm.dyn $vrs1, $vrs2, $s2, $s1">;
def RVT_VEC_V2SM_DYN_INT32_ADD : RVTVecDyn<0b011000, 0b10,
  "tx.vec.v2sm.dyn.int32.add $vb, $vrs1, $vrs2, $s2, $s1">;
def RVT_VEC_V2SM_DYN_INT32_AND : RVTVecDyn<0b011000, 0b11,
  "tx.vec.v2sm.dyn.int32.and $vb, $vrs1, $vrs2, $s2, $s1">;
def RVT_VEC_V2SM_DYN_INT32_OR : RVTVecDyn<0b011001, 0b00,
  "tx.vec.v2sm.dyn.int32.or $vb, $vrs1, $vrs2, $s2, $s1">;
def RVT_VEC_V2SM_DYN_INT32_XOR : RVTVecDyn<0b011001, 0b01,
  "tx.vec.v2sm.dyn.int32.xor $vb, $vrs1, $vrs2, $s2, $s1">;

def RVT_VEC_SHUFFLE_XOR : RVTVecShuffleRd<0b000,
  "tx.vec.shuffle.xor $rd, $vb, $vrs1">;
def RVT_VEC_SHUFFLE_XOR_DYN : RVTVecShuffleDyn<0b001,
  "tx.vec.shuffle.xor.dyn $vb, $vrs1, $s1">;
def RVT_VEC_SHUFFLE_ROR : RVTVecShuffleRd<0b010,
  "tx.vec.shuffle.ror $rd, $vb, $vrs1">;
def RVT_VEC_SHUFFLE_ROR_DYN : RVTVecShuffleDyn<0b011,
  "tx.vec.shuffle.ror.dyn $vb, $vrs1, $s1">;
def RVT_VEC_SHUFFLE_BROADCAST : RVTVecShuffleRd<0b100,
  "tx.vec.shuffle.broadcast $rd, $vb, $vrs1">;
def RVT_VEC_SHUFFLE_BROADCAST_DYN : RVTVecShuffleDyn<0b101,
  "tx.vec.shuffle.broadcast.dyn $vb, $vrs1, $s1">;
def RVT_VEC_SHUFFLE_COMPRESS : RVTVecShuffleRd<0b110,
  "tx.vec.shuffle.compress $rd, $vb, $vrs1">;
def RVT_VEC_SHUFFLE_COMPRESS_DYN : RVTVecShuffleDyn<0b111,
  "tx.vec.shuffle.compress.dyn $vb, $vrs1, $s1">;
} // Predicates = [HasTXVecOrZTXVec]

// Macro.
// Instruction classes set hasSideEffects themselves; InstAlias has no such field.
let Predicates = [HasTXMacroOrZTXMacro] in {
// FP32, S32 and U32 elementwise operations with independent destination pointers.
defm RVT_MACRO_ELEMWISE_ADD_FP32 :
  RVTMacroElemwiseForms<0, 0x00, "add.fp32">;
defm RVT_MACRO_ELEMWISE_SUB_FP32 :
  RVTMacroElemwiseForms<1, 0x00, "sub.fp32">;
defm RVT_MACRO_ELEMWISE_MUL_FP32 :
  RVTMacroElemwiseForms<2, 0x00, "mul.fp32">;
defm RVT_MACRO_ELEMWISE_MAX_FP32 :
  RVTMacroElemwiseForms<4, 0x00, "max.fp32">;
defm RVT_MACRO_ELEMWISE_MIN_FP32 :
  RVTMacroElemwiseForms<5, 0x00, "min.fp32">;
defm RVT_MACRO_ELEMWISE_ADD_S32 :
  RVTMacroElemwiseForms<0, 0x10, "add.s32">;
defm RVT_MACRO_ELEMWISE_SUB_S32 :
  RVTMacroElemwiseForms<1, 0x10, "sub.s32">;
defm RVT_MACRO_ELEMWISE_MUL_S32 :
  RVTMacroElemwiseForms<2, 0x10, "mul.s32">;
defm RVT_MACRO_ELEMWISE_MAX_S32 :
  RVTMacroElemwiseForms<4, 0x10, "max.s32">;
defm RVT_MACRO_ELEMWISE_MIN_S32 :
  RVTMacroElemwiseForms<5, 0x10, "min.s32">;

defm RVT_MACRO_ELEMWISE_ADD_U32 :
  RVTMacroElemwiseForms<0, 0x11, "add.u32">;
defm RVT_MACRO_ELEMWISE_SUB_U32 :
  RVTMacroElemwiseForms<1, 0x11, "sub.u32">;
defm RVT_MACRO_ELEMWISE_MUL_U32 :
  RVTMacroElemwiseForms<2, 0x11, "mul.u32">;
defm RVT_MACRO_ELEMWISE_MAX_U32 :
  RVTMacroElemwiseForms<4, 0x11, "max.u32">;
defm RVT_MACRO_ELEMWISE_MIN_U32 :
  RVTMacroElemwiseForms<5, 0x11, "min.u32">;

def RVT_MACRO_SHUFFLE_COMPRESS : RVTMacroShuffleCompress<
  "tx.macro.shuffle.compress $rd, $rs1, $rs2, $group">;

def RVT_MACRO_REDUCE_ADD_S32 : RVTMacroReduce<0, 0b000000, 0x10,
  "tx.macro.reduce.add.s32 $rd, $rs1, $group">;

// Conversion suffixes are destination.source. FP8 and MXFP8 use E4M3.
def RVT_MACRO_CONVERT_FP32_S32 : RVTMacroConvert<0x00, 0x10,
  "tx.macro.convert.fp32.s32 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_S32_FP32 : RVTMacroConvert<0x10, 0x00,
  "tx.macro.convert.s32.fp32 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_S16_S32 : RVTMacroConvert<0x12, 0x10,
  "tx.macro.convert.s16.s32 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_BF16_FP32 : RVTMacroConvert<0x02, 0x00,
  "tx.macro.convert.bf16.fp32 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_FP32_BF16 : RVTMacroConvert<0x00, 0x02,
  "tx.macro.convert.fp32.bf16 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_S32_S16 : RVTMacroConvert<0x10, 0x12,
  "tx.macro.convert.s32.s16 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_MXFP8_BF16 : RVTMacroConvert<0x06, 0x02,
  "tx.macro.convert.mxfp8.bf16 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_S8_S16 : RVTMacroConvert<0x14, 0x12,
  "tx.macro.convert.s8.s16 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_S16_S8 : RVTMacroConvert<0x12, 0x14,
  "tx.macro.convert.s16.s8 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_BF16_S8 : RVTMacroConvert<0x02, 0x14,
  "tx.macro.convert.bf16.s8 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_BF16_U8 : RVTMacroConvert<0x02, 0x15,
  "tx.macro.convert.bf16.u8 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_FP8_S4 : RVTMacroConvert<0x04, 0x18,
  "tx.macro.convert.fp8.s4 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_FP8_U4 : RVTMacroConvert<0x04, 0x19,
  "tx.macro.convert.fp8.u4 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_FP32_FP8 : RVTMacroConvert<0x00, 0x04,
  "tx.macro.convert.fp32.fp8 $rd, $rs1, $group">;
def RVT_MACRO_CONVERT_FP32_FP16 : RVTMacroConvert<0x00, 0x03,
  "tx.macro.convert.fp32.fp16 $rd, $rs1, $group">;

def RVT_MACRO_REDUCE_MAX_FP32 : RVTMacroReduce3P<0b000100, 0x00,
  "tx.macro.reduce.max.fp32 $rd, $rs1, $rs2, $group, $index">;
def RVT_MACRO_REDUCE_MIN_FP32 : RVTMacroReduce3P<0b000101, 0x00,
  "tx.macro.reduce.min.fp32 $rd, $rs1, $rs2, $group, $index">;
def RVT_MACRO_REDUCE_ADD_FP32 : RVTMacroReduce<0, 0b000000, 0x00,
  "tx.macro.reduce.add.fp32 $rd, $rs1, $group">;
def RVT_MACRO_REDUCE_MAX_S32 : RVTMacroReduce3P<0b000100, 0x10,
  "tx.macro.reduce.max.s32 $rd, $rs1, $rs2, $group, $index">;
def RVT_MACRO_REDUCE_MIN_S32 : RVTMacroReduce3P<0b000101, 0x10,
  "tx.macro.reduce.min.s32 $rd, $rs1, $rs2, $group, $index">;
def RVT_MACRO_SFU_LOG2_FP32  : RVTMacroSFU<0x00, "tx.macro.sfu.log2 $rd, $rs1, $group">;
def RVT_MACRO_SFU_EXP2_FP32  : RVTMacroSFU<0x01, "tx.macro.sfu.exp2 $rd, $rs1, $group">;
def RVT_MACRO_SFU_SIN_FP32   : RVTMacroSFU<0x02, "tx.macro.sfu.sin $rd, $rs1, $group">;
def RVT_MACRO_SFU_COS_FP32   : RVTMacroSFU<0x03, "tx.macro.sfu.cos $rd, $rs1, $group">;
def RVT_MACRO_SFU_RSQRT_FP32 : RVTMacroSFU<0x04, "tx.macro.sfu.rsqrt $rd, $rs1, $group">;
def RVT_MACRO_SFU_RECIP_FP32 : RVTMacroSFU<0x05, "tx.macro.sfu.recip $rd, $rs1, $group">;
}

// Alias-only mnemonics that intentionally share encoding with canonical forms.
// Keep parser compatibility without introducing duplicate decode table entries.
let Predicates = [HasTXMacroOrZTXMacro] in {
def : InstAlias<"tx.macro.sfu.log2.fp32 $rd, $rs1, $group",
                (RVT_MACRO_SFU_LOG2_FP32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
def : InstAlias<"tx.macro.sfu.exp2.fp32 $rd, $rs1, $group",
                (RVT_MACRO_SFU_EXP2_FP32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
def : InstAlias<"tx.macro.sfu.sin.fp32 $rd, $rs1, $group",
                (RVT_MACRO_SFU_SIN_FP32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
def : InstAlias<"tx.macro.sfu.cos.fp32 $rd, $rs1, $group",
                (RVT_MACRO_SFU_COS_FP32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
def : InstAlias<"tx.macro.sfu.rsqrt.fp32 $rd, $rs1, $group",
                (RVT_MACRO_SFU_RSQRT_FP32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
def : InstAlias<"tx.macro.sfu.recip.fp32 $rd, $rs1, $group",
                (RVT_MACRO_SFU_RECIP_FP32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
// Historical spelling of fp32.
def : InstAlias<"tx.macro.convert.s32.f32 $rd, $rs1, $group",
                (RVT_MACRO_CONVERT_S32_FP32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
def : InstAlias<"tx.macro.sfu.exp $rd, $rs1, $group",
                (RVT_MACRO_SFU_EXP2_FP32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
def : InstAlias<"tx.macro.convert.fp32.s32.round $rd, $rs1, $group",
                (RVT_MACRO_CONVERT_FP32_S32 GPR:$rd, GPR:$rs1, uimm3:$group), 0>;
} // Predicates = [HasTXMacroOrZTXMacro]
