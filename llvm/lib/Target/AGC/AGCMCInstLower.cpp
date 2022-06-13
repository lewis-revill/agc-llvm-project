//===-- AGCMCInstLower.cpp - Convert an AGC MachineInstr to an MCInst -------=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower AGC MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "AGC.h"
#include "MCTargetDesc/AGCBaseInfo.h"
#include "MCTargetDesc/AGCMCExpr.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

static MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                                    AsmPrinter &AP) {
  MCContext &Ctx = AP.OutContext;
  AGCMCExpr::VariantKind Kind;

  const MCExpr *ME =
      MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, Ctx);
  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on operand");
  case AGCII::MO_None:
    Kind = AGCMCExpr::VK_AGC_None;
    break;
  case AGCII::MO_CPI:
    Kind = AGCMCExpr::VK_AGC_CPI;
    // We actually don't want LLVM to emit/use any 'CPI symbols', but instead we
    // want to encode the constant in the relocation for the linker to analyse
    // directly.
    ME = MCConstantExpr::create(MO.getIndex(), Ctx);
    break;
  case AGCII::MO_BANKS:
    Kind = AGCMCExpr::VK_AGC_BANKS;
    break;
  case AGCII::MO_LO12:
    Kind = AGCMCExpr::VK_AGC_LO12;
    break;
  }

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset())
    ME = MCBinaryExpr::createAdd(
        ME, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  if (Kind != AGCMCExpr::VK_AGC_None)
    ME = AGCMCExpr::create(ME, Kind, Ctx);
  return MCOperand::createExpr(ME);
}

bool llvm::LowerAGCMachineOperandToMCOperand(const MachineOperand &MO,
                                             MCOperand &MCOp, AsmPrinter &AP) {
  switch (MO.getType()) {
  default:
    report_fatal_error("LowerAGCMachineInstrToMCInst: unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
    break;
  case MachineOperand::MO_RegisterMask:
    // Regmasks are like implicit defs.
    return false;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    break;
  case MachineOperand::MO_GlobalAddress:
    MCOp = lowerSymbolOperand(MO, AP.getSymbolPreferLocal(*MO.getGlobal()), AP);
    break;
  case MachineOperand::MO_ExternalSymbol:
    MCOp = lowerSymbolOperand(
        MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP);
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    MCOp = lowerSymbolOperand(MO, AP.GetCPISymbol(MO.getIndex()), AP);
    break;
  }
  return true;
}

void llvm::LowerAGCMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                        AsmPrinter &AP) {

  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    if (LowerAGCMachineOperandToMCOperand(MO, MCOp, AP))
      OutMI.addOperand(MCOp);
  }
}
