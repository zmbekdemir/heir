#include "lib/Conversion/MemrefToArith/MemrefToArith.h"

namespace mlir {
namespace heir {
namespace {
#define GEN_PASS_REGISTRATION
#include "lib/Conversion/MemrefToArith/MemrefToArith.h.inc"

bool register_all_passes = ([] { registerMemrefToArithPasses(); }(), true);

}  // end namespace
}  // namespace heir
}  // namespace mlir
