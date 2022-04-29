//===-- AGCMCExpr.cpp - AGC specific MC expression classes ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the assembly expression modifiers
// accepted by AGC.
//
//===----------------------------------------------------------------------===//

#include "AGCMCExpr.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCValue.h"

using namespace llvm;

const AGCMCExpr *AGCMCExpr::create(const MCExpr *Expr, VariantKind Kind,
                                   MCContext &Ctx) {
  return new (Ctx) AGCMCExpr(Expr, Kind);
}

void AGCMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  VariantKind Kind = getKind();
  bool HasVariant = (Kind != VK_AGC_None);

  if (HasVariant)
    OS << '%' << getVariantKindName(getKind()) << '(';
  Expr->print(OS, MAI);
  if (HasVariant)
    OS << ')';
}

bool AGCMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                          const MCAsmLayout *Layout,
                                          const MCFixup *Fixup) const {
  if (!getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup))
    return false;

  return true;
}

void AGCMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

void AGCMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {}

AGCMCExpr::VariantKind AGCMCExpr::getVariantKindForName(StringRef Name) {
  return StringSwitch<AGCMCExpr::VariantKind>(Name)
      .Case("cpi", VK_AGC_CPI)
      .Default(VK_AGC_Invalid);
}

StringRef AGCMCExpr::getVariantKindName(VariantKind Kind) {
  switch (Kind) {
  default:
    llvm_unreachable("Invalid ELF symbol kind");
  case VK_AGC_CPI:
    return "cpi";
  }
}
