//===-- AGCMCTargetDesc.h - AGC Target Descriptions -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides AGC specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AGC_MCTARGETDESC_AGCMCTARGETDESC_H
#define LLVM_LIB_TARGET_AGC_MCTARGETDESC_AGCMCTARGETDESC_H

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;
class raw_pwrite_stream;

Target &getTheAGCTarget();

MCCodeEmitter *createAGCMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx);

MCAsmBackend *createAGCAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createAGCELFObjectWriter(uint8_t OSABI);
} // namespace llvm

// Defines symbolic names for AGC registers.
#define GET_REGINFO_ENUM
#include "AGCGenRegisterInfo.inc"

// Defines symbolic names for AGC instructions.
#define GET_INSTRINFO_ENUM
#include "AGCGenInstrInfo.inc"

#endif
