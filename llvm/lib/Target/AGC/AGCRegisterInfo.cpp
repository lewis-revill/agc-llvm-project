//===-- AGCRegisterInfo.cpp - AGC Register Information --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the AGC implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "AGCRegisterInfo.h"
#include "AGCFrameLowering.h"
#include "AGCSubtarget.h"
#include "MCTargetDesc/AGCMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define GET_REGINFO_MC_DESC
#define GET_REGINFO_TARGET_DESC
#include "AGCGenRegisterInfo.inc"

using namespace llvm;

AGCRegisterInfo::AGCRegisterInfo(unsigned HwMode)
    : AGCGenRegisterInfo(AGC::R2, /*DwarfFlavour=*/0, /*EHFlavor=*/0, /*PC=*/0,
                         HwMode) {}

const MCPhysReg *
AGCRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector AGCRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  // All registers from R3 to R49 (inclusive) have some special functionality
  // which means they cannot be overwritten/allocated.
  for (unsigned Offs = 0; AGC::R3 + Offs < AGC::R50; ++Offs)
    markSuperRegs(Reserved, AGC::R3 + Offs);

  return Reserved;
}

void AGCRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  const MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFI = getFrameLowering(MF);

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  Register FrameReg;
  StackOffset Offset =
      getFrameLowering(MF)->getFrameIndexReference(MF, FrameIndex, FrameReg);
  Offset += StackOffset::getFixed(MI.getOperand(FIOperandNum + 1).getImm());

  // TODO: Update this when using a frame pointer as the frame reg. For now we
  // are referencing the stack pointer so subtract the stack adjustment.
  assert(!TFI->hasFP(MF));
  Offset -= StackOffset::getFixed(MFI.getStackSize());

  assert(isInt<12>(Offset.getFixed()) && "Stack offset out of range");
  assert(!Offset.getScalable() && "Expected fixed offset only");

  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, /*isDef=*/false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset.getFixed());
}

Register AGCRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  // FIXME: Decide on a frame pointer.
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  assert(!TFI->hasFP(MF));
  return AGC::R49;
}
