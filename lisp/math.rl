(use ("base.rl"))

;; Mathematics standard library & numeric type hierarchy defined here.

;; important numeric limits.
(val *small-max*  2,147,483,647)
(val *small-min* -2,147,483,648)

;; Numeric type system.
(union Integer
  @doc "Numeric types with no fractional component."
  (Small Big))

(record Ratio
  @doc "Exact fractional types."
  ((Integer numer)
   (Integer denom)))


(union Rational
  @doc "Real types with exact representation."
  (Integer Ratio)) ;; unions are automatically flattened, so this is identical to (Big Small Ratio)

(union Real
  @doc "All numeric types except `Complex`."
  (Rational Float))

(record Complex
  @doc "A complex number with real and imaginary parts."
  ((Float real)
   (Float imag)))

;; basic utilities.
(fun zero?
  ()
  )