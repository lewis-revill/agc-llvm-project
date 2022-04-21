//===-- AGCAsmParser.cpp - Parse AGC code to MCInst instructions ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the definition of the AGCAsmParser class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/AGCMCTargetDesc.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

// Temporary - This should be defined in AGCInstrInfo.h
namespace AGCII {
enum { IsExtracode = 1 << 3 };
}

namespace {
class AGCAsmParser : public MCTargetAsmParser {
  bool ParsedExtend = false;
  bool ParsingExtracode = false;

  SMLoc getLoc() { return getParser().getTok().getLoc(); }

  bool ParseDirective(AsmToken DirectiveID) override;

  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;
  OperandMatchResultTy tryParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  unsigned
  checkEarlyTargetMatchPredicate(MCInst &Inst,
                                 const OperandVector &Operands) override;

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

// Auto-generated instruction matching functions
#define GET_ASSEMBLER_HEADER
#include "AGCGenAsmMatcher.inc"

  OperandMatchResultTy parseImmediate(OperandVector &Operands);
  OperandMatchResultTy parseRegister(OperandVector &Operands);
  bool parseOperand(OperandVector &Operands, StringRef Mnemonic);

public:
  enum AGCMatchResultTy {
    Match_IgnoredExtend = FIRST_TARGET_MATCH_RESULT_TY,
    Match_ExtracodeFail,
    Match_NonExtracodeFail,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "AGCGenAsmMatcher.inc"
  };

  AGCAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
               const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, STI, MII) {}
};

/// AGCOperand - Instances of this class represent a parsed machine operand.
struct AGCOperand : public MCParsedAsmOperand {

  enum KindTy {
    Token,
    Register,
    Immediate,
  } Kind;

  struct RegOp {
    MCRegister RegNum;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  SMLoc StartLoc;
  SMLoc EndLoc;
  union {
    StringRef Tok;
    RegOp Reg;
    ImmOp Imm;
  };

  AGCOperand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}

public:
  AGCOperand(const AGCOperand &o) : MCParsedAsmOperand() {
    Kind = o.Kind;
    StartLoc = o.StartLoc;
    EndLoc = o.EndLoc;
    switch (Kind) {
    case Token:
      Tok = o.Tok;
      break;
    case Register:
      Reg = o.Reg;
      break;
    case Immediate:
      Imm = o.Imm;
      break;
    }
  }

  bool isToken() const override { return Kind == Token; }
  bool isReg() const override { return Kind == Register; }
  bool isImm() const override { return Kind == Immediate; }
  bool isMem() const override { return false; }

  bool isConstantImm() const {
    return isImm() && getImm()->getKind() == MCExpr::Constant;
  }

  int64_t getConstantImm() const {
    uint64_t Val = static_cast<const MCConstantExpr *>(getImm())->getValue();
    return Val;
  }

  bool isSymbolRef() const {
    return isImm() && getImm()->getKind() == MCExpr::SymbolRef;
  }

  bool isMem12() const {
    return (isConstantImm() && isUInt<12>(getConstantImm())) || isSymbolRef();
  }

  bool isIO9() const {
    return (isConstantImm() && isUInt<9>(getConstantImm())) || isSymbolRef();
  }

  /// getStartLoc - Gets location of the first token of this operand
  SMLoc getStartLoc() const override { return StartLoc; }
  /// getEndLoc - Gets location of the last token of this operand
  SMLoc getEndLoc() const override { return EndLoc; }

  StringRef getToken() const {
    assert(Kind == Token && "Invalid type access!");
    return Tok;
  }

  unsigned getReg() const override {
    assert(Kind == Register && "Invalid type access!");
    return Reg.RegNum.id();
  }

  const MCExpr *getImm() const {
    assert(Kind == Immediate && "Invalid type access!");
    return Imm.Val;
  }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case Token:
      OS << "'" << getToken() << "'";
      break;
    case Register:
      OS << "<register R" << getReg() << ">";
      break;
    case Immediate:
      OS << *getImm();
      break;
    }
  }

  static std::unique_ptr<AGCOperand> createToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<AGCOperand>(Token);
    Op->Tok = Str;
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<AGCOperand> createReg(unsigned RegNo, SMLoc S,
                                               SMLoc E) {
    auto Op = std::make_unique<AGCOperand>(Register);
    Op->Reg.RegNum = RegNo;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<AGCOperand> createImm(const MCExpr *Val, SMLoc S,
                                               SMLoc E) {
    auto Op = std::make_unique<AGCOperand>(Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    assert(Expr && "Expr shouldn't be null!");
    if (auto *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  // Used by the TableGen Code
  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  // Used by the TableGen Code
  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }
};
} // namespace

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "AGCGenAsmMatcher.inc"

bool AGCAsmParser::ParseDirective(AsmToken DirectiveID) { return true; }

bool AGCAsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                 SMLoc &EndLoc) {
  if (tryParseRegister(RegNo, StartLoc, EndLoc) != MatchOperand_Success)
    return Error(StartLoc, "invalid register name");
  return false;
}

OperandMatchResultTy AGCAsmParser::tryParseRegister(unsigned &RegNo,
                                                    SMLoc &StartLoc,
                                                    SMLoc &EndLoc) {
  const AsmToken &Tok = getParser().getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  RegNo = 0;
  StringRef Name = getLexer().getTok().getIdentifier();

  RegNo = MatchRegisterName(Name);
  if (RegNo == AGC::NoRegister)
    return MatchOperand_NoMatch;

  getParser().Lex(); // Eat identifier token.
  return MatchOperand_Success;
}

OperandMatchResultTy AGCAsmParser::parseRegister(OperandVector &Operands) {
  switch (getLexer().getKind()) {
  default:
    return MatchOperand_NoMatch;
  case AsmToken::Identifier:
    break;
  }

  StringRef Name = getLexer().getTok().getIdentifier();

  MCRegister RegNo = MatchRegisterName(Name);
  if (RegNo == AGC::NoRegister)
    return MatchOperand_NoMatch;

  SMLoc StartLoc = getLoc();
  SMLoc EndLoc = SMLoc::getFromPointer(StartLoc.getPointer() + Name.size());
  getLexer().Lex();
  Operands.push_back(AGCOperand::createReg(RegNo, StartLoc, EndLoc));

  return MatchOperand_Success;
}

OperandMatchResultTy AGCAsmParser::parseImmediate(OperandVector &Operands) {
  switch (getLexer().getKind()) {
  default:
    return MatchOperand_NoMatch;
  case AsmToken::Minus:
  case AsmToken::Plus:
  case AsmToken::Integer:
  case AsmToken::Identifier:
    break;
  }

  const MCExpr *IdVal;
  SMLoc StartLoc = getLoc();
  if (getParser().parseExpression(IdVal))
    return MatchOperand_ParseFail;

  if (auto *CE = dyn_cast<MCConstantExpr>(IdVal))
    IdVal = MCConstantExpr::create(CE->getValue(), getContext());

  SMLoc EndLoc = SMLoc::getFromPointer(StartLoc.getPointer() - 1);
  Operands.push_back(AGCOperand::createImm(IdVal, StartLoc, EndLoc));
  return MatchOperand_Success;
}

/// Looks at a token type and creates the relevant operand from this
/// information, adding to Operands. If operand was parsed, returns false, else
/// true.
bool AGCAsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {
  // TODO: Custom operand parsing?

  // Attempt to parse token as a register.
  if (parseRegister(Operands) == MatchOperand_Success)
    return false;

  // Attempt to parse token as an immediate
  if (parseImmediate(Operands) == MatchOperand_Success)
    return false;

  // Finally we have exhausted all options and must declare defeat.
  return Error(getLoc(), "unknown operand");
}

bool AGCAsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                    SMLoc NameLoc, OperandVector &Operands) {
  ParsingExtracode = ParsedExtend;
  ParsedExtend = false;

  // First operand is token for instruction
  Operands.push_back(AGCOperand::createToken(Name, NameLoc));

  // If there are no more operands, then finish
  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  // Try parsing first operand.
  if (parseOperand(Operands, Name))
    return true;

  // Only 0/1 operand instructions are implemented.
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLoc();
    getParser().eatToEndOfStatement();
    return Error(Loc, "unexpected token");
  }

  getParser().Lex(); // Consume the EndOfStatement.
  return false;
}

// Handle extracode instructions.
unsigned
AGCAsmParser::checkEarlyTargetMatchPredicate(MCInst &Inst,
                                             const OperandVector &Operands) {
  unsigned Opcode = Inst.getOpcode();

  // Indicate that the next instruction parsed should be an extracode when an
  // EXTEND instruction is encountered.
  if (Opcode == AGC::EXTEND) {
    ParsedExtend = true;
    return Match_IgnoredExtend;
  }

  // TODO: Handle INDEX instruction.

  bool InstIsExtracode = MII.get(Opcode).TSFlags & AGCII::IsExtracode;

  // Check that extracode instructions are preceded by an EXTEND instruction.
  if (!ParsingExtracode)
    return InstIsExtracode ? Match_ExtracodeFail : Match_Success;

  // Check that non-extracode instructions are not preceded by an EXTEND
  // instruction.
  return InstIsExtracode ? Match_Success : Match_NonExtracodeFail;

  return Match_Success;
}

bool AGCAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                           OperandVector &Operands,
                                           MCStreamer &Out, uint64_t &ErrorInfo,
                                           bool MatchingInlineAsm) {
  MCInst Inst;

  switch (MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm)) {
  default:
    llvm_unreachable("Unknown match type detected!");
  case Match_IgnoredExtend:
    return false;
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_MnemonicFail:
    return Error(IDLoc, "unrecognized instruction mnemonic");
  case Match_ExtracodeFail:
    return Error(IDLoc, "extracode instruction should be prefixed with EXTEND");
  case Match_NonExtracodeFail:
    return Error(IDLoc, "instruction prefixed with EXTEND is not an extracode");
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0U) {
      if (ErrorInfo >= Operands.size())
        return Error(ErrorLoc, "too few operands for instruction");

      ErrorLoc = Operands[ErrorInfo]->getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }
    return Error(ErrorLoc, "invalid operand for instruction");
  }
  case Match_InvalidMem12: {
    SMLoc ErrorLoc = Operands[ErrorInfo]->getStartLoc();
    return Error(ErrorLoc, "memory address must be an immediate in the range "
                           "[0, 4095] or a symbol");
  }
  case Match_InvalidIO9: {
    SMLoc ErrorLoc = Operands[ErrorInfo]->getStartLoc();
    return Error(ErrorLoc, "IO channel address must be an immediate in the "
                           "range [0, 511] or a symbol");
  }
  }
}

extern "C" void LLVMInitializeAGCAsmParser() {
  RegisterMCAsmParser<AGCAsmParser> X(getTheAGCTarget());
}
