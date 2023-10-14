(use (base))

;; Mathematics standard library & numeric type hierarchy defined here.

;; important numeric limits.
(val *small-max*  2,147,483,647)
(val *small-min* -2,147,483,648)

;; Numeric type system.
(class (Term Number)
  @doc "Base class for numeric types."
  ((fun zero?
     @doc "Returns `true` if `x` is numerically equal to `0`."
     ((Number x)))
   (fun one?
     @doc "Returns `true` if `x` is numerically equal to `1`."
     ((Number x)))
   (fun promote
     @doc "Converts `x` and `y` to their most conservative common numeric type."
     ((Number x) (Number y)))))

(class (Number Real)
  @doc "Base class for real numeric types."
  ())

(class (Real Rational)
  @doc "Base class for rational numeric types."
  ())

(class (Rational Integer)
  @doc "Base class for integral numeric types."
  ())

(struct (Rational Ratio)
  @doc "Compound numeric type representing an exact fraction."
  ((Integer numer)
   (Integer denom)))

(struct (Number Complex)
  @doc "Compound numeric type representing a complex number with real and imaginary components."
  ((Float real)
   (Float imag)))
