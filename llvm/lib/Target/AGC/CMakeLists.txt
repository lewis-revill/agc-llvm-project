add_llvm_component_group(AGC)

set(LLVM_TARGET_DEFINITIONS AGC.td)

tablegen(LLVM AGCGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM AGCGenRegisterInfo.inc -gen-register-info)

add_public_tablegen_target(AGCCommonTableGen)

add_llvm_target(AGCCodeGen
  AGCTargetMachine.cpp

  LINK_COMPONENTS
  AGCInfo
  CodeGen
  Support
  Target

  ADD_TO_COMPONENT
  AGC
  )

add_subdirectory(TargetInfo)
