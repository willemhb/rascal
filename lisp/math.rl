(ns math
  "Math standard library (mostly ffi calls to functions defined in <math.h>)."

  ;; union types.
  (union Signed
    "All signed integer types."
    (Small Big))

  (union Integer
    "All integer types (signed or unsigned)."
    (Small Big Arity))

  (union Number
    "All valid numeric types."
    (Small Big Arity Real Ratio))

  ;; constants.
  (val pi
    "Mathematical constant pi."
    3.14159)

  (val e
    "Mathematical constant e."
    2.71828)

  ;; promote.
  (fun promote
    "Coerce two numbers to nearest common type."
    ([x, y])
    ([x, y, & a] (fold promote [x, y, & a]))

  (method promote
    [x: Small, y: Small] [x, y])

  (method promote
    [x: Real, y: Real] [x, y])

  (method promote
    [x: Arity, y: Arity] [x, y])

  (method promote
    [x: Small, y: Real] [(Real x), y])

  (method promote
    [x: Real, y: Small] [x, (Real y)])

  (method promote
    [x: Small, y: Arity] [(Real x), (Real y)])

  (method promote
    [x: Arity, y: Small] [(Real x), (Real y)])

  ;; Constructor overloads.
  (method Real
    [x: Small]
    (*cast-small-to-real* x))

  (method Real
    [x: Arity]
    (*cast-arity-to-real* x))

  (method Real
    [x: String]
    (*read-string-as-real* x))

  (fun +
    "Generic addition."
    ([x])
    ([x y])
    ([x y & a] (+ (+ x y) & a)))

  (method +
    [x: Number] x)

  (method +
    [x: Number, y: Number]
    (+ & (promote x y)))

  (method +
    [x: Small,  y: Small]
    (*add-small* x y)))