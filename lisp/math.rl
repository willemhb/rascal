(module math
  "This is the rascal builtin math library.

   Mostly just wrappers for functions in <math.h>."
  (import (core base))
  (export (pi e
           sin cos tan asin acos atan
           pow sqrt hypot))

  (begin
    ;; Common mathematical constants.
    (val pi 3.1415926535)
    (val e  2.7182818285)

    ;; Trig functions
    (fun sin
      "The `sin` function."
      ((Number x))
      (c-call :sin :float64 (:float64) (x)))

    (fun cos
      "The `cos` function."
      ((Number x))
      (c-call :cos :float64 (:float64) (x)))

    (fun tan
      "The `tan` function."
      ((Number x))
      (c-call :tan :float64 (:float64) (x)))))
