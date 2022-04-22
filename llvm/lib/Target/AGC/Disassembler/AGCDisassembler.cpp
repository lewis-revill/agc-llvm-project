//===-- AGCDisassembler.cpp - Disassembler for AGC ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the AGCDisassembler class.
//
//===----------------------------------------------------------------------===//

#include "AGCDisassembler.h"
#include "MCTargetDesc/AGCMCTargetDesc.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"

using namespace llvm;

#define DEBUG_TYPE "agc-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

static MCDisassembler *createAGCDisassembler(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx) {
  return new AGCDisassembler(STI, Ctx);
}

extern "C" void LLVMInitializeAGCDisassembler() {
  // Register the disassembler for the AGC target.
  TargetRegistry::RegisterMCDisassembler(getTheAGCTarget(),
                                         createAGCDisassembler);
}

static DecodeStatus decodeMem12Operand(MCInst &Inst, uint64_t Imm,
                                       int64_t Address, const void *Decoder) {
  assert(isUInt<12>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

static DecodeStatus decodeIO9Operand(MCInst &Inst, uint64_t Imm,
                                     int64_t Address, const void *Decoder) {
  assert(isUInt<9>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeARegisterClass(MCInst &Inst, uint64_t RegNo,
                                         int64_t Address, const void *Decoder) {
  MCRegister Reg = AGC::R0;
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeLRegisterClass(MCInst &Inst, uint64_t RegNo,
                                         int64_t Address, const void *Decoder) {
  MCRegister Reg = AGC::R1;
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeALRegisterClass(MCInst &Inst, uint64_t RegNo,
                                          int64_t Address,
                                          const void *Decoder) {
  MCRegister Reg = AGC::RD0;
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeQRegisterClass(MCInst &Inst, uint64_t RegNo,
                                         int64_t Address, const void *Decoder) {
  MCRegister Reg = AGC::R2;
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                           int64_t Address,
                                           const void *Decoder) {
  MCRegister Reg = AGC::R0 + RegNo;
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeGPRDRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            int64_t Address,
                                            const void *Decoder) {
  MCRegister Reg = AGC::RD0 + RegNo;
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

#include "AGCGenDisassemblerTables.inc"

DecodeStatus AGCDisassembler::getInstruction(MCInst &MI, uint64_t &Size,
                                             ArrayRef<uint8_t> Bytes,
                                             uint64_t Address,
                                             raw_ostream &CS) const {
  uint16_t Instruction;
  DecodeStatus Result;

  // Try non-extracode instructions first.
  Instruction = support::endian::read16be(Bytes.data());
  // Mask off the parity bit.
  Instruction = (Instruction & 0xFFFE) >> 1;
  Result =
      decodeInstruction(DecoderTable16, MI, Instruction, Address, this, STI);

  if (MI.getOpcode() != AGC::EXTEND) {
    Size = 2;
    return Result;
  }

  // TODO: Handle INDEX instructions.

  // Try parsing the following instruction using the decoder table for
  // extracodes.
  Instruction = support::endian::read16be(Bytes.drop_front(2).data());
  // Mask off the parity bit of the second instruction.
  Instruction = (Instruction & 0xFFFE) >> 1;
  Result = decodeInstruction(DecoderTableExtracode16, MI, Instruction, Address,
                             this, STI);

  // FIXME: An EXTEND followed by another EXTEND is valid, and the first should
  // be considered a standalone instruction.

  Size = 4;
  return Result;
}
