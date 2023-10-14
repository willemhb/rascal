(use (base))

;; Mathematics standard library & numeric type hierarchy defined here.

;; important numeric limits.
(val *small-max*  2,147,483,647)
(val *small-min* -2,147,483,648)

;; Numeric type system.
(union Integer
  @doc "Numeric types with no fractional component."
  (Term Small Big))

(struct Ratio
  @doc "Exact fractional types."
  ((Integer numer)
   (Integer denom)))

(union Rational
  @doc "Real types with exact representation."
  (Integer Ratio)) ;; unions are automatically flattened, so this is identical to (Big Small Ratio)

(union Real
  @doc "All numeric types except `Complex`."
  (Rational Float))

(struct Complex
  @doc "A complex number with real and imaginary parts."
  ((Float real)
   (Float imag)))

;; basic utilities.
;; method signatures.
(fun zero?    (x)   @doc "Returns `true` if `x` is numerically equal to `0`.")
(fun one?     (x)   @doc "Returns `true` if `x` is numerically equal to `1`.")
(fun even?    (x)   @doc "Returns `true` if `x` is an even integer.")
(fun odd?     (x)   @doc "Returns `true` if `x` is an odd integer.")
(fun divides? (a b) @doc "Returns `true` if `a` is a divisor of `b`.")
(fun sqr      (x)   @doc "Raises `x` to the power of `2`.")
(fun cub      (x)   @doc "Raises `x` to the power of `3`.")
(fun pow      (x n) @doc "Raises `x` to the power of `n`.")

;; method implementations.
(fun zero?
  ((Small x))
  (= x 0))

(fun zero?
  ((Big x))
  (= x 0L))

(fun zero?
  ((Float x))
  (= x 0.0))

(fun zero?
  ((Ratio x))
  (= x 0/1))

(fun zero?
  ((Complex x))
  (= x 0+0i))

(fun one?
  ((Small x))
  (= x 1))

(fun one?
  ((Big x))
  (= x 1L))

