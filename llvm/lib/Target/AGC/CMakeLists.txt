add_llvm_component_group(AGC)

set(LLVM_TARGET_DEFINITIONS AGC.td)

tablegen(LLVM AGCGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM AGCGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM AGCGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM AGCGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(AGCCommonTableGen)

add_llvm_target(AGCCodeGen
  AGCTargetMachine.cpp

  LINK_COMPONENTS
  AGCDesc
  AGCInfo
  CodeGen
  Support
  Target

  ADD_TO_COMPONENT
  AGC
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
