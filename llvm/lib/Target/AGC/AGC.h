//===-- AGC.h - Top-level interface for AGC ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// AGC back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AGC_AGC_H
#define LLVM_LIB_TARGET_AGC_AGC_H

namespace llvm {
class AGCTargetMachine;
class AsmPrinter;
class FunctionPass;
class MachineInstr;
class MachineOperand;
class MCInst;
class MCOperand;
class PassRegistry;

bool LowerAGCMachineOperandToMCOperand(const MachineOperand &MO,
                                       MCOperand &MCOp, AsmPrinter &AP);
void LowerAGCMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                  AsmPrinter &AP);

FunctionPass *createAGCISelDag(AGCTargetMachine &TM);

FunctionPass *createAGCExpandPseudosPass();
void initializeAGCExpandPseudosPass(PassRegistry &);
} // namespace llvm

#endif // LLVM_LIB_TARGET_AGC_AGC_H
