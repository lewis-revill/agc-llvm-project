add_llvm_component_library(LLVMAGCAsmPrinter
  AGCInstPrinter.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  AGC
  )
