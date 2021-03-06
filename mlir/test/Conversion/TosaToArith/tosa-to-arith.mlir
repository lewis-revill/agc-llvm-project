// RUN: mlir-opt --split-input-file --tosa-to-arith="include-apply-rescale=true" %s -verify-diagnostics -o -| FileCheck %s
// RUN: mlir-opt --split-input-file --tosa-to-arith="include-apply-rescale=false" %s -verify-diagnostics -o -| FileCheck --check-prefix="SCALE" %s

// CHECK-LABEL: func @const_test
func @const_test() -> (tensor<i32>) {
  // CHECK: [[C3:%.+]] = arith.constant dense<3> : tensor<i32>
  %0 = "tosa.const"() {value = dense<3> : tensor<i32>} : () -> tensor<i32>

  // CHECK: return [[C3]]
  return %0 : tensor<i32>
}

// -----

// CHECK-LABEL: @apply_scale_test_i32
func @apply_scale_test_i32(%arg0 : i32, %arg1 : i32, %arg2 : i8) -> (i32) {
  // CHECK-DAG: [[C1_8:%.+]] = arith.constant 1 : i8
  // CHECK-DAG: [[C1_32:%.+]] = arith.constant 1 : i32
  // CHECK-DAG: [[C1_64:%.+]] = arith.constant 1 : i64
  // CHECK-DAG: [[SHIFT_MINUS_ONE_8:%.+]] = arith.subi %arg2, [[C1_8]]

  // CHECK-DAG: [[SHIFT_32:%.+]] = arith.extsi %arg2 : i8 to i32
  // CHECK-DAG: [[SHIFT_MINUS_ONE_64:%.+]] = arith.extsi [[SHIFT_MINUS_ONE_8]] : i8 to i64
  // CHECK-DAG: [[SHIFTED_64:%.+]] = arith.shli [[C1_64]], [[SHIFT_MINUS_ONE_64]]

  // CHECK-DAG: [[C0_32:%.+]] = arith.constant 0 : i32
  // CHECK-DAG: [[C30_32:%.+]] = arith.constant 30 : i32
  // CHECK-DAG: [[SECOND_BIAS:%.+]] = arith.shli [[C1_32]], [[C30_32]]
  // CHECK-DAG: [[SECOND_BIAS_64:%.+]] = arith.extsi [[SECOND_BIAS]] : i32 to i64
  // CHECK-DAG: [[POSITIVE_ROUND:%.+]] = arith.addi [[SHIFTED_64]], [[SECOND_BIAS_64]]
  // CHECK-DAG: [[NEGATIVE_ROUND:%.+]] = arith.subi [[SHIFTED_64]], [[SECOND_BIAS_64]]
  // CHECK-DAG: [[VALUE_NEGATIVE:%.+]] = arith.cmpi sge, %arg0, [[C0_32]] : i32
  // CHECK-DAG: [[DOUBLE_ROUNDED:%.+]] = arith.select [[VALUE_NEGATIVE]], [[POSITIVE_ROUND]], [[NEGATIVE_ROUND]] : i64
  // CHECK-DAG: [[C32_32:%.+]] = arith.constant 32 : i32
  // CHECK-DAG: [[IS_32BIT_SHIFT:%.+]] = arith.cmpi sge, [[SHIFT_32]], [[C32_32]]
  // CHECK-DAG: [[ROUND:%.+]] = arith.select [[IS_32BIT_SHIFT]], [[DOUBLE_ROUNDED]], [[SHIFTED_64]]

  // CHECK-DAG: [[VAL_64:%.+]] = arith.extsi %arg0 : i32 to i64
  // CHECK-DAG: [[MULTIPLY_64:%.+]] = arith.extsi %arg1 : i32 to i64
  // CHECK-DAG: [[SHIFT_64:%.+]] = arith.extsi %arg2 : i8 to i64
  // CHECK-DAG: [[SCALED:%.+]] = arith.muli [[VAL_64]], [[MULTIPLY_64]]
  // CHECK-DAG: [[BIASED:%.+]] = arith.addi [[SCALED]], [[ROUND]]
  // CHECK-DAG: [[DOWNSHIFTED:%.+]] = arith.shrsi [[BIASED]], [[SHIFT_64]]
  // CHECK: [[TRUNCATED:%.+]] = arith.trunci [[DOWNSHIFTED]]

  // SCALE: "tosa.apply_scale"
  %0 = "tosa.apply_scale"(%arg0, %arg1, %arg2) {double_round = true} : (i32, i32, i8) -> i32
  return %0 : i32
}

// -----

// CHECK-LABEL: @apply_scale_test_vector
func @apply_scale_test_vector(%arg0 : vector<4xi32>, %arg1 : vector<4xi32>, %arg2 : vector<4xi8>) -> (vector<4xi32>) {
  // CHECK-DAG: [[C1_8:%.+]] = arith.constant dense<1> : vector<4xi8>
  // CHECK-DAG: [[C1_32:%.+]] = arith.constant dense<1> : vector<4xi32>
  // CHECK-DAG: [[C1_64:%.+]] = arith.constant dense<1> : vector<4xi64>
  // CHECK-DAG: [[SHIFT_MINUS_ONE_8:%.+]] = arith.subi %arg2, [[C1_8]]

  // CHECK-DAG: [[SHIFT_32:%.+]] = arith.extsi %arg2 : vector<4xi8> to vector<4xi32>
  // CHECK-DAG: [[SHIFT_MINUS_ONE_64:%.+]] = arith.extsi [[SHIFT_MINUS_ONE_8]] : vector<4xi8> to vector<4xi64>
  // CHECK-DAG: [[SHIFTED_64:%.+]] = arith.shli [[C1_64]], [[SHIFT_MINUS_ONE_64]]

  // CHECK-DAG: [[C0_32:%.+]] = arith.constant dense<0> : vector<4xi32>
  // CHECK-DAG: [[C30_32:%.+]] = arith.constant dense<30> : vector<4xi32>
  // CHECK-DAG: [[SECOND_BIAS:%.+]] = arith.shli [[C1_32]], [[C30_32]]
  // CHECK-DAG: [[SECOND_BIAS_64:%.+]] = arith.extsi [[SECOND_BIAS]] : vector<4xi32> to vector<4xi64>
  // CHECK-DAG: [[POSITIVE_ROUND:%.+]] = arith.addi [[SHIFTED_64]], [[SECOND_BIAS_64]]
  // CHECK-DAG: [[NEGATIVE_ROUND:%.+]] = arith.subi [[SHIFTED_64]], [[SECOND_BIAS_64]]
  // CHECK-DAG: [[VALUE_NEGATIVE:%.+]] = arith.cmpi sge, %arg0, [[C0_32]] : vector<4xi32>
  // CHECK-DAG: [[DOUBLE_ROUNDED:%.+]] = arith.select [[VALUE_NEGATIVE]], [[POSITIVE_ROUND]], [[NEGATIVE_ROUND]] : vector<4xi1>, vector<4xi64>
  // CHECK-DAG: [[C32_32:%.+]] = arith.constant dense<32> : vector<4xi32>
  // CHECK-DAG: [[IS_32BIT_SHIFT:%.+]] = arith.cmpi sge, [[SHIFT_32]], [[C32_32]]
  // CHECK-DAG: [[ROUND:%.+]] = arith.select [[IS_32BIT_SHIFT]], [[DOUBLE_ROUNDED]], [[SHIFTED_64]]

  // CHECK-DAG: [[VAL_64:%.+]] = arith.extsi %arg0 : vector<4xi32> to vector<4xi64>
  // CHECK-DAG: [[MULTIPLY_64:%.+]] = arith.extsi %arg1 : vector<4xi32> to vector<4xi64>
  // CHECK-DAG: [[SHIFT_64:%.+]] = arith.extsi %arg2 : vector<4xi8> to vector<4xi64>
  // CHECK-DAG: [[SCALED:%.+]] = arith.muli [[VAL_64]], [[MULTIPLY_64]]
  // CHECK-DAG: [[BIASED:%.+]] = arith.addi [[SCALED]], [[ROUND]]
  // CHECK-DAG: [[DOWNSHIFTED:%.+]] = arith.shrsi [[BIASED]], [[SHIFT_64]]
  // CHECK: [[TRUNCATED:%.+]] = arith.trunci [[DOWNSHIFTED]]

  %0 = "tosa.apply_scale"(%arg0, %arg1, %arg2) {double_round = true} : (vector<4xi32>, vector<4xi32>, vector<4xi8>) -> vector<4xi32>
  return %0 : vector<4xi32>
}

// -----

// CHECK-LABEL: @apply_scale_test_i48
func @apply_scale_test_i48(%arg0 : i48, %arg1 : i32, %arg2 : i8) -> (i32) {
  // CHECK-DAG: [[C1_8:%.+]] = arith.constant 1 : i8
  // CHECK-DAG: [[C1_32:%.+]] = arith.constant 1 : i32
  // CHECK-DAG: [[C1_64:%.+]] = arith.constant 1 : i64
  // CHECK-DAG: [[C30_32:%.+]] = arith.constant 30 : i32
  // CHECK-DAG: [[C0_32:%.+]] = arith.constant 0 : i48
  // CHECK-DAG: [[C32_32:%.+]] = arith.constant 32 : i32
  // CHECK-DAG: [[SHIFT_MINUS_ONE_8:%.+]] = arith.subi %arg2, [[C1_8]]
  // CHECK-DAG: [[SHIFT_32:%.+]] = arith.extsi %arg2 : i8 to i32
  // CHECK-DAG: [[SHIFT_MINUS_ONE_64:%.+]] = arith.extsi [[SHIFT_MINUS_ONE_8]] : i8 to i64
  // CHECK-DAG: [[SHIFTED_64:%.+]] = arith.shli [[C1_64]], [[SHIFT_MINUS_ONE_64]]
  // CHECK-DAG: [[SECOND_BIAS:%.+]] = arith.shli [[C1_32]], [[C30_32]]
  // CHECK-DAG: [[SECOND_BIAS_64:%.+]] = arith.extsi [[SECOND_BIAS]] : i32 to i64
  // CHECK-DAG: [[POSITIVE_ROUND:%.+]] = arith.addi [[SHIFTED_64]], [[SECOND_BIAS_64]]
  // CHECK-DAG: [[NEGATIVE_ROUND:%.+]] = arith.subi [[SHIFTED_64]], [[SECOND_BIAS_64]]
  // CHECK-DAG: [[VALUE_NEGATIVE:%.+]] = arith.cmpi sge, %arg0, [[C0_32]] : i48
  // CHECK-DAG: [[DOUBLE_ROUNDED:%.+]] = arith.select [[VALUE_NEGATIVE]], [[POSITIVE_ROUND]], [[NEGATIVE_ROUND]] : i64
  // CHECK-DAG: [[IS_32BIT_SHIFT:%.+]] = arith.cmpi sge, [[SHIFT_32]], [[C32_32]]
  // CHECK-DAG: [[ROUND:%.+]] = arith.select [[IS_32BIT_SHIFT]], [[DOUBLE_ROUNDED]], [[SHIFTED_64]]
  // CHECK-DAG: [[VAL_64:%.+]] = arith.extsi %arg0 : i48 to i64
  // CHECK-DAG: [[MULTIPLY_64:%.+]] = arith.extsi %arg1 : i32 to i64
  // CHECK-DAG: [[SHIFT_64:%.+]] = arith.extsi %arg2 : i8 to i64
  // CHECK-DAG: [[SCALED:%.+]] = arith.muli [[VAL_64]], [[MULTIPLY_64]]
  // CHECK-DAG: [[BIASED:%.+]] = arith.addi [[SCALED]], [[ROUND]]
  // CHECK-DAG: [[DOWNSHIFTED:%.+]] = arith.shrsi [[BIASED]], [[SHIFT_64]]
  // CHECK: [[TRUNCATED:%.+]] = arith.trunci [[DOWNSHIFTED]]
  %0 = "tosa.apply_scale"(%arg0, %arg1, %arg2) {double_round = true} : (i48, i32, i8) -> i32
  return %0 : i32
}
