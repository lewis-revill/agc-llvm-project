add_llvm_component_group(AGC)

set(LLVM_TARGET_DEFINITIONS AGC.td)

tablegen(LLVM AGCGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM AGCGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM AGCGenCallingConv.inc -gen-callingconv)
tablegen(LLVM AGCGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM AGCGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM AGCGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM AGCGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM AGCGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM AGCGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM AGCGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(AGCCommonTableGen)

add_llvm_target(AGCCodeGen
  AGCAsmPrinter.cpp
  AGCExpandPseudos.cpp
  AGCFrameLowering.cpp
  AGCInstrInfo.cpp
  AGCISelDAGToDAG.cpp
  AGCISelLowering.cpp
  AGCMCInstLower.cpp
  AGCRegisterInfo.cpp
  AGCSubtarget.cpp
  AGCTargetMachine.cpp

  LINK_COMPONENTS
  AGCDesc
  AGCInfo
  AsmPrinter
  CodeGen
  Core
  MC
  SelectionDAG
  Support
  Target

  ADD_TO_COMPONENT
  AGC
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
