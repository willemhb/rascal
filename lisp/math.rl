(ns math
  "Math standard library (mostly ffi calls to functions defined in <math.h>)."

  ;; union types.
  (union Signed
    "All signed integer types."
    (Small Big))

  (union Int
    "All integer types (signed or unsigned)."
    (Small Big Arity))

  (union Num
    "All valid numeric types."
    (Small Big Arity Real Ratio))

  ;; constants.
  (val pi
    "Mathematical constant pi."
    3.14159)

  (val e
    "Mathematical constant e."
    2.71828)

  ;; Examples using famous integer operations.
  (fun fac
    "Compute the `n`th factorial."
    [n])

  (fun pow
    "Computes `x` to the `nth` power."
    [i n])

  (fun fib
    "Computes the `n`th Fibonacci number."
    [n])

  (method fac
    "Various implementations."
    [n: Int] @when (< n 0)
      (throw :bad-argument $"Can't compute factorial of negative number ${n}.")
    [n: Int]
      (label
        [_: 0, ac: Int]
          ac
        [_: 1, ac: Int]
          ac
        [n: Int, ac: Int]
          (loop (dec n) (* ac n))
        ;; Entry.
        (loop n 1)))

  (method pow
    "Defined for `Int`s."
    [i: Int, n: Int]
      (label [i, n, ac]
        (cond
          (= n 0)   ac
          (even? n) (loop (* i i) (/ n 2) ac)
          otherwise (loop i (- n 1) (* i ac)))
        ;; Entry.
        (if (< 0 n)
          (Ratio 1 (loop i (- n) 1))
          (loop i n 1))))

  (method fib
    "Defined for `Int`s."
    [n: Int] @when (< n 0)
      (throw :bad-argument $"Can't compute negative Fibonacci number ${n}.")
    [n: Int]
      (label
        [_: 0, a: Int, _: Int]
          a
        [n: Int, a: Int, b: Int]
          (loop (dec n) b (+ a b))
        ;; Entry.
        (loop n 0 1))))
