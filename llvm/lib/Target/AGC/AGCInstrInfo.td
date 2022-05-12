//===-- AGCInstrInfo.td - Target Description for AGC -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Declarations that describe the AGC instruction set
//
//===----------------------------------------------------------------------===//

include "AGCInstrFormats.td"

//===----------------------------------------------------------------------===//
// AGC specific DAG nodes.
//===----------------------------------------------------------------------===//

// Target-independent type requirements, but with target-specific formats.
def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i16>,
                                       SDTCisVT<1, i16>]>;
def SDT_CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i16>,
                                     SDTCisVT<1, i16>]>;


// Target-dependent type requirements.
def SDT_AGCCall : SDTypeProfile<0, -1, [SDTCisVT<0, i16>]>;

// Target-independent nodes, but with target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// Target-dependent nodes.
def Call : SDNode<"AGCISD::CALL", SDT_AGCCall,
                  [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;
def RetFlag : SDNode<"AGCISD::RET_FLAG", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

//===----------------------------------------------------------------------===//
// Operand and SDNode transformation definitions
//===----------------------------------------------------------------------===//

class AGCAsmOperand<string N> : AsmOperandClass {
  let Name = N;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

// A 12 bit address into general purpose memory, encompassing erasable and
// read-only memory locations.
def mem12 : Operand<i16>, ImmLeaf<i16, [{return isUInt<12>(Imm);}]> {
  let ParserMatchClass = AGCAsmOperand<"Mem12">;
  let DecoderMethod = "decodeMem12Operand";
}

// A 10 bit address into general purpose memory, encompassing only erasable
// memory locations.
def mem10 : Operand<i16>, ImmLeaf<i16, [{return isUInt<10>(Imm);}]> {
  let ParserMatchClass = AGCAsmOperand<"Mem10">;
  let DecoderMethod = "decodeMem10Operand";
}

// A 9 bit address into IO channel memory.
def io9 : Operand<i16>, ImmLeaf<i16, [{return isUInt<9>(Imm);}]> {
  let ParserMatchClass = AGCAsmOperand<"IO9">;
  let DecoderMethod = "decodeIO9Operand";
}

// A symbol that cannot be a constant immediate.
def bare_symbol : Operand<i16> {
  let ParserMatchClass = AGCAsmOperand<"BareSymbol">;
}

//===----------------------------------------------------------------------===//
// Instruction class templates
//===----------------------------------------------------------------------===//

// Base instruction for defining instructions which read in from a general
// purpose memory location.
class ALUReadInst<bits<3> opcode, dag outs, dag ins, string opcodestr,
                  list<dag> pattern = []>
    : AGCInst12<opcode, outs, ins, opcodestr, pattern> {
  let hasSideEffects = 0;
  let mayLoad = 1;
  let mayStore = 0;
}

// Base instruction for defining instructions which read in from a general
// purpose memory location - pseudo (register only) version.
class ALUReadPseudo<dag outs, dag ins, string opcodestr, list<dag> pattern = []>
    : Pseudo<outs, ins, opcodestr, "$k", pattern> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let Size = 2;
  let isCodeGenOnly = 0;
  let isAsmParserOnly = 1;
}

// Base instruction for defining instructions which write out to a general
// purpose memory location.
class ALUWriteInst<bits<5> opcode, dag outs, dag ins, string opcodestr,
                   list<dag> pattern = []>
    : AGCInst10<opcode, outs, ins, opcodestr, pattern> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 1;
}

// Base instruction for defining instructions which write out to a general
// purpose memory location - pseudo (register only) version.
class ALUWritePseudo<dag outs, dag ins, string opcodestr,
                     list<dag> pattern = []>
    : Pseudo<outs, ins, opcodestr, "$k", pattern> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let Size = 2;
  let isCodeGenOnly = 0;
  let isAsmParserOnly = 1;
}

// Base instruction for defining instructions which transfer control within the
// program.
class TCInst<bits<3> opcode, dag outs, dag ins, string opcodestr,
             list<dag> pattern = []>
    : AGCInst12<opcode, outs, ins, opcodestr, pattern> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let isBranch = 1;
  let isBarrier = 1;
  let isTerminator = 1;
}

// Base instruction for defining conditional branch instructions.
class BranchInst<bits<3> opcode, dag outs, dag ins, string opcodestr,
             list<dag> pattern = []>
    : AGCInst12<opcode, outs, ins, opcodestr, pattern> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let isBranch = 1;
  let isTerminator = 1;
}

// Base instruction for defining instructions which read in from an IO channel.
class IOReadInst<bits<6> opcode, dag outs, dag ins, string opcodestr,
                 list<dag> pattern = []>
    : AGCInst9<opcode, outs, ins, opcodestr, pattern> {
  let hasSideEffects = 0;
  let mayLoad = 1;
  let mayStore = 0;
  let isExtracode = 1;
  let DecoderNamespace = "Extracode";
}

// Base instruction for defining instructions which write out to an IO channel.
class IOWriteInst<bits<6> opcode, dag outs, dag ins, string opcodestr,
                  list<dag> pattern = []>
    : AGCInst9<opcode, outs, ins, opcodestr, pattern> {
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 1;
  let isExtracode = 1;
  let DecoderNamespace = "Extracode";
}

// Base instruction for defining instructions which read in from an IO channel
// and write out to the same IO channel.
class IOReadWriteInst<bits<6> opcode, dag outs, dag ins, string opcodestr,
                      list<dag> pattern = []>
    : AGCInst9<opcode, outs, ins, opcodestr, pattern> {
  let hasSideEffects = 1;
  let mayLoad = 1;
  let mayStore = 1;
  let isExtracode = 1;
  let DecoderNamespace = "Extracode";
}

//===----------------------------------------------------------------------===//
// EXTEND instruction
//===----------------------------------------------------------------------===//

// The extend instruction is used to indicate that the next instruction is to be
// interpreted as an extracode instruction, and to inform the assembler that the
// next instruction should appear as an extracode instruction.
def EXTEND : AGCInst0<0b000000000000110, "extend">;

//===----------------------------------------------------------------------===//
// Index instruction
//===----------------------------------------------------------------------===//

// The index instruction is used to directly modify the operand of the following
// instruction, by adding the value in the given memory location to the encoding
// of the instruction which comes after this one.
// TODO: It should preserve extracode state set by an extend occurring before it
// - so it needs to be a valid extracode as well as a valid non-extracode.
let Constraints = "$k = $k_wb" in
def INDEX : AGCInst10<0b10100, (outs GPR:$k_wb), (ins GPR:$k), "index">;

//===----------------------------------------------------------------------===//
// Basic arithmetic instructions
//===----------------------------------------------------------------------===//

// Add value of general purpose memory location to accumulator. This instruction
// writes back the original value of the memory location, so memory locations
// which edit their contents are updated. Note that this occurs even though the
// operand is not necessarily erasable.
let isAdd = 1 in {
let mayStore = 1 in
def AD : ALUReadInst<0b110, (outs A:$a_dst), (ins mem12:$k_wb, A:$a, mem12:$k),
                     "ad">;

let Constraints = "$k = $k_wb" in
def PseudoAD : ALUReadPseudo<(outs A:$a_dst, GPR:$k_wb), (ins A:$a, GPR:$k),
                             "ad">;
} // AD

// Double precision add value of accumulator to general purpose memory location.
// This instruction writes the sign of the output to the accumulator, and writes
// zero to the lower accumulator.
let isAdd = 1 in {
let mayLoad = 1 in
def DAS : ALUWriteInst<0b01000, (outs AL:$al_wb),
                       (ins mem10:$k_dst, mem10:$k, AL:$al), "das">;

let Constraints = "$k = $k_dst" in
def PseudoDAS : ALUWritePseudo<(outs AL:$al_wb, GPRD:$k_dst),
                               (ins GPRD:$k, AL:$al), "das">;
} // DAS

// Subtract value of general purpose memory location from accumulator. This
// instruction writes back the original value of the memory location, so memory
// locations which edit their contents are updated.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
let mayLoad = 1 in
def SU : ALUWriteInst<0b11000, (outs A:$a_dst),
                      (ins mem10:$k_wb, A:$a, mem10:$k), "su">;

let Constraints = "$k = $k_wb" in
def PseudoSU : ALUWritePseudo<(outs A:$a_dst, GPR:$k_wb), (ins A:$a, GPR:$k),
                              "su">;
} // SU

// Modular subtract value of general purpose memory location from accumulator.
// This instruction writes back the original value of the memory location, so
// memory locations which edit their contents are updated.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
let mayLoad = 1 in
def MSU : ALUWriteInst<0b01000, (outs A:$a_dst),
                       (ins mem10:$k_wb, A:$a, mem10:$k), "msu">;

let Constraints = "$k = $k_wb" in
def PseudoMSU : ALUWritePseudo<(outs A:$a_dst, GPR:$k_wb), (ins A:$a, GPR:$k),
                               "msu">;
} // MSU

// And value of general purpose memory location with value of accumulator into
// accumulator.
def MASK : ALUReadInst<0b111, (outs A:$a_dst), (ins A:$a, mem12:$k), "mask">;

def PseudoMASK : ALUReadPseudo<(outs A:$a_dst), (ins A:$a, GPR:$k), "mask">;

// Douple precision multiply value of general purpose memory location with
// accumulator. This instruction stores the result of the multiplication in both
// the accumulator and the lower accumulator.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
def MP : ALUReadInst<0b111, (outs AL:$a_dst), (ins A:$a, mem12:$k), "mp">;

def PseudoMP : ALUReadPseudo<(outs AL:$a_dst), (ins A:$a, GPR:$k), "mp">;
} // MP

// Double precision divide value of the accumulator and lower accumulator pair
// by general purpose memory location. This instruction stores the result of the
// division in the accumulator and any remainder in the lower accumulator. Note
// that if the divisor is not larger than the value in the accumulator the
// result will not fit in a single memory location. Also note that if the
// divisor is larger than the value of the accumulator and lower accumulator
// pair then the result is completely undefined. Note that even though the
// operand is erasable, no editing or write back occurs.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
let mayLoad = 1, mayStore = 0 in
def DV : ALUWriteInst<0b00100, (outs A:$a_dst, L:$l_dst),
                      (ins AL:$a, mem10:$k), "dv">;

def PseudoDV : ALUWritePseudo<(outs A:$a_dst, L:$l_dst), (ins AL:$a, GPRD:$k),
                              "dv">;
} // DV



// Increment general purpose memory location by 1.
let mayLoad = 1 in
def INCR : ALUWriteInst<0b01010, (outs), (ins mem10:$k_dst, mem10:$k), "incr">;

let Constraints = "$k = $k_dst" in
def PseudoINCR : ALUWritePseudo<(outs GPR:$k_dst), (ins GPR:$k), "incr">;

// Add value of accumulator to general purpose memory location.
let mayLoad = 1 in
def ADS : ALUWriteInst<0b01011, (outs), (ins mem10:$k_dst, mem10:$k, A:$a),
                       "ads">;

let Constraints = "$k = $k_dst" in
def PseudoADS : ALUWritePseudo<(outs GPR:$k_dst), (ins GPR:$k, A:$a), "ads">;

// 'Diminish' general purpose memory location by 1. This instruction either adds
// 1 to a negative value or subtracts 1 from a positive value. If the value is
// zero, it is unchanged.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
let mayLoad = 1 in
def DIM : ALUWriteInst<0b01011, (outs), (ins mem10:$k_dst, mem10:$k), "dim">;

let Constraints = "$k = $k_dst" in
def PseudoDIM : ALUWritePseudo<(outs GPR:$k_dst), (ins GPR:$k), "dim">;
} // DIM

// 'Augment' general purpose memory location by 1. This instruction either adds
// 1 to a positive value or subtracts 1 from a negative value. If the value is
// zero, it is augmented according to its sign.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
let mayLoad = 1 in
def AUG : ALUWriteInst<0b01010, (outs), (ins mem10:$k_dst, mem10:$k), "aug">;

let Constraints = "$k = $k_dst" in
def PseudoAUG : ALUWritePseudo<(outs GPR:$k_dst), (ins GPR:$k), "aug">;
} // AUG

// Copy value of general purpose memory location to accumulator. This
// instruction writes back the original value of the memory location, so memory
// locations which edit their contents are updated. Note that this occurs even
// though the operand is not necessarily erasable.
let mayStore = 1 in
def CA : ALUReadInst<0b011, (outs A:$a_dst), (ins mem12:$k_wb, mem12:$k), "ca">;

let Constraints = "$k = $k_wb" in
def PseudoCA : ALUReadPseudo<(outs A:$a_dst, GPR:$k_wb), (ins GPR:$k), "ca">;

// Double precision copy general purpose memory location pair to accumulator and
// lower accumulator pair. This instruction writes back the original value of
// the memory location, so memory locations which edit their contents are
// updated. Note that this occurs even though the operand is not necessarily
// erasable.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
let mayStore = 1 in
def DCA : ALUReadInst<0b011, (outs AL:$a_dst), (ins mem12:$k_wb, mem12:$k),
                      "dca">;

let Constraints = "$k = $k_wb" in
def PseudoDCA : ALUReadPseudo<(outs AL:$a_dst, GPRD:$k_wb), (ins GPRD:$k),
                              "dca">;
} // DCA

// Copy inverted value of general purpose memory location to accumulator. This
// instruction writes back the original value of the memory location, so memory
// locations which edit their contents are updated. Note that this occurs even
// though the operand is not necessarily erasable.
let mayStore = 1 in
def CS : ALUReadInst<0b100, (outs A:$a_dst), (ins mem12:$k_wb, mem12:$k), "cs">;

let Constraints = "$k = $k_wb" in
def PseudoCS : ALUReadPseudo<(outs A:$a_dst, GPR:$k_wb), (ins GPR:$k), "cs">;

// Double precision copy inverted value of general purpose memory location pair
// to accumulator pair. This instruction writes back the original value of the
// memory location, so memory locations which edit their contents are updated.
// Note that this occurs even though the operand is not necessarily erasable.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
let mayStore = 1 in
def DCS : ALUReadInst<0b100, (outs AL:$a_dst), (ins mem12:$k_wb, mem12:$k),
                      "dcs">;

let Constraints = "$k = $k_wb" in
def PseudoDCS : ALUReadPseudo<(outs AL:$a_dst, GPRD:$k_wb), (ins GPRD:$k),
                              "dcs">;
} // DCS


// Transfer value of accumulator to general purpose memory location. This
// instruction corrects overflow of the accumulator value if the destination is
// not also a 16 bit memory location. If the accumulator contained overflow, the
// accumulator is set to +1 or -1 depending on the sign of the overflow.
def TS : ALUWriteInst<0b10110, (outs A:$a_wb), (ins mem10:$k, A:$a), "ts">;

def PseudoTS : ALUWritePseudo<(outs A:$a_wb, GPR:$k), (ins A:$a), "ts">;

// Exchange accumulator and general purpose memory location. This instruction
// corrects overflow of the accumulator value if the destination is not also a
// 16 bit memory location.
let mayLoad = 1 in
def XCH : ALUWriteInst<0b10111, (outs A:$a_dst),
                       (ins mem10:$k_dst, A:$a, mem10:$k), "xch">;

let Constraints = "$k = $k_dst" in
def PseudoXCH : ALUWritePseudo<(outs A:$a_dst, GPR:$k_dst), (ins A:$a, GPR:$k),
                               "xch">;

// Exchange lower accumulator and general purpose memory location. This
// instruction corrects overflow of the destination value if it is a 16 bit
// memory location.
let mayLoad = 1 in
def LXCH : ALUWriteInst<0b01001, (outs L:$l_dst),
                        (ins mem10:$k_dst, L:$l, mem10:$k), "lxch">;

let Constraints = "$k = $k_dst" in
def PseudoLXCH : ALUWritePseudo<(outs L:$l_dst, GPR:$k_dst), (ins L:$l, GPR:$k),
                                "lxch">;

// Exchange return address and general purpose memory location. This instruction
// corrects overflow of the return address value if the destination is not also
// a 16 bit memory location.
let isExtracode = 1, DecoderNamespace = "Extracode" in {
let mayLoad = 1 in
def QXCH : ALUWriteInst<0b01001, (outs Q:$q_dst),
                        (ins  mem10:$k_dst, Q:$q, mem10:$k), "qxch">;

let Constraints = "$k = $k_dst" in
def PseudoQXCH : ALUWritePseudo<(outs Q:$q_dst, GPR:$k_dst), (ins Q:$q, GPR:$k),
                                "qxch">;
} // QXCH

// Double precision exchange accumulator and lower accumulator pair and general
// purpose memory location pair. This instruction corrects only the overflow of
// the accumulator value if the destination is not also a 16 bit memory
// location.
let mayLoad = 1 in
def DXCH : ALUWriteInst<0b10101, (outs AL:$a_dst),
                        (ins mem10:$k_dst, AL:$a, mem10:$k), "dxch">;

let Constraints = "$k = $k_dst" in
def PseudoDXCH : ALUWritePseudo<(outs AL:$a_dst, GPRD:$k_dst),
                                (ins AL:$a, GPRD:$k), "dxch">;

//===----------------------------------------------------------------------===//
// Control flow instructions
//===----------------------------------------------------------------------===//

// Transfer control to address, saving the return address. Note that the address
// cannot reference the return address register, the erasable bank select
// register, the fixed bank select register or the both banks select register.
let Defs = [R5, R2], isCall = 1 in
def TC : TCInst<0b000, (outs), (ins mem12:$k), "tc">;

// Transfer control to address, without saving the return address.
let Defs = [R5] in
def TCF : TCInst<0b001, (outs), (ins mem12:$k), "tcf">;

// Branch if accumulator is zero to fixed address.
let Defs = [R5], isExtracode = 1, DecoderNamespace = "Extracode" in
def BZF : BranchInst<0b001, (outs), (ins A:$a, mem12:$k), "bzf">;

// Branch if accumulator is zero or a negative number to fixed address.
let Defs = [R5], isExtracode = 1, DecoderNamespace = "Extracode" in
def BZMF : BranchInst<0b110, (outs), (ins A:$a, mem12:$k), "bzmf">;

//===----------------------------------------------------------------------===//
// IO instructions
//===----------------------------------------------------------------------===//

// Read value of IO channel to accumulator.
def READ : IOReadInst<0b000000, (outs A:$a_dst), (ins io9:$kc), "read">;

// Write value of accumulator to IO channel.
def WRITE : IOWriteInst<0b000001, (outs), (ins A:$a, io9:$kc), "write">;

// And value of IO channel to accumulator.
def RAND : IOReadInst<0b000010, (outs A:$a_dst), (ins A:$a, io9:$kc),
                      "rand">;

// And value of IO channel to accumulator and write resulting value back to IO
// channel.
def WAND : IOReadWriteInst<0b000011, (outs A:$a_dst), (ins A:$a, io9:$kc),
                           "wand">;

// Or value of IO channel to accumulator.
def ROR : IOReadInst<0b000100, (outs A:$a_dst), (ins A:$a, io9:$kc), "ror">;

// Or value of IO channel to accumulator and write resulting value back to IO
// channel.
def WOR : IOReadWriteInst<0b000101, (outs A:$a_dst), (ins A:$a, io9:$kc),
                          "wor">;

// Xor value of IO channel to accumulator.
def RXOR : IOReadInst<0b000110, (outs A:$a_dst), (ins A:$a, io9:$kc), "rxor">;

//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//

/// Generic pattern classes

class PatAGPR<SDPatternOperator OpNode, AGCInst Inst>
    : Pat<(OpNode A:$a, GPR:$k), (Inst A:$a, GPR:$k)>;

def : PatAGPR<add, PseudoAD>;
def : PatAGPR<sub, PseudoSU>;
def : PatAGPR<and, PseudoMASK>;

// Pessimistically assume the stack pointer will be clobbered
let Defs = [R49], Uses = [R49] in {
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i16imm:$amt1, i16imm:$amt2), "", "",
                              [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i16imm:$amt1, i16imm:$amt2), "", "",
                              [(callseq_end timm:$amt1, timm:$amt2)]>;
} // Defs = [R49], Uses = [R49]

let isCall = 1, Defs = [R0, R4] in
def PseudoCALL : Pseudo<(outs), (ins bare_symbol:$func), "call", "$func">;

def : Pat<(Call tglobaladdr:$func), (PseudoCALL tglobaladdr:$func)>;
def : Pat<(Call texternalsym:$func), (PseudoCALL texternalsym:$func)>;

let isBarrier = 1, isReturn = 1, isTerminator = 1 in
def PseudoRET : Pseudo<(outs), (ins), "ret", "", [(RetFlag)]>,
                PseudoInstExpansion<(TC 2)>;

def PseudoLoadInd : Pseudo<(outs A:$a), (ins GPR:$k, mem12:$offs), "ld",
                            "${offs}(${k})">;

def : Pat<(load (add GPR:$k, mem12:$offs)),
          (PseudoLoadInd GPR:$k, mem12:$offs)>;
def : Pat<(load GPR:$k), (PseudoLoadInd GPR:$k, 0)>;

def PseudoStoreInd : Pseudo<(outs), (ins A:$a, GPR:$k, mem12:$offs), "st",
                            "${offs}(${k})">;

def : Pat<(store A:$a, (add GPR:$k, mem12:$offs)),
          (PseudoStoreInd A:$a, GPR:$k, mem12:$offs)>;
def : Pat<(store A:$a, GPR:$k), (PseudoStoreInd A:$a, GPR:$k, 0)>;

// We will expand this into a CA with an operand representing the constant pool
// index for the given constant. The linker can deal with the allocation of
// memory locations to constants.
def PseudoLoadConst : Pseudo<(outs A:$a), (ins i16imm:$imm), "lc", "$imm">;
