add_llvm_component_library(LLVMAGCDesc
  AGCAsmBackend.cpp
  AGCMCAsmInfo.cpp
  AGCMCCodeEmitter.cpp
  AGCMCELFObjectWriter.cpp
  AGCMCTargetDesc.cpp

  LINK_COMPONENTS
  AGCInfo
  MC
  Support

  ADD_TO_COMPONENT
  AGC
  )
