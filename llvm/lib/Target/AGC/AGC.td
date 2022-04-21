//===-- AGC.td - AGC Target Description --------------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Describe the AGC tablegen target
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register file, instruction descriptions
//===----------------------------------------------------------------------===//

include "AGCRegisterInfo.td"
include "AGCInstrInfo.td"

def AGCInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// AGC processors
//===----------------------------------------------------------------------===//

// Currently only block 2 support is planned.
def : ProcessorModel<"agc-block2", NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// Define the AGC target
//===----------------------------------------------------------------------===//

def AGC : Target {
  let InstructionSet = AGCInstrInfo;
}
