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

  // Select the default instruction.
  SelectCode(Node);
}

// This pass converts a legalized DAG into an AGC-specific DAG, ready for
// instruction scheduling.
FunctionPass *llvm::createAGCISelDag(AGCTargetMachine &TM) {
  return new AGCDAGToDAGISel(TM);
}
