add_llvm_component_group(AGC)

set(LLVM_TARGET_DEFINITIONS AGC.td)

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
