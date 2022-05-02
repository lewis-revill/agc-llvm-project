//===-- AGCFrameLowering.cpp - AGC Frame Information ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the AGC implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "AGCFrameLowering.h"
#include "AGCInstrInfo.h"
#include "AGCSubtarget.h"
#include "MCTargetDesc/AGCMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

using namespace llvm;

void AGCFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");

  // TODO: Implement support for frame pointer.
  assert(!hasFP(MF) && "No frame pointer available yet");

  const AGCInstrInfo *TII = MF.getSubtarget<AGCSubtarget>().getInstrInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();

  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc DL;

  // Determine the correct frame layout. Updates MFI stack size to be accurate
  // according to alignment.
  determineFrameLayout(MF);

  // Get the number of bytes to allocate from the FrameInfo.
  uint64_t StackSize = MFI.getStackSize();

  // Early exit if there is no need to allocate on the stack.
  if (StackSize == 0)
    return;

  assert(isUInt<9>(StackSize) && "Stack size too big for AGC");

  // Prepare to add the constant stack size to the stack pointer register.
  BuildMI(MBB, MBBI, DL, TII->get(AGC::PseudoLoadConst), AGC::R0)
      .addImm(StackSize);

  // Perform the addition, leaving the result in the accumulator.
  BuildMI(MBB, MBBI, DL, TII->get(AGC::PseudoAD), AGC::R0)
      .addReg(AGC::R49, RegState::Define)
      .addReg(AGC::R0, RegState::Kill)
      .addReg(AGC::R49);

  // Write back to the stack pointer register.
  BuildMI(MBB, MBBI, DL, TII->get(AGC::PseudoTS), AGC::R0)
      .addReg(AGC::R49, RegState::Define)
      .addReg(AGC::R0)
      .setMIFlag(MachineInstr::FrameSetup);
}

void AGCFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  // TODO: Implement support for frame pointer.
  assert(!hasFP(MF) && "No frame pointer available yet");

  const AGCInstrInfo *TII = MF.getSubtarget<AGCSubtarget>().getInstrInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL = MBBI->getDebugLoc();

  // TODO: Account for callee-saved registers.
  int64_t StackSize = MFI.getStackSize();

  // Early exit if there was no need to allocate on the stack.
  if (StackSize == 0)
    return;

  assert(isUInt<9>(StackSize) && "Stack size too big for AGC");

  // Deallocate stack.
  // Prepare to subtract the constant stack size from the stack pointer.
  BuildMI(MBB, MBBI, DL, TII->get(AGC::PseudoLoadConst), AGC::R0)
      .addImm(StackSize);

  // Perform the subtraction, leaving the result in the accumulator.
  BuildMI(MBB, MBBI, DL, TII->get(AGC::PseudoSU), AGC::R0)
      .addReg(AGC::R49, RegState::Define)
      .addReg(AGC::R0, RegState::Kill)
      .addReg(AGC::R49);

  // Write back to the stack pointer register.
  BuildMI(MBB, MBBI, DL, TII->get(AGC::PseudoTS), AGC::R0)
      .addReg(AGC::R49, RegState::Define)
      .addReg(AGC::R0)
      .setMIFlag(MachineInstr::FrameDestroy);
}

// Determines the size of the frame and maximum call frame size.
void AGCFrameLowering::determineFrameLayout(MachineFunction &MF) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Get the number of bytes to allocate from the FrameInfo.
  uint64_t FrameSize = MFI.getStackSize();

  // Get the alignment.
  Align StackAlign = getStackAlign();

  // Get the maximum call frame size of all the calls.
  uint64_t MaxCallFrameSize = MFI.getMaxCallFrameSize();

  // If we have dynamic alloca then MaxCallFrameSize needs to be aligned so
  // that allocations will be aligned.
  if (MFI.hasVarSizedObjects())
    MaxCallFrameSize = alignTo(MaxCallFrameSize, StackAlign);

  // Update maximum call frame size.
  MFI.setMaxCallFrameSize(MaxCallFrameSize);

  // Include call frame size in total.
  if (!(hasReservedCallFrame(MF) && MFI.adjustsStack()))
    FrameSize += MaxCallFrameSize;

  // Make sure the frame is aligned.
  FrameSize = alignTo(FrameSize, StackAlign);

  // Update frame info.
  MFI.setStackSize(FrameSize);
}
