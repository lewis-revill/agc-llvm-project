//===- PassDetail.h - SPIRV Pass class details ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef DIALECT_SPIRV_TRANSFORMS_PASSDETAIL_H_
#define DIALECT_SPIRV_TRANSFORMS_PASSDETAIL_H_

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

namespace mlir {

namespace spirv {
class ModuleOp;
} // namespace spirv

#define GEN_PASS_CLASSES
#include "mlir/Dialect/SPIRV/Transforms/Passes.h.inc"

} // namespace mlir

#endif // DIALECT_SPIRV_TRANSFORMS_PASSDETAIL_H_
