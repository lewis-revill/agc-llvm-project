//===-- AGCISelDAGToDAG.cpp - A DAG to DAG instruction selector for AGC ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares an instruction selector for the AGC target.
//
//===----------------------------------------------------------------------===//

#include "AGCISelDAGToDAG.h"
#include "AGC.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "agc-isel"

void AGCDAGToDAGISel::Select(SDNode *Node) {
  // If we have a custom node, we have already selected
  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(dbgs() << "== " << Node << "\n");
    Node->setNodeId(-1);
    return;
  }

  switch (Node->getOpcode()) {
  default:
    break;
  case ISD::Constant:
    SelectConstant(Node);
    return;
  }

  // Select the default instruction.
  SelectCode(Node);
}

void AGCDAGToDAGISel::SelectConstant(SDNode *Node) {
  MVT VT = Node->getSimpleValueType(0);
  SDLoc DL(Node);

  assert(VT == MVT::i16 && "Cannot yet materialize >16-bit constants");

  auto *ConstNode = cast<ConstantSDNode>(Node);

  if (ConstNode->isZero()) {
    SDValue New =
        CurDAG->getCopyFromReg(CurDAG->getEntryNode(), DL, AGC::R7, VT);
    ReplaceNode(Node, New.getNode());
    return;
  }

  // Create our own load constant pseudo instruction to expand later.
  int64_t Imm = ConstNode->getSExtValue();
  SDNode *LC = CurDAG->getMachineNode(AGC::PseudoLoadConst, DL, VT,
                                      CurDAG->getTargetConstant(Imm, DL, VT));
  ReplaceNode(Node, LC);
}

// This pass converts a legalized DAG into an AGC-specific DAG, ready for
// instruction scheduling.
FunctionPass *llvm::createAGCISelDag(AGCTargetMachine &TM) {
  return new AGCDAGToDAGISel(TM);
}
