//===-- AGCCallingConv.td - Calling Conventions for AGC ----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the AGC architecture.
//
//===----------------------------------------------------------------------===//

// AGC C return-value convention.
def RetCC_AGC : CallingConv<[
  // We can return values in general purpose registers.
  CCIfType<[i32], CCAssignToReg<[RD50]>>,
  CCIfType<[i16], CCAssignToReg<[R50, R51]>>
]>;

// AGC C Calling convention.
def CC_AGC : CallingConv<[
  // Use some general purpose registers to store arguments.
  CCIfType<[i32], CCAssignToReg<[RD50, RD52, RD54, RD56]>>,
  CCIfType<[i16], CCAssignToReg<[R50, R51, R52, R53, R54, R55, R56, R57]>>,

  // Fall back to the emulated stack.
  CCAssignToStack<2, 2>
]>;

// TODO: Define more callee-saved registers.
def CSR : CalleeSavedRegs<(add R2)>;
