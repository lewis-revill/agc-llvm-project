add_llvm_component_library(LLVMAGCAsmParser
  AGCAsmParser.cpp

  LINK_COMPONENTS
  AGCDesc
  AGCInfo
  MC
  MCParser
  Support

  ADD_TO_COMPONENT
  AGC
  )
