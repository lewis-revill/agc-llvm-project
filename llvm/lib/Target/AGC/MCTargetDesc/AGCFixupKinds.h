//===-- AGCFixupKinds.h - AGC Specific Fixup Entries ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AGC_MCTARGETDESC_AGCFIXUPKINDS_H
#define LLVM_LIB_TARGET_AGC_MCTARGETDESC_AGCFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

#undef AGC

namespace llvm {
namespace AGC {
enum Fixups {
  // 12-bit fixup corresponding to %cpi(N) for loading constants.
  fixup_agc_cpi12 = FirstTargetFixupKind,

  // Used as a sentinel, must be the last
  fixup_agc_invalid,
  NumTargetFixupKinds = fixup_agc_invalid - FirstTargetFixupKind
};
} // end namespace AGC
} // end namespace llvm

#endif
