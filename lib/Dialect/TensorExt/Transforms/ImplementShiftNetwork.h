#ifndef LIB_DIALECT_TENSOREXT_TRANSFORMS_IMPLEMENTSHIFTNETWORK_H_
#define LIB_DIALECT_TENSOREXT_TRANSFORMS_IMPLEMENTSHIFTNETWORK_H_

#include "mlir/include/mlir/Pass/Pass.h"  // from @llvm-project

namespace mlir {
namespace heir {
namespace tensor_ext {

#define GEN_PASS_DECL_IMPLEMENTSHIFTNETWORK
#include "lib/Dialect/TensorExt/Transforms/Passes.h.inc"

}  // namespace tensor_ext
}  // namespace heir
}  // namespace mlir

#endif  // LIB_DIALECT_TENSOREXT_TRANSFORMS_IMPLEMENTSHIFTNETWORK_H_
