// RUN: heir-opt --polynomial-to-mod-arith %s | FileCheck %s

!eval_poly_ty = !polynomial.polynomial<ring=<coefficientType=i64>>
#eval_poly = #polynomial.typed_int_polynomial<4+x+x**2> : !eval_poly_ty

func.func @test_eval() -> i64 {
    %c6 = arith.constant 6 : i64
    %0 = polynomial.eval #eval_poly, %c6 : i64
    return %0 : i64
}


// CHECK: %c1_i64 = arith.constant 1 : i64
