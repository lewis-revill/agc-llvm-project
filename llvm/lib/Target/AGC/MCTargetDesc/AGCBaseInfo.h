//===-- AGCBaseInfo.h - Top level definitions for AGC MC --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone enum definitions for the AGC target
// useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_AGC_MCTARGETDESC_AGCBASEINFO_H
#define LLVM_LIB_TARGET_AGC_MCTARGETDESC_AGCBASEINFO_H

namespace llvm {

namespace AGCII {

// AGC-specific machine operand flags
enum {
  MO_None = 0,
  MO_CPI = 1,
  MO_BANKS = 2,
  MO_LO12 = 3
};
} // namespace AGCII

} // namespace llvm

#endif
