(use (base))

;; Mathematics standard library & numeric type hierarchy defined here.

;; important numeric limits.
(val *small-max*  2,147,483,647)
(val *small-min* -2,147,483,648)

;; Numeric type system.
(abstract (Term Number)
  @doc "Base type for all numbers.")

(abstract (Number Real)
  @doc "Base type for real numbers.")

(abstract (Real Rational)
  @doc "Base type for rational numbers.")

(abstract (Rational Integer)
  @doc "Base type for integers.")

(struct (Rational Ratio)
  @doc "Compound numeric type for exact fractions."
  ((Integer numer)
   (Integer denom)))

(struct (Number Complex)
  @doc "Compound numeric type for complex numbers with real and imaginary parts."
  ((Float real)
   (Float imag)))

