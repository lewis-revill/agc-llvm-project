//===-- AGCInstrInfo.cpp - AGC Instruction Information --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the AGC implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "AGCInstrInfo.h"
#include "AGCSubtarget.h"
#include "MCTargetDesc/AGCMCTargetDesc.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

#define GET_INSTRINFO_MC_DESC
#define GET_INSTRINFO_CTOR_DTOR
#include "AGCGenInstrInfo.inc"

using namespace llvm;

static void getLiveInRegsAt(LivePhysRegs &Regs, const MachineInstr &MI) {
  SmallVector<std::pair<MCPhysReg, const MachineOperand*>,2> Clobbers;
  const MachineBasicBlock &MBB = *MI.getParent();
  Regs.addLiveIns(MBB);
  auto E = MachineBasicBlock::const_iterator(MI.getIterator());
  for (auto I = MBB.begin(); I != E; ++I) {
    Clobbers.clear();
    Regs.stepForward(*I, Clobbers);
  }
}

AGCInstrInfo::AGCInstrInfo() : AGCGenInstrInfo() {}

Optional<DestSourcePair>
AGCInstrInfo::isCopyInstrImpl(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    break;
  case AGC::PseudoCA:
  case AGC::PseudoDCA:
  case AGC::PseudoTS:
    return DestSourcePair(MI.getOperand(0), MI.getOperand(2));
  }
  return None;
}

void AGCInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MBBI,
                               const DebugLoc &DL, MCRegister DstReg,
                               MCRegister SrcReg, bool KillSrc) const {
  const TargetRegisterInfo *TRI =
      MBB.getParent()->getSubtarget<AGCSubtarget>().getRegisterInfo();

  if (SrcReg == DstReg)
    return;

  // Any copy to the accumulator should be done with CA.
  if (AGC::ARegClass.contains(DstReg)) {
    assert(AGC::GPRRegClass.contains(SrcReg));
    BuildMI(MBB, MBBI, DL, get(AGC::PseudoCA), DstReg)
        .addReg(SrcReg, RegState::Define)
        .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }

  // Any copy from the accumulator should be done with TS.
  if (AGC::ARegClass.contains(SrcReg)) {
    assert(AGC::GPRRegClass.contains(DstReg));
    BuildMI(MBB, MBBI, DL, get(AGC::PseudoTS), SrcReg)
        .addReg(DstReg, RegState::Define)
        .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }

  // Copying to the double accumulator can be done with DCA.
  if (AGC::ALRegClass.contains(DstReg)) {
    assert(AGC::GPRDRegClass.contains(SrcReg));
    BuildMI(MBB, MBBI, DL, get(AGC::PseudoDCA), DstReg)
        .addReg(SrcReg, RegState::Define)
        .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }

  // There is no 'DTS' so copying from the double accumulator is a case of
  // using two copies for the subregisters.
  if (AGC::ALRegClass.contains(SrcReg)) {
    assert(AGC::GPRDRegClass.contains(DstReg));

    Register LoDst = TRI->getSubReg(DstReg, AGC::subreg_lower);
    Register LoSrc = TRI->getSubReg(SrcReg, AGC::subreg_lower);
    copyPhysReg(MBB, MBBI, DL, LoDst, LoSrc, KillSrc);

    Register HiDst = TRI->getSubReg(DstReg, AGC::subreg_upper);
    Register HiSrc = TRI->getSubReg(SrcReg, AGC::subreg_upper);
    copyPhysReg(MBB, MBBI, DL, HiDst, HiSrc, KillSrc);
    return;
  }

  // The general case for copying between double general purpose registers.
  // Fall back on copying to the double accumulator, then copying to the
  // destination.
  if (AGC::GPRDRegClass.contains(DstReg) &&
      AGC::GPRDRegClass.contains(SrcReg)) {
    // Find out whether we need to keep the old accumulator value intact.
    LivePhysRegs LiveAtMI(*TRI);
    getLiveInRegsAt(LiveAtMI, *MBBI);

    if (LiveAtMI.contains(AGC::RD0))
      // We need to keep the old accumulator value intact, so use a DXCH here.
      BuildMI(MBB, MBBI, DL, get(AGC::PseudoDXCH), AGC::RD0)
          .addReg(SrcReg, RegState::Define)
          .addReg(AGC::R0, RegState::Kill)
          .addReg(SrcReg, getKillRegState(KillSrc));
    else
      copyPhysReg(MBB, MBBI, DL, AGC::RD0, SrcReg, KillSrc);

    // Copy back out to the destination register.
    copyPhysReg(MBB, MBBI, DL, DstReg, AGC::RD0, /*KillSrc=*/true);

    if (LiveAtMI.contains(AGC::RD0))
      // Return the old values back to how they were.
      BuildMI(MBB, MBBI, DL, get(AGC::PseudoDXCH), AGC::RD0)
          .addReg(SrcReg, RegState::Define)
          .addReg(AGC::R0, RegState::Kill)
          .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }

  // Finally, the general case for copying between general purpose registers.
  // Fall back on copying to the accumulator, then copying to the destination.
  assert(AGC::GPRRegClass.contains(DstReg));
  assert(AGC::GPRRegClass.contains(SrcReg));

  // Find out whether we need to keep the old accumulator value intact.
  LivePhysRegs LiveAtMI(*TRI);
  getLiveInRegsAt(LiveAtMI, *MBBI);

  if (LiveAtMI.contains(AGC::R0))
    // We need to keep the old accumulator value intact, so use an XCH here.
    BuildMI(MBB, MBBI, DL, get(AGC::PseudoXCH), AGC::R0)
        .addReg(SrcReg, RegState::Define)
        .addReg(AGC::R0, RegState::Kill)
        .addReg(SrcReg, getKillRegState(KillSrc));
  else
    copyPhysReg(MBB, MBBI, DL, AGC::R0, SrcReg, KillSrc);


  // Copy back out to the destination register.
  copyPhysReg(MBB, MBBI, DL, DstReg, AGC::R0, /*KillSrc=*/true);

  if (LiveAtMI.contains(AGC::R0))
    // Return the old values back to how they were.
    BuildMI(MBB, MBBI, DL, get(AGC::PseudoXCH), AGC::R0)
        .addReg(SrcReg, RegState::Define)
        .addReg(AGC::R0, RegState::Kill)
        .addReg(SrcReg, getKillRegState(KillSrc));
}

unsigned AGCInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  if (MI.getOpcode() != AGC::PseudoLoadInd)
    return 0;

  if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
      MI.getOperand(2).getImm() == 0) {
    FrameIndex = MI.getOperand(1).getIndex();
    return MI.getOperand(0).getReg();
  }

  return 0;
}

unsigned AGCInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                          int &FrameIndex) const {
  if (MI.getOpcode() != AGC::PseudoStoreInd)
    return 0;

  if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
      MI.getOperand(2).getImm() == 0) {
    FrameIndex = MI.getOperand(1).getIndex();
    return MI.getOperand(0).getReg();
  }

  return 0;
}

void AGCInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator I,
                                       Register SrcReg, bool IsKill, int FI,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (RC == &AGC::ARegClass) {
    // We already have the value in the accumulator so we only need to do the
    // store.
    BuildMI(MBB, I, DL, get(AGC::PseudoStoreInd))
        .addReg(SrcReg, getKillRegState(IsKill))
        .addFrameIndex(FI)
        .addImm(0);
    return;
  }

  // Copy source into the accumulator.
  BuildMI(MBB, I, DL, get(TargetOpcode::COPY), AGC::R0)
      .addReg(SrcReg, getKillRegState(IsKill));

  // Store the value into the stack slot.
  BuildMI(MBB, I, DL, get(AGC::PseudoStoreInd))
      .addReg(AGC::R0)
      .addFrameIndex(FI)
      .addImm(0);
}

void AGCInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator I,
                                        Register DstReg, int FI,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (RC == &AGC::ARegClass) {
    // We will already place the value in the accumulator so we only need to do
    // the load.
    BuildMI(MBB, I, DL, get(AGC::PseudoLoadInd), DstReg)
        .addFrameIndex(FI)
        .addImm(0);
    return;
  }

  // Load the value into the accumulator.
  BuildMI(MBB, I, DL, get(AGC::PseudoLoadInd), AGC::R0)
      .addFrameIndex(FI)
      .addImm(0);

  // Copy the value back into the destination register.
  BuildMI(MBB, I, DL, get(TargetOpcode::COPY), DstReg)
      .addReg(AGC::R0, RegState::Kill);
}
