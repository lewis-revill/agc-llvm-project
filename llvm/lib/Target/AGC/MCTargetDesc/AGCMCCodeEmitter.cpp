//===-- AGCMCCodeEmitter.cpp - Convert AGC MC to machine code -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the AGCMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "AGCMCCodeEmitter.h"
#include "AGCMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

#define DEBUG_TYPE "agc-mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

// Temporary - This should be defined in AGCInstrInfo.h
namespace AGCII {
enum { IsExtracode = 1 << 3 };
}

MCCodeEmitter *llvm::createAGCMCCodeEmitter(const MCInstrInfo &MCII,
                                            MCContext &Ctx) {
  return new AGCMCCodeEmitter(MCII, Ctx);
}

// Determine the odd parity bit that applies to the given 15-bit instruction
// code.
static uint16_t getParityBitForEncoding(uint16_t Enc) {
  // Accumulate the correct parity bit for odd parity.
  uint16_t ParityBit = 1;
  for (int i = 0; i < 15; i++)
    ParityBit ^= ((Enc >> i) & 1);

  return ParityBit;
}

// Append an odd parity bit to a 15-bit binary code.
static uint16_t getEncodingWithParity(uint16_t Bits) {
  return (Bits << 1) | getParityBitForEncoding(Bits);
}

void AGCMCCodeEmitter::emitBitsWithParity(raw_ostream &OS, uint16_t Bits) {
  uint16_t Encoding = getEncodingWithParity(Bits);
  support::endian::write<uint16_t>(OS, Encoding, support::big);
}

static unsigned getEquivalentNonPseudo(unsigned Opcode) {
  switch (Opcode) {
  default:
    break;
  case AGC::PseudoCA:
    return AGC::CA;
  case AGC::PseudoDCA:
    return AGC::DCA;
  case AGC::PseudoCS:
    return AGC::CS;
  case AGC::PseudoDCS:
    return AGC::DCS;
  case AGC::PseudoAD:
    return AGC::AD;
  case AGC::PseudoSU:
    return AGC::SU;
  case AGC::PseudoDAS:
    return AGC::DAS;
  case AGC::PseudoMSU:
    return AGC::MSU;
  case AGC::PseudoMASK:
    return AGC::MASK;
  case AGC::PseudoMP:
    return AGC::MP;
  case AGC::PseudoDV:
    return AGC::DV;
  case AGC::PseudoINCR:
    return AGC::INCR;
  case AGC::PseudoAUG:
    return AGC::AUG;
  case AGC::PseudoADS:
    return AGC::ADS;
  case AGC::PseudoDIM:
    return AGC::DIM;
  case AGC::PseudoTS:
    return AGC::TS;
  case AGC::PseudoXCH:
    return AGC::XCH;
  case AGC::PseudoLXCH:
    return AGC::LXCH;
  case AGC::PseudoQXCH:
    return AGC::QXCH;
  case AGC::PseudoDXCH:
    return AGC::DXCH;
  }

  return Opcode;
}

void AGCMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  assert(Desc.getSize() == 2);

  // Handle extracode instructions.
  // TODO: If we have an extracode with an INDEX instruction, the EXTEND is not
  // needed after the INDEX but instead before the INDEX.
  if (Desc.TSFlags & AGCII::IsExtracode) {
    // Prefix this instruction with an EXTEND instruction.
    emitBitsWithParity(OS, 0x0006);
    ++MCNumEmitted;
  }

  // TODO: Handle last-minute pseudo instructions.
  MCInst TrueMI = MI;
  TrueMI.setOpcode(getEquivalentNonPseudo(MI.getOpcode()));

  uint16_t Bits = getBinaryCodeForInstr(TrueMI, Fixups, STI);
  emitBitsWithParity(OS, Bits);
  ++MCNumEmitted;
}

unsigned AGCMCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                             const MCOperand &MO,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  // TODO: Implement fixups to resolve SymbolRef operands.
  if (MO.isExpr() && MO.getExpr()->getKind() == MCExpr::SymbolRef)
    // Upper bits of the address for branch instructions distinguishes them from
    // other instructions, so we cannot use 0 here!
    return (MI.getOpcode() == AGC::BZF || MI.getOpcode() == AGC::BZMF) ? 1024
                                                                       : 0;

  llvm_unreachable("Unhandled expression!");
  return 0;
}

// Get TableGen'erated function definitions.
#include "AGCGenMCCodeEmitter.inc"
