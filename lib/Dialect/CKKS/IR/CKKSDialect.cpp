#include "lib/Dialect/CKKS/IR/CKKSDialect.h"

#include <optional>

#include "lib/Dialect/CKKS/IR/CKKSAttributes.h"
#include "lib/Dialect/CKKS/IR/CKKSOps.h"
#include "lib/Dialect/FHEHelpers.h"
#include "mlir/include/mlir/IR/Location.h"     // from @llvm-project
#include "mlir/include/mlir/IR/MLIRContext.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Types.h"        // from @llvm-project
#include "mlir/include/mlir/Support/LLVM.h"    // from @llvm-project

// Generated definitions
#include "lib/Dialect/CKKS/IR/CKKSDialect.cpp.inc"
#include "mlir/include/mlir/Support/LogicalResult.h"  // from @llvm-project
#define GET_ATTRDEF_CLASSES
#include "lib/Dialect/CKKS/IR/CKKSAttributes.cpp.inc"
#define GET_OP_CLASSES
#include "lib/Dialect/CKKS/IR/CKKSOps.cpp.inc"

namespace mlir {
namespace heir {
namespace ckks {

//===----------------------------------------------------------------------===//
// CKKS dialect.
//===----------------------------------------------------------------------===//

// Dialect construction: there is one instance per context and it registers its
// operations, types, and interfaces here.
void CKKSDialect::initialize() {
  addAttributes<
#define GET_ATTRDEF_LIST
#include "lib/Dialect/CKKS/IR/CKKSAttributes.cpp.inc"
      >();
  addOperations<
#define GET_OP_LIST
#include "lib/Dialect/CKKS/IR/CKKSOps.cpp.inc"
      >();
}

LogicalResult MulOp::verify() { return verifyMulOp(this); }

LogicalResult RotateOp::verify() { return verifyRotateOp(this); }

LogicalResult RelinearizeOp::verify() { return verifyRelinearizeOp(this); }

LogicalResult RescaleOp::verify() {
  return verifyModulusSwitchOrRescaleOp(this);
}

LogicalResult AddOp::inferReturnTypes(
    MLIRContext *ctx, std::optional<Location>, AddOp::Adaptor adaptor,
    SmallVectorImpl<Type> &inferredReturnTypes) {
  return inferAddOpReturnTypes(ctx, adaptor, inferredReturnTypes);
}

LogicalResult SubOp::inferReturnTypes(
    MLIRContext *ctx, std::optional<Location>, SubOp::Adaptor adaptor,
    SmallVectorImpl<Type> &inferredReturnTypes) {
  return inferAddOpReturnTypes(ctx, adaptor, inferredReturnTypes);
}

LogicalResult MulOp::inferReturnTypes(
    MLIRContext *ctx, std::optional<Location>, MulOp::Adaptor adaptor,
    SmallVectorImpl<Type> &inferredReturnTypes) {
  return inferMulOpReturnTypes(ctx, adaptor, inferredReturnTypes);
}

LogicalResult RelinearizeOp::inferReturnTypes(
    MLIRContext *ctx, std::optional<Location>, RelinearizeOp::Adaptor adaptor,
    SmallVectorImpl<Type> &inferredReturnTypes) {
  return inferRelinearizeOpReturnTypes(ctx, adaptor, inferredReturnTypes);
}

LogicalResult ExtractOp::verify() { return verifyExtractOp(this); }

}  // namespace ckks
}  // namespace heir
}  // namespace mlir
