#include "lib/Dialect/ModArith/IR/ModArithDialect.h"

#include <cassert>
#include <optional>

#include "llvm/include/llvm/ADT/TypeSwitch.h"            // from @llvm-project
#include "mlir/include/mlir/IR/BuiltinAttributes.h"      // from @llvm-project
#include "mlir/include/mlir/IR/BuiltinTypes.h"           // from @llvm-project
#include "mlir/include/mlir/IR/DialectImplementation.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Location.h"               // from @llvm-project
#include "mlir/include/mlir/IR/MLIRContext.h"            // from @llvm-project
#include "mlir/include/mlir/IR/OpImplementation.h"       // from @llvm-project
#include "mlir/include/mlir/IR/OperationSupport.h"       // from @llvm-project
#include "mlir/include/mlir/IR/TypeUtilities.h"          // from @llvm-project
#include "mlir/include/mlir/Support/LLVM.h"              // from @llvm-project
#include "mlir/include/mlir/Support/LogicalResult.h"     // from @llvm-project

// NOLINTBEGIN(misc-include-cleaner): Required to define
// ModArithDialect, ModArithTypes, ModArithOps,
#include "lib/Dialect/ModArith/IR/ModArithOps.h"
#include "lib/Dialect/ModArith/IR/ModArithTypes.h"
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"  // from @llvm-project
// NOLINTEND(misc-include-cleaner)

// Generated definitions
#include "lib/Dialect/ModArith/IR/ModArithDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "lib/Dialect/ModArith/IR/ModArithTypes.cpp.inc"

#define GET_OP_CLASSES
#include "lib/Dialect/ModArith/IR/ModArithOps.cpp.inc"

namespace mlir {
namespace heir {
namespace mod_arith {

void ModArithDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "lib/Dialect/ModArith/IR/ModArithTypes.cpp.inc"
      >();
  addOperations<
#define GET_OP_LIST
#include "lib/Dialect/ModArith/IR/ModArithOps.cpp.inc"
      >();
}

/// Ensures that the underlying integer type is wide enough for the coefficient
template <typename OpType>
LogicalResult verifyModArithType(OpType op, ModArithType type) {
  APInt modulus = type.getModulus().getValue();
  unsigned bitWidth = modulus.getBitWidth();
  unsigned modWidth = modulus.getActiveBits();
  if (modWidth > bitWidth - 1)
    return op.emitOpError()
           << "underlying type's bitwidth must be 1 bit larger than "
           << "the modulus bitwidth, but got " << bitWidth
           << " while modulus requires width " << modWidth << ".";
  return success();
}

template <typename OpType>
LogicalResult verifySameWidth(OpType op, ModArithType modArithType,
                              IntegerType integerType) {
  unsigned bitWidth = modArithType.getModulus().getValue().getBitWidth();
  unsigned intWidth = integerType.getWidth();
  if (intWidth != bitWidth)
    return op.emitOpError()
           << "the result integer type should be of the same width as the "
           << "mod arith type width, but got " << intWidth
           << " while mod arith type width " << bitWidth << ".";
  return success();
}

LogicalResult ExtractOp::verify() {
  auto modArithType = getOperandModArithType(*this);
  auto integerType = getResultIntegerType(*this);
  auto result = verifySameWidth(*this, modArithType, integerType);
  if (result.failed()) return result;
  return verifyModArithType(*this, modArithType);
}

LogicalResult ReduceOp::verify() {
  return verifyModArithType(*this, getResultModArithType(*this));
}

LogicalResult AddOp::verify() {
  return verifyModArithType(*this, getResultModArithType(*this));
}

LogicalResult SubOp::verify() {
  return verifyModArithType(*this, getResultModArithType(*this));
}

LogicalResult MulOp::verify() {
  return verifyModArithType(*this, getResultModArithType(*this));
}

LogicalResult MacOp::verify() {
  return verifyModArithType(*this, getResultModArithType(*this));
}

LogicalResult BarrettReduceOp::verify() {
  auto inputType = getInput().getType();
  unsigned bitWidth;
  if (auto tensorType = dyn_cast<RankedTensorType>(inputType)) {
    bitWidth = tensorType.getElementTypeBitWidth();
  } else {
    auto integerType = dyn_cast<IntegerType>(inputType);
    assert(integerType &&
           "expected input to be a ranked tensor type or integer type");
    bitWidth = integerType.getWidth();
  }
  auto expectedBitWidth = (getModulus() - 1).getActiveBits();
  if (bitWidth < expectedBitWidth || 2 * expectedBitWidth < bitWidth) {
    return emitOpError()
           << "input bitwidth is required to be in the range [w, 2w], where w "
              "is the smallest bit-width that contains the range [0, modulus). "
              "Got "
           << bitWidth << " but w is " << expectedBitWidth << ".";
  }
  if (getModulus().slt(0))
    return emitOpError() << "provided modulus " << getModulus().getSExtValue()
                         << " is not a positive integer.";
  return success();
}

ParseResult ConstantOp::parse(OpAsmParser &parser, OperationState &result) {
  unsigned minBitwidth = 4;  // bitwidth assigned by parser to integer `1`
  Type parsedType;
  if (parser.parseOptionalKeyword("dense").succeeded()) {
    // Dense case
    // We parse the integers as a list, rather than an ArrayAttr, so we can more
    // easily convert them to the correct bitwidth (ArrayAttr forces I64)
    std::vector<APInt> parsedInts;
    if (parser.parseLess() ||
        parser.parseCommaSeparatedList(mlir::AsmParser::Delimiter::Square,
                                       [&] {
                                         APInt parsedInt;
                                         if (parser.parseInteger(parsedInt))
                                           return failure();
                                         parsedInts.push_back(parsedInt);
                                         return success();
                                       }) ||
        parser.parseGreater() || parser.parseColonType(parsedType))
      return failure();
    if (parsedInts.empty())
      return parser.emitError(parser.getNameLoc(),
                              "expected at least one integer in dense list.");

    unsigned maxWidth = 0;
    for (auto &parsedInt : parsedInts) {
      // zero becomes `i64` when parsed, so truncate back down to minBitwidth
      parsedInt = parsedInt.isZero() ? parsedInt.trunc(minBitwidth) : parsedInt;
      maxWidth = std::max(maxWidth, parsedInt.getBitWidth());
    }
    for (auto &parsedInt : parsedInts) {
      parsedInt = parsedInt.zextOrTrunc(maxWidth);
    }
    auto attr = DenseIntElementsAttr::get(
        RankedTensorType::get({static_cast<int64_t>(parsedInts.size())},
                              IntegerType::get(parser.getContext(), maxWidth)),
        parsedInts);
    result.addAttribute("value", attr);
  } else {
    // Scalar case
    APInt parsedInt;
    if (parser.parseInteger(parsedInt) || parser.parseColonType(parsedType))
      return failure();
    // zero becomes `i64` when parsed, so truncate back down to minBitwidth
    if (parsedInt.isZero()) parsedInt = parsedInt.trunc(minBitwidth);
    result.addAttribute(
        "value", IntegerAttr::get(IntegerType::get(parser.getContext(),
                                                   parsedInt.getBitWidth()),
                                  parsedInt));
  }
  result.addTypes(parsedType);
  return success();
}

LogicalResult ConstantOp::verify() {
  auto shapedType = dyn_cast<ShapedType>(getType());
  auto modType = dyn_cast<ModArithType>(getElementTypeOrSelf(getType()));
  auto denseAttr = dyn_cast<DenseIntElementsAttr>(getValue());
  auto intAttr = dyn_cast<IntegerAttr>(getValue());

  assert(modType &&
         "return type should be constrained to "
         "ModArithLike by its ODS definition/type constraints.");

  if (!!shapedType != !!denseAttr)
    return emitOpError("must have shaped type iff value is `dense`.");

  auto modBW = modType.getModulus().getValue().getBitWidth();

  if (intAttr) {
    if (intAttr.getValue().getBitWidth() > modBW)
      return emitOpError(
          "value's bitwidth must not be larger than underlying type.");
    return success();
  }

  if (denseAttr) {
    assert(denseAttr.getShapedType().hasStaticShape() &&
           "dense attribute should be guaranteed to have static shape.");

    if (!shapedType.hasStaticShape() ||
        shapedType.getShape() != denseAttr.getShapedType().getShape())
      return emitOpError("tensor shape must be static and match value shape.");

    if (denseAttr.getElementType().getIntOrFloatBitWidth() > modBW)
      return emitOpError(
          "values's bitwidth must not be larger than underlying type");

    return success();
  }
  // anything else: failure
  return emitOpError("value must be IntegerAttr or DenseIntElementsAttr.");
}

void ConstantOp::print(OpAsmPrinter &p) {
  p << " ";
  p.printAttributeWithoutType(getValue());
  p << " : ";
  p.printType(getOutput().getType());
}

}  // namespace mod_arith
}  // namespace heir
}  // namespace mlir
