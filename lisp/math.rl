;; math standard library

(use (base))

(fun zero?
  "Check whether `x` is numerically equivalent to `0`."
  ((Number x))
  (= x 0))

(fun one?
  "Check whether `x` is numerically equivalent to `1`."
  ((Number x))
  (= x 1))

(fun even?
  "Check whether `n` is an even integer."
  ((Integer n))
  (zero? (rem n 2)))

(fun odd?
  "Check whether `n` is an odd integer."
  ((Integer n))
  (one? (rem n 2)))

(fun sqr
  "Shorthand for `x^2`."
  ((Number x))
  (* x x))

(fun cub
  "Shorthand for `x^3`."
  ((Number x))
  (* x x x))

(fun hlv
  "Shorthand for `x//2`."
  ((Number x))
  (// x 2))

(fun hlv*
  "Shorthand for `x/2`."
  (/ x 2))

(fun abs
  "Returns the absolute value of its input."
  ((Number x))
  (if (< x 0) (- x) x))

(fun pow
  "Fast power (tail recursive)."
  ((Number x) (Integer n))
  (label loop ((x x)
               (n n)
               (a 1))
    (cond ((zero? n) a)
          ((even? n) (loop (sqr x) (hlv n) a))
          (otherwise (loop x (dec n) (* x a))))))

(fun deriv
  "With one argument, return a function that calculates a numerical derivative of `f`."
  ((Function f))
  (fun (x) (/ (- (f (+ x 0.00001)) (f x)) 0.00001)))

(fun deriv
  "With two arguments, evaluate the derivative of `f` at `x`."
  ((Function f) (Number x))
  (/ (- (f (+ x 0.00001)) (f x)) 0.00001))

(fun sqrt
  "Newton's method."
  ((Number x))
  (label loop ((a x))
    (if (< (abs (- (sqr a) x)) 0.00001)
        a
        (loop (- a (hlv (+ a (deriv sqr a))))))))

;; simple collection utilities.
(fun sum
  "Get the sum of the inputs."
  ((List args))
  (fold + args 0))

(fun prod
  "Get the product of the inputs."
  ((List args))
  (fold * args 1))

;; trigonometry.
(fun norm
  "Hypotenuse."
  (args)
  (sqrt|sum (map sqr args)))

