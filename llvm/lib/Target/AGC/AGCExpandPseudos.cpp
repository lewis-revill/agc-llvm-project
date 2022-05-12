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
#include "MCTargetDesc/AGCBaseInfo.h"
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

  bool expandPseudoCALL(MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MBBI);
  bool expandPseudoLoadIndirect(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MBBI);
  bool expandPseudoStoreIndirect(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI);
  bool expandPseudoLoadConst(MachineBasicBlock &MBB,
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

bool AGCExpandPseudos::expandPseudoCALL(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  const MachineOperand &Callee = MI.getOperand(0);

  assert(Callee.isGlobal() && "Only global addresses supported so far");

  // If the callee is in a different bank then we must use a dispatch function
  // to jump between banks and still be able to call the function. Only the
  // linker can tell if this is needed so in the compiler we must be cautious
  // and assume it is always needed. With the required linker work we might be
  // able to relax the sequence.

  // CA %banks(func)
  // TS R62
  // CA %lo12(func)
  // TS R63
  // TC __dispatch

  // Retrieve the 'bank configuration bits' from a location in memory - this is
  // filled in by the linker when we know where the function ends up and so know
  // which bank configuration bits must be used.
  BuildMI(MBB, MI, DL, TII->get(AGC::CA), AGC::R0)
      .addGlobalAddress(Callee.getGlobal(), 0, AGCII::MO_BANKS)
      .addGlobalAddress(Callee.getGlobal(), 0, AGCII::MO_BANKS);

  // Write the bank select bits to the register R62 to pass to the dispatch
  // function.
  BuildMI(MBB, MI, DL, TII->get(AGC::PseudoTS), AGC::R0)
      .addReg(AGC::R62, RegState::Define)
      .addReg(AGC::R0);

  // Copy the lower 12 bits of the function - this is filled in by the linker
  // when we know where the function ends up.
  BuildMI(MBB, MI, DL, TII->get(AGC::CA), AGC::R0)
      .addGlobalAddress(Callee.getGlobal(), 0, AGCII::MO_LO12)
      .addGlobalAddress(Callee.getGlobal(), 0, AGCII::MO_LO12);

  // Write the lower 12 bits to the register R63 to pass to the dispatch
  // function.
  BuildMI(MBB, MI, DL, TII->get(AGC::PseudoTS), AGC::R0)
      .addReg(AGC::R63, RegState::Define)
      .addReg(AGC::R0);

  // Call the dispatch function. We must still add the implicit defs for the
  // return arguments here to satisfy the verifier.
  BuildMI(MBB, MI, DL, TII->get(AGC::TC))
      .addExternalSymbol("__dispatch")
      .copyImplicitOps(MI);

  MI.eraseFromParent();
  return true;
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

bool AGCExpandPseudos::expandPseudoLoadConst(MachineBasicBlock &MBB,
                                             MachineBasicBlock::iterator MBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  Register DstReg = MI.getOperand(0).getReg();
  uint64_t Imm = MI.getOperand(1).getImm();

  assert(DstReg == AGC::R0 && "Expecting accumulator dest only");
  assert(isInt<16>(Imm) && "Constant out of range");

  BuildMI(MBB, MI, DL, TII->get(AGC::CA), DstReg)
      .addConstantPoolIndex(Imm, 0, AGCII::MO_CPI)
      .addConstantPoolIndex(Imm, 0, AGCII::MO_CPI);

  MI.eraseFromParent();
  return true;
}

bool AGCExpandPseudos::expandSimplePseudoInstr(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI) {
  switch (MBBI->getOpcode()) {
  default:
    break;
  case AGC::PseudoCALL:
    return expandPseudoCALL(MBB, MBBI);
  case AGC::PseudoLoadInd:
    return expandPseudoLoadIndirect(MBB, MBBI);
  case AGC::PseudoStoreInd:
    return expandPseudoStoreIndirect(MBB, MBBI);
  case AGC::PseudoLoadConst:
    return expandPseudoLoadConst(MBB, MBBI);
  }
  return false;
}

} // end of anonymous namespace

INITIALIZE_PASS(AGCExpandPseudos, "agc-expand-pseudos", AGC_EXPAND_PSEUDOS_NAME,
                false, false)

namespace llvm {

FunctionPass *createAGCExpandPseudosPass() { return new AGCExpandPseudos(); }

} // end of namespace llvm
