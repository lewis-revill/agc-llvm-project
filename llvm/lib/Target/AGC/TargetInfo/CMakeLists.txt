add_llvm_component_library(LLVMAGCInfo
  AGCTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  AGC
  )
