;; Rascal builtin math module.
(ns math "Rascal math standard library module.")

;; Common mathematical constants.
(val (Num pi) 3.14159)
(val (Num e)  2.71828)

;; Basic extension functions.
(fun neg?
  "Predicate returns true if `x` is a negative number."
  (Num x) (< x 0))

(fun abs
  "Returns the absolute value of its argument."
  (Num x) (if (neg? x) x (- x)))

(fun sqr
  "x^2."
  (Num x) (* x x))

(fun cub
  "x^3."
  (Num x) (* x x x))

(fun hlv
  "x / 2."
  (Num x) (/ x 2))

(fun dbl
  "x * 2."
  (Num x) (* x 2))

(fun pow
  "Fast exponentiation."
  [x n]
  (label loop [x n 1]
    [x 1 a]
      a
    [x n a] @when (even? n)
      (loop (dbl x) (hlv n) a)
    [x n a]
      (loop x (1- n) (* a x))))

(fun len
  "Implementation for lists."
  ()
    0
  (List _ t)
    (1+ (len t)))

(fun nth
  "Implementation for lists."
  [() n]
    (throw :value-error "Index ${n} out of range")
  [(List h _) 1]
    h
  [(List _ t) n]
   (nth t (1- n)))

(fun sqrt
  "Compute square root using Newton's method."
  [x]
  (val epsilon 0.0001)
  (fun finished?
    [x0 x1] (<= epsilon (- x1.sqr.abs x0)))
  (label loop [x x g x]
    (if (finished? x g)
      g
      (loop x (- g (/ (sqr g) (dbl g)))))))

(fun sum
  "Get the sum of the given collection."
  [xs]
  (fold + xs))

(fun hypot
  "Euclidean norm (supports generalization to arbitrarily many dimensions)."
  [x y & r]
  (let
    [x2  (sqr x)
     y2  (sqr y)
     r2  (map sqr r)
     all (+ x2 y2 (sum r2))]
    (sqrt all)))