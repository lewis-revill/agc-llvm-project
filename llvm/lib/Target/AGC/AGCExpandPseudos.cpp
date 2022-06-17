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

#include "llvm/CodeGen/LivePhysRegs.h"
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
                               MachineBasicBlock::iterator MBBI,
                               MachineBasicBlock::iterator &NextMBBI);

  bool expandPseudoCALL(MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MBBI);
  bool expandPseudoBNZF(MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MBBI,
                        MachineBasicBlock::iterator &NextMBBI);
  bool expandPseudoLoad(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI);
  bool expandPseudoStore(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI);
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
      Modified |= expandSimplePseudoInstr(MBB, MBBI, NextMBBI);
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

bool AGCExpandPseudos::expandPseudoBNZF(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MBBI,
                                        MachineBasicBlock::iterator &NextMBBI) {
  MachineFunction *MF = MBB.getParent();
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  Register CmpReg = MI.getOperand(0).getReg();
  const MachineOperand &Dest = MI.getOperand(1);

  assert(CmpReg == AGC::R0 && "Expecting comparison of accumulator only");
  assert(Dest.isMBB() && "Only basic block targets supported for branch");

  // We have no 'branch if not equal (to zero) instruction, so we must invert
  // the branch logic. IE:
  //
  //   PseudoBNZF a, %lo12(.true_block)
  //   ...
  //
  // becomes:
  //
  //   BZF a, %lo12(.false_block)
  //   TCF %lo12(.true_block)
  // .false_block:
  //   ...

  // Insert a new basic block for the false case.
  MachineBasicBlock *FalseMBB =
      MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  FalseMBB->setLabelMustBeEmitted();
  MF->insert(++MBB.getIterator(), FalseMBB);

  // Branch to the false block if the value is equal to zero.
  BuildMI(MBB, MI, DL, TII->get(AGC::BZF))
      .addReg(CmpReg)
      .addMBB(FalseMBB, AGCII::MO_LO12);

  // Unconditionally branch to the true block if that branch wasn't taken.
  MachineBasicBlock *DestMBB = Dest.getMBB();
  BuildMI(MBB, MI, DL, TII->get(AGC::TCF)).addMBB(DestMBB, AGCII::MO_LO12);

  // Move all the rest of the instructions to FalseMBB.
  FalseMBB->splice(FalseMBB->end(), &MBB, std::next(MBBI), MBB.end());
  // Update machine-CFG edges.
  FalseMBB->transferSuccessorsAndUpdatePHIs(&MBB);
  // Make the original basic block fall-through to the new.
  MBB.addSuccessor(FalseMBB);

  // Make sure live-ins are correctly attached to this new basic block.
  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *FalseMBB);

  NextMBBI = MBB.end();
  MI.eraseFromParent();
  return true;
}

bool AGCExpandPseudos::expandPseudoLoad(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  Register DstReg = MI.getOperand(0).getReg();
  const MachineOperand &Addr = MI.getOperand(1);
  uint64_t Offs = MI.getOperand(2).getImm();

  assert(DstReg == AGC::R0 && "Expecting load to accumulator only");
  assert(Addr.isGlobal() && "Only global addresses supported for load");
  assert(isInt<12>(Offs) && "Offset out of range for load");

  // CA %banks(addr)
  // TS EB
  // CA %lo12(addr)

  // Retrieve the 'bank configuration bits' from a location in memory - this is
  // filled in by the linker when we know where the global ends up and so know
  // which bank configuration bits must be used.
  BuildMI(MBB, MI, DL, TII->get(AGC::CA), AGC::R0)
      .addGlobalAddress(Addr.getGlobal(), Offs, AGCII::MO_BANKS)
      .addGlobalAddress(Addr.getGlobal(), Offs, AGCII::MO_BANKS);

  // Overwrite the erasable bank selection bits with those determined for this
  // address.
  BuildMI(MBB, MI, DL, TII->get(AGC::PseudoTS), AGC::R0)
      .addReg(AGC::R4, RegState::Define)
      .addReg(AGC::R0);

  // Copy the value at the constructed address into the accumulator.
  BuildMI(MBB, MI, DL, TII->get(AGC::CA), DstReg)
      .addGlobalAddress(Addr.getGlobal(), Offs, AGCII::MO_LO12)
      .addGlobalAddress(Addr.getGlobal(), Offs, AGCII::MO_LO12);

  MI.eraseFromParent();
  return true;
}

bool AGCExpandPseudos::expandPseudoStore(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  Register SrcReg = MI.getOperand(0).getReg();
  const MachineOperand &Addr = MI.getOperand(1);
  uint64_t Offs = MI.getOperand(2).getImm();

  assert(SrcReg == AGC::R0 && "Expecting store of accumulator only");
  assert(Addr.isGlobal() && "Only global addresses supported for load");
  assert(isInt<12>(Offs) && "Offset out of range for store");

  // TS R62
  // CA %banks(addr)
  // TS EB
  // CA R62
  // TS %lo12(addr)

  // Store the value in another register - we will need to use the accumulator
  // to setup the bank select bits.
  //
  // R62 is a reserved register which we won't need to preserve here so we can
  // use it for free.
  BuildMI(MBB, MI, DL, TII->get(AGC::PseudoTS), SrcReg)
      .addReg(AGC::R62, RegState::Define)
      .addReg(SrcReg);

  // Retrieve the 'bank configuration bits' from a location in memory - this is
  // filled in by the linker when we know where the global ends up and so know
  // which bank configuration bits must be used.
  BuildMI(MBB, MI, DL, TII->get(AGC::CA), AGC::R0)
      .addGlobalAddress(Addr.getGlobal(), Offs, AGCII::MO_BANKS)
      .addGlobalAddress(Addr.getGlobal(), Offs, AGCII::MO_BANKS);

  // Overwrite the erasable bank selection bits with those determined for this
  // address.
  BuildMI(MBB, MI, DL, TII->get(AGC::PseudoTS), AGC::R0)
      .addReg(AGC::R4, RegState::Define)
      .addReg(AGC::R0);

  // Copy the value to be stored back from R62 before storing it in the
  // destination address.
  BuildMI(MBB, MI, DL, TII->get(AGC::PseudoCA), SrcReg)
      .addReg(AGC::R62, RegState::Define)
      .addReg(AGC::R62);

  // Store the value to the destination address.
  BuildMI(MBB, MI, DL, TII->get(AGC::TS), SrcReg)
      .addGlobalAddress(Addr.getGlobal(), Offs, AGCII::MO_LO12)
      .addReg(SrcReg);

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
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  switch (MBBI->getOpcode()) {
  default:
    break;
  case AGC::PseudoCALL:
    return expandPseudoCALL(MBB, MBBI);
  case AGC::PseudoBNZF:
    return expandPseudoBNZF(MBB, MBBI, NextMBBI);
  case AGC::PseudoLoad:
    return expandPseudoLoad(MBB, MBBI);
  case AGC::PseudoStore:
    return expandPseudoStore(MBB, MBBI);
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
