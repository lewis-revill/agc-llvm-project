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

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);

  setOperationAction(ISD::BR_CC, MVT::i16, Expand);
  setOperationAction(ISD::BRCOND, MVT::Other, Custom);
  // TODO: Custom lowering required for widening ops.
}

SDValue AGCTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    report_fatal_error("unimplemented operation");
  case ISD::GlobalAddress:
    return lowerGlobalAddress(Op, DAG);
  case ISD::BRCOND:
    return lowerBRCOND(Op, DAG);
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

SDValue AGCTargetLowering::lowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
  int64_t Offset = N->getOffset();

  SDValue Addr = DAG.getTargetGlobalAddress(N->getGlobal(), DL, Ty, 0);

  if (Offset != 0)
    return DAG.getNode(ISD::ADD, DL, Ty, Addr, DAG.getConstant(Offset, DL, MVT::i16));
  return Addr;
}

static void translateSetCCForBranch(const SDLoc &DL, SDValue &LHS, SDValue &RHS,
                                    ISD::CondCode &CCVal, SelectionDAG &DAG) {
  EVT Ty = LHS.getValueType();
  switch (CCVal) {
  default:
    break;
  case ISD::SETGT:
  case ISD::SETGE:
  case ISD::SETUGT:
  case ISD::SETUGE:
    CCVal = ISD::getSetCCSwappedOperands(CCVal);
    std::swap(LHS, RHS);
    break;
  }

  switch (CCVal) {
  default:
    break;
  case ISD::SETLT:
  case ISD::SETULT:
    CCVal = CCVal == ISD::SETLT ? ISD::SETLE : ISD::SETULE;
    LHS = DAG.getNode(ISD::ADD, DL, Ty, LHS, DAG.getConstant(1, DL, Ty));
    break;
  }

  // We are going to be comparing against zero no matter what so perform the
  // subtraction here to simplify later parts.
  LHS = DAG.getNode(ISD::SUB, DL, Ty, LHS, RHS);
}

SDValue AGCTargetLowering::lowerBRCOND(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);

  SDValue CondV = Op.getOperand(1);

  if (CondV.getOpcode() == ISD::SETCC &&
      CondV.getOperand(0).getValueType() == MVT::i16) {

    SDValue LHS = CondV.getOperand(0);
    SDValue RHS = CondV.getOperand(1);

    ISD::CondCode CCVal = cast<CondCodeSDNode>(CondV.getOperand(2))->get();

    translateSetCCForBranch(DL, LHS, RHS, CCVal, DAG);

    SDValue TargetCC = DAG.getCondCode(CCVal);
    return DAG.getNode(AGCISD::BR_CC_ZERO, DL, Op.getValueType(),
                       Op.getOperand(0), LHS, TargetCC, Op.getOperand(2));
  }

  return DAG.getNode(AGCISD::BR_CC_ZERO, DL, Op.getValueType(),
                     Op.getOperand(0), CondV, DAG.getCondCode(ISD::SETNE),
                     Op.getOperand(2));
}

/// isEligibleForTailCallOptimization - Check whether the call is eligible
/// for tail call optimization.
bool AGCTargetLowering::isEligibleForTailCallOptimization(
    CCState &CCInfo, CallLoweringInfo &CLI, MachineFunction &MF,
    const SmallVector<CCValAssign, 16> &ArgLocs) const {
  // TODO: Support tail calls.
  return false;
}

// Calling Convention Implementation.
#include "AGCGenCallingConv.inc"

SDValue AGCTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &IsTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  auto &MF = DAG.getMachineFunction();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_AGC);

  if (IsVarArg)
    report_fatal_error("Var args not supported yet");

  // Check if it's really possible to do a tail call
  if (IsTailCall)
    IsTailCall = isEligibleForTailCallOptimization(CCInfo, CLI, MF, ArgLocs);

  // TODO: Support tail calls.
  if (IsTailCall)
    report_fatal_error("Tail calls not supported yet");

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  // Walk the register/memloc assignments, inserting copies/loads.
  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;
  for (unsigned I = 0, E = ArgLocs.size(); I != E; ++I) {
    CCValAssign &VA = ArgLocs[I];
    SDValue Arg = OutVals[I];
    ISD::ArgFlagsTy Flags = Outs[I].Flags;

    // TODO: Use local copy if it is a byval arg.
    if (Flags.isByVal())
      report_fatal_error("Byval argument not yet supported");

    // Arguments that can be passed on register are stored in the RegsToPass
    // vector.
    // TODO: Support stack arguments.
    if (VA.isRegLoc())
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    else
      report_fatal_error("Stack argument not yet supported");
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other,
                        ArrayRef<SDValue>(&MemOpChains[0], MemOpChains.size()));

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The Glue is
  // necessary since all emitted instructions must be stuck together.
  SDValue Glue;
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, Glue);
    Glue = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress/ExternalSymbol node, turn it into a
  // TargetGlobalAddress/TargetExternalSymbol node so that legalize won't
  // split it.
  // TODO: Add target-specific opflags to lower to fixups.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, MVT::i16);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i16);

  // The first call operand is the chain and the second is the target address.
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are known live
  // into the call.
  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  // Add a register mask operand representing the call-preserved registers.
  if (!IsTailCall) {
    const uint32_t *Mask = Subtarget.getRegisterInfo()->getCallPreservedMask(
        DAG.getMachineFunction(), CallConv);
    assert(Mask && "Missing call preserved mask for calling convention");
    Ops.push_back(DAG.getRegisterMask(Mask));
  }

  // As the last operand, glue the call to the argument copies, if any.
  if (Glue.getNode())
    Ops.push_back(Glue);

  // Emit the call.
  // TODO: Handle tail calls.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(AGCISD::CALL, DL, NodeTys, Ops);
  DAG.addNoMergeSiteInfo(Chain.getNode(), CLI.NoMerge);
  Glue = Chain.getValue(1);

  // Mark the end of the call, which is glued to the call itself.
  EVT PtrVT = getPointerTy(DAG.getDataLayout());
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getConstant(NumBytes, DL, PtrVT, true),
                             DAG.getConstant(0, DL, PtrVT, true), Glue, DL);
  Glue = Chain.getValue(1);

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState RetCCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                    *DAG.getContext());
  RetCCInfo.AnalyzeCallResult(Ins, RetCC_AGC);

  // Copy all of the result registers out of their specified physreg.
  for (auto &VA : RVLocs) {
    // Copy the value out.
    Chain = DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VA.getValVT(), Glue)
                .getValue(1);
    Glue = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

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
  case AGCISD::CALL:
    return "AGCISD::CALL";
  case AGCISD::BR_CC_ZERO:
    return "AGCISD::BR_CC_ZERO";
  }
  return nullptr;
}
