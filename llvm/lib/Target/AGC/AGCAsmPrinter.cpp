//===-- AGCAsmPrinter.cpp - AGC LLVM assembly writer ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the AGC assembly language.
//
//===----------------------------------------------------------------------===//

#include "AGCAsmPrinter.h"
#include "AGC.h"
#include "MCTargetDesc/AGCMCTargetDesc.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

// Simple pseudo-instructions have their lowering (with expansion to real
// instructions) auto-generated.
#include "AGCGenMCPseudoLowering.inc"

void AGCAsmPrinter::emitInstruction(const MachineInstr *MI) {
  // Do any auto-generated pseudo lowerings.
  if (emitPseudoExpansionLowering(*OutStreamer, MI))
    return;

  MCInst TmpInst;
  LowerAGCMachineInstrToMCInst(MI, TmpInst, *this);
  EmitToStreamer(*OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" void LLVMInitializeAGCAsmPrinter() {
  RegisterAsmPrinter<AGCAsmPrinter> X(getTheAGCTarget());
}
