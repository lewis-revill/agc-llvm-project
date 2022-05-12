//===-- AGCAsmBackend.cpp - AGC Assembler Backend --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the definition of the AGCAsmBackend class.
//
//===----------------------------------------------------------------------===//

#include "AGCAsmBackend.h"
#include "AGCMCCodeEmitter.h"
#include "AGCMCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"

using namespace llvm;

std::unique_ptr<MCObjectTargetWriter>
AGCAsmBackend::createObjectTargetWriter() const {
  return createAGCELFObjectWriter(OSABI);
}

bool AGCAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                 const MCSubtargetInfo *STI) const {
  // Can only write 2-byte nops. Should be an assert since getMinimumNopSize is
  // defined.
  if (Count % 2 != 0)
    return false;

  uint64_t NopCount = Count / 2;
  for (uint64_t i = 0; i < NopCount; i++)
    // Canonical nop on AGC is CA A (copy value of accumulator to accumulator).
    AGCMCCodeEmitter::emitBitsWithParity(OS, 0x3000);

  return true;
}

void AGCAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                               const MCValue &Target,
                               MutableArrayRef<char> Data, uint64_t Value,
                               bool IsResolved,
                               const MCSubtargetInfo *STI) const {
  return;
}

void AGCAsmBackend::relaxInstruction(MCInst &Inst,
                                     const MCSubtargetInfo &STI) const {
  llvm_unreachable("No relaxation implemented");
}

const MCFixupKindInfo &AGCAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[] = {
      // This table *must* be in the order that the fixup_* kinds are defined in
      // AGCFixupKinds.h.
      //
      // name               offset bits  flags
      {"fixup_agc_cpi12",   3,     12,   0},
      {"fixup_agc_banks12", 3,     12,   0},
      {"fixup_agc_lo12",    3,     12,   0},
  };
  static_assert((array_lengthof(Infos)) == AGC::NumTargetFixupKinds,
                "Not all fixup kinds added to Infos array");

  // Fixup kinds from .reloc directive do not require any extra processing.
  if (Kind >= FirstLiteralRelocationKind)
    return MCAsmBackend::getFixupKindInfo(FK_NONE);

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

MCAsmBackend *llvm::createAGCAsmBackend(const Target &T,
                                        const MCSubtargetInfo &STI,
                                        const MCRegisterInfo &MRI,
                                        const MCTargetOptions &Options) {
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(Triple::UnknownOS);
  return new AGCAsmBackend(OSABI);
}
