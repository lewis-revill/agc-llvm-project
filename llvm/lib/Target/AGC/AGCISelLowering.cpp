//===-- AGCISelLowering.cpp - AGC DAG Lowering Implementation  ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that AGC uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "AGCISelLowering.h"
#include "AGCSubtarget.h"
#include "MCTargetDesc/AGCMCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"

using namespace llvm;

#define DEBUG_TYPE "agc-lower"

AGCTargetLowering::AGCTargetLowering(const TargetMachine &TM,
                                     const AGCSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {

  // Ensure TLI knows about the accumulator register classes, even though they
  // aren't used in the same way as most register classes would be.
  addRegisterClass(MVT::i16, &AGC::ARegClass);
  addRegisterClass(MVT::i16, &AGC::LRegClass);
  addRegisterClass(MVT::i32, &AGC::ALRegClass);

  // Add the general purpose register classes.
  addRegisterClass(MVT::i16, &AGC::GPRRegClass);
  addRegisterClass(MVT::i32, &AGC::GPRDRegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(STI.getRegisterInfo());

  // TODO: Custom lowering required for widening ops.
}

SDValue AGCTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    report_fatal_error("unimplemented operation");
  }
}

void AGCTargetLowering::ReplaceNodeResults(SDNode *N,
                                           SmallVectorImpl<SDValue> &Results,
                                           SelectionDAG &DAG) const {
  switch (N->getOpcode()) {
  default:
    SDValue Res = LowerOperation(SDValue(N, 0), DAG);
    for (unsigned I = 0, E = Res->getNumValues(); I != E; ++I)
      Results.push_back(Res.getValue(I));
    break;
  }
}

// Calling Convention Implementation.
#include "AGCGenCallingConv.inc"

SDValue AGCTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  switch (CallConv) {
  default:
    report_fatal_error("unsupported calling convention");
  case CallingConv::C:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  if (IsVarArg)
    report_fatal_error("vararg not yet supported");

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_AGC);

  for (auto &VA : ArgLocs) {
    if (!VA.isRegLoc())
      report_fatal_error("stack arguments not yet supported");

    // Arguments passed in the upper/lower accumulators and GPRs.
    EVT RegVT = VA.getLocVT();
    if (RegVT != MVT::i32 && RegVT != MVT::i16) {
      LLVM_DEBUG(dbgs() << "LowerFormalArguments Unhandled argument type: "
                        << RegVT.getEVTString() << "\n");
      report_fatal_error("unhandled argument type");
    }

    unsigned VReg = RegInfo.createVirtualRegister(
        RegVT == MVT::i32 ? &AGC::GPRDRegClass : &AGC::GPRRegClass);
    RegInfo.addLiveIn(VA.getLocReg(), VReg);
    SDValue ArgIn = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

    InVals.push_back(ArgIn);
  }

  return Chain;
}

SDValue
AGCTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &DL, SelectionDAG &DAG) const {
  if (IsVarArg)
    report_fatal_error("vararg not yet supported");

  // Stores the assignment of the return value to a location.
  SmallVector<CCValAssign, 16> RVLocs;

  // Info about the registers and stack slot.
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_AGC);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the designated registers.
  for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(AGCISD::RET_FLAG, DL, MVT::Other, RetOps);
}

const char *AGCTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((AGCISD::NodeType)Opcode) {
  default:
    break;
  case AGCISD::RET_FLAG:
    return "AGCISD::RET_FLAG";
  }
  return nullptr;
}
