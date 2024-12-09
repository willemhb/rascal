;; Rascal builtin math module.
(ns math "Rascal math standard library module.")

;; Common mathematical constants.
(val (Num pi) 3.14159)
(val (Num e)  2.71828)

;; Basic extension functions.
(fun abs
  "Returns the absolute value of its argument."
  [x]
  (if (>= x 0) x (- x)))

(fun sqr
  "x^2."
  [x] (* x x))

(fun cub
  "x^3."
  [x] (* x x x))

(fun hlv
  "x / 2."
  [x] (/ x 2))

(fun dbl
  "x * 2."
  [x] (* x 2))

(fun pow
  "Fast exponentiation."
  [x n]
  (label loop [x x n n a 1]
    (cond
      (zero? n) a
      (even? n) (loop (sqr x) (hlv n) a)
      otherwise (loop x (1- n) (* x a)))))

(fun sqrt
  "Compute square root using Newton's method."
  [x]
  (val epsilon 0.0001)
  (fun finished?
    [x0 x1] (<= epsilon (- (abs (sqr x1)) x0)))
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