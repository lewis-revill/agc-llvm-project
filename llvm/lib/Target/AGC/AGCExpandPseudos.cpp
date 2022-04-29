//===-- AGCExpandPseudos.cpp - Peephole to expand pseudo instructions -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a pass which expands pseudo instructions just before
// emission.
//
//===----------------------------------------------------------------------===//

#include "AGC.h"
#include "AGCInstrInfo.h"
#include "MCTargetDesc/AGCMCTargetDesc.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"

#include <functional>

using namespace llvm;

#define DEBUG_TYPE "agc-expand-pseudos"
#define AGC_EXPAND_PSEUDOS_NAME "AGC expand pseudos pass"

namespace {

class AGCExpandPseudos : public MachineFunctionPass {
public:
  const AGCInstrInfo *TII;
  static char ID;

  AGCExpandPseudos() : MachineFunctionPass(ID) {
    initializeAGCExpandPseudosPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return AGC_EXPAND_PSEUDOS_NAME; }

private:
  bool expandSimplePseudoInstr(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MBBI);

  bool expandPseudoLoadIndirect(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MBBI);
  bool expandPseudoStoreIndirect(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI);
};

char AGCExpandPseudos::ID = 0;

bool AGCExpandPseudos::runOnMachineFunction(MachineFunction &MF) {
  TII = static_cast<const AGCInstrInfo *>(MF.getSubtarget().getInstrInfo());

  bool Modified = false;

  // Attempt simple pseudo instruction expansions.
  for (auto &MBB : MF) {
    auto MBBI = MBB.begin(), MBBE = MBB.end();
    while (MBBI != MBBE) {
      auto NextMBBI = std::next(MBBI);
      Modified |= expandSimplePseudoInstr(MBB, MBBI);
      MBBI = NextMBBI;
    }
  }

  return Modified;
}

bool AGCExpandPseudos::expandPseudoLoadIndirect(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  Register DstReg = MI.getOperand(0).getReg();
  Register PtrReg = MI.getOperand(1).getReg();
  uint64_t Offs = MI.getOperand(2).getImm();

  assert(DstReg == AGC::R0 && "Expecting load to accumulator only");
  assert(isInt<12>(Offs) && "Offset out of range for indirect load");

  BuildMI(MBB, MI, DL, TII->get(AGC::INDEX), PtrReg).addReg(PtrReg);

  BuildMI(MBB, MI, DL, TII->get(AGC::CA), DstReg)
      .addImm(Offs)
      .addImm(Offs);

  MI.eraseFromParent();
  return true;
}

bool AGCExpandPseudos::expandPseudoStoreIndirect(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  Register SrcReg = MI.getOperand(0).getReg();
  Register PtrReg = MI.getOperand(1).getReg();
  uint64_t Offs = MI.getOperand(2).getImm();

  assert(SrcReg == AGC::R0 && "Expecting store of accumulator only");
  assert(isInt<12>(Offs) && "Offset out of range for indirect store");

  BuildMI(MBB, MI, DL, TII->get(AGC::INDEX), PtrReg).addReg(PtrReg);

  BuildMI(MBB, MI, DL, TII->get(AGC::TS), SrcReg)
      .addImm(Offs)
      .addReg(SrcReg);

  MI.eraseFromParent();
  return true;
}

bool AGCExpandPseudos::expandSimplePseudoInstr(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI) {
  switch (MBBI->getOpcode()) {
  default:
    break;
  case AGC::PseudoLoadInd:
    return expandPseudoLoadIndirect(MBB, MBBI);
  case AGC::PseudoStoreInd:
    return expandPseudoStoreIndirect(MBB, MBBI);
  }
  return false;
}

} // end of anonymous namespace

INITIALIZE_PASS(AGCExpandPseudos, "agc-expand-pseudos", AGC_EXPAND_PSEUDOS_NAME,
                false, false)

namespace llvm {

FunctionPass *createAGCExpandPseudosPass() { return new AGCExpandPseudos(); }

} // end of namespace llvm
