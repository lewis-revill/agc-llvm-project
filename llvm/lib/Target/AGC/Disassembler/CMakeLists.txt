add_llvm_component_library(LLVMAGCDisassembler
  AGCDisassembler.cpp

  LINK_COMPONENTS
  AGCInfo
  MC
  MCDisassembler
  Support

  ADD_TO_COMPONENT
  AGC
  )
