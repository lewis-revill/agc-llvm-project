//===-- AGCMCExpr.h - AGC specific MC expression classes -------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the AGC-specific MCExprs, used for modifiers like "%hi"
// or "%lo" etc.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AGC_MCTARGETDESC_AGCMCEXPR_H
#define LLVM_LIB_TARGET_AGC_MCTARGETDESC_AGCMCEXPR_H

#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;

class AGCMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_AGC_None,
    VK_AGC_CPI,
    VK_AGC_BANKS,
    VK_AGC_LO12,
    VK_AGC_Invalid,
  };

private:
  const MCExpr *Expr;
  const VariantKind Kind;

  int64_t evaluateAsInt64(int64_t Value) const;

  explicit AGCMCExpr(const MCExpr *Expr, VariantKind Kind)
      : Expr(Expr), Kind(Kind) {}

public:
  static const AGCMCExpr *create(const MCExpr *Expr, VariantKind Kind,
                                 MCContext &Ctx);

  VariantKind getKind() const { return Kind; }

  const MCExpr *getSubExpr() const { return Expr; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static bool classof(const AGCMCExpr *) { return true; }

  static VariantKind getVariantKindForName(StringRef Name);
  static StringRef getVariantKindName(VariantKind Kind);
};

} // namespace llvm

#endif
