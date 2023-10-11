(module Base
  "Core language syntax."
  (mac guard (handlers .. body)
    (fun expand-handler (handler)
      (stx handler
        (((.. exception-type) .. body)
         ((:exception .. exception-type) .. body))
        (((.. exception-type (.. args)) .. body)
         ((:exception .. exception-type (.. args)) .. body))))
    `(handle ~(map expand-handler handlers) ~@body))

  (mac raise (exception-type)
    "Syntactic sugar for perform."
    `(perform (:exception ~exception-type)))

  (mac raise (exception-type .. args)
    `(perform (:exception ~exception-type) ~args)))

(module Math/core
  "Core mathematical functions & types."
  (use (Base))

  ;; abstract numeric type hierarchy.
  (abstract Number   (Term) "Base type for all numbers.")
  (abstract Real     (Number Float) "All numeric types except `Complex`.")
  (abstract Rational (Real) "Exact ratios and integers.")
  (abstract Integer  (Rational Big Small) "Rational numbers with no fractional component.")

  ;; core mathematical functions.
  ;; basic utilities.
  (fun zero? (x) "Returns `true` if `x` is numerically equivalent to 0.")
  (fun one?  (x) "Returns `true` if `x` is numerically equivalent to 1.")

  ;; min/max.
  (fun max ((Number x) (Number y))
    "Returns the greatest element of the given inputs."
    (if (< x y) y x))

  (fun max ((Number x) & (Number r))
    "Returns the greatest element of the given inputs."
    (if (empty? r)
      x
      (let (((h & t) r))
        (max (max x h) & t))))

  (fun min ((Number x) (Number y))
    "Returns the least element of the given inputs."
    (if (> x y) y x))

  (fun min ((Number x) & (Number r))
    "Returns the least element of the given inputs."
    (if (empty? r)
      x
      (let (((h & t) r))
        (min (min x h) & t))))

  ;; miscellaneous integer utilities.
  (fun even?    (x)   "Returns `true` if `x` is an even integer.")
  (fun odd?     (x)   "Returns `true` if `x` is an odd integer.")
  (fun inc      (x)   "Returns the result of adding 1 to `x`.")
  (fun dec      (x)   "Returns the result of subtracting 1 from `x`.")
  (fun divides? (a b) "Returns `true` if `a` is a divisor of `b`.")
  (fun gcd      (a b) "Returns the greatest integer `c` such that `c` divides `a` and `c` divides `b`.")
  (fun lcm      (a b) "Returns the least integer `c` such that `a` divides `c` and `b` divides `c`.")

  ;; conversion.
  (fun promote (x y) "Returns a 2-tuple `[x1 y1]` where `x1` and `y1` are the result of converting `x` and `y` to their most conservative common type.")

  ;; exponentiation operations.
  (fun sqr (x) "Raises `x` to the power of 2.")
  (fun cub (x) "Raises `x` to the power of 3.")
  (fun pow (x n) "Raises `x` to the power of `n`.")

  ;; Small implementations.
  (fun zero? ((Small x)) (= x 0))
  (fun one?  ((Small x)) (= x 1))
  (fun even? ((Small x)) (zero? (rem x 2)))
  (fun odd?  ((Small x)) (one? (rem x 2)))
  (fun inc   ((Small x)) (+ x 1))
  (fun dec   ((Small x)) (- x 1))

  (fun divides? ((Small a) (Small b))
    (zero? (rem b a)))

  (fun gcd ((Small a) (Small b))
    "Euclid's algorithm (obviously)."
    (cond
      ((< a b)        (gcd b a))
      ((divides? b a) b)
      (otherwise      (gcd b (rem a b)))))

  (fun lcm ((Small a) (Small b))
    (/ (* a b) (gcd a b)))

  (fun sqr   ((Small x)) (* x x))
  (fun cub   ((Small x)) (* x x x))

  (fun pow ((Small x) (Integer n))
    "Fast power implementation."
    (loop ((x x) (n n) (acc 1))
      (cond
        ((zero? n) acc)
        ((even? n) (loop (sqr x) (/ n 2) acc))
        (otherwise (loop x (dec n) (* x acc)))))))

(module Math/Ratio
  "Compound numeric type representing an exact fraction."
  (use (Base Math/core))

  (struct (Rational Ratio)
    "Exact fraction."
    ((Integer numer)
     (Integer denom))))

(module Math/Complex
  "Compound numeric type representing a complex number."
  (use (Base Math/core))

  (struct (Number Complex)
    "A complex number with real and imaginary components."
    ((Float real)
     (Float imag))))

(module Enum
  "Core sequence and iteration utilities."
  (use (Base))

  ;; Transducing functions (declarations).
  (fun walk (f xs) "Apply `f` to the elements of `xs`. Return a collection of the same type as `xs`.")
  (fun keep (p? xs) "Return a collection of the same type as `xs`, whose elements are all those elements `x` in `xs` such that `(p? x)` evalutes to `true`.")
  (fun take (s xs) "Return a collection with the first `s` elements of `xs`, where `s` is either a number or a predicate.")
  (fun drop (s xs) "Return a collection without the first `s` elements of `xs`, where `s` is either a number or a predicate.")
  (fun fold (op xs) "Apply binary function `op` cumulatively to the elements of `xs` from left to right.")
  (fun fold (op xs ini) "Apply binary function `op` cumulatively to the elements of `xs` from left to right. Use `ini` as the first argument.")

  ;; List methods.
  (fun walk ((Function f) (List xs))
    (loop ((xs xs) (acc ()))
      (case (xs)
        ((())      (reverse acc))
        (((h & t)) (loop t (cons (f h) acc))))))

  (fun keep ((Function p?) (List xs))
    (loop ((xs xs) (acc ()))
      (case (xs)
        ((())      (reverse acc))
        (((h & t)) (if (~p? h)
                     (loop t acc)
                     (loop t (cons h acc)))))))

  (fun take ((Function p?) (List xs))
    (loop ((xs xs) (acc ()))
      (case (xs)
        ((())      (reverse acc))
        (((h & t)) (if (~p? h)
                     (reverse acc)
                     (loop t (cons h acc)))))))

  (fun take ((Integer i) (List xs))
    (loop ((i i) (xs xs) (acc ()))
      (case (i xs)
        ((_ ())      ())
        ((0 _)       ())
        ((i (h & t)) (loop (dec i) t (cons h acc))))))

  (fun drop ((Function p?) (List xs))
    (case (xs)
      ((())      ())
      (((h & t)) (if (~p? h)
                   xs
                   (drop p? t)))))

  (fun drop ((Integer i) (List xs)) 
    (case (i xs)
      ((_ ())      ())
      ((0 _)       xs)
      ((i (_ & t)) (drop (dec i) t))))

  (fun fold ((Function op) (List xs))
    (case (xs)
      ((())      (raise :value-error "`()` passed to `fold` with no initializer."))
      (((h & t)) (fold op t h))))

  (fun fold ((Function op) (List xs) acc)
    (case (xs)
      ((())      acc)
      (((h & t)) (fold op t (op acc h))))))