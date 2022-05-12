//===-- AGCELFObjectWriter.cpp - AGC ELF Writer --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the AGCMCELFObjectWriter class.
//
//===----------------------------------------------------------------------===//

#include "AGCMCELFObjectWriter.h"
#include "AGCFixupKinds.h"
#include "AGCMCTargetDesc.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"

using namespace llvm;

unsigned AGCELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
                                          const MCFixup &Fixup,
                                          bool IsPCRel) const {
  switch (Fixup.getKind()) {
  default:
    llvm_unreachable("Unimplemented fixup kind!");
  case AGC::fixup_agc_cpi12:
    return ELF::R_AGC_CPI12;
  case AGC::fixup_agc_banks12:
    return ELF::R_AGC_BANKS12;
  case AGC::fixup_agc_lo12:
    return ELF::R_AGC_LO12;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createAGCELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<AGCELFObjectWriter>(OSABI);
}
