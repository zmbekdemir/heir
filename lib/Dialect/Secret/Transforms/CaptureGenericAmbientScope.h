#ifndef LIB_DIALECT_SECRET_TRANSFORMS_CAPTUREGENERICAMBIENTSCOPE_H_
#define LIB_DIALECT_SECRET_TRANSFORMS_CAPTUREGENERICAMBIENTSCOPE_H_

#include "mlir/include/mlir/Pass/Pass.h"  // from @llvm-project

namespace mlir {
namespace heir {
namespace secret {

#define GEN_PASS_DECL_SECRETCAPTUREGENERICAMBIENTSCOPE
#include "lib/Dialect/Secret/Transforms/Passes.h.inc"

}  // namespace secret
}  // namespace heir
}  // namespace mlir

#endif  // LIB_DIALECT_SECRET_TRANSFORMS_CAPTUREGENERICAMBIENTSCOPE_H_
