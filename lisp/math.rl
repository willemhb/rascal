;; math standard library.

(use (base))

;; compound numeric types.
(struct (Rational Ratio)
  "Represents an exact fraction."
  ((Integer numer)
   (Integer denom)))

(struct (Number Complex)
  "Represents a compex number with real and imaginary components."
  ((Float real)
   (Float imag)))

;; common math utilities.
(fun zero?
  "Exactly what it sounds like."
  ((Number x))
  (= 0 x))

(fun one?
  "Exactly what it sounds like."
  ((Number x))
  (= 1 x))

(fun even?
  "Exactly what it sounds like."
  ((Integer x))
  (zero? (rem x 2)))

(fun odd?
  "Exactly what it sounds like."
  ((Integer x))
  (one? (rem x 2)))

(fun hlf
  "Exactly what it sounds like."
  ((Integer x))
  (/ x 2))

(fun inc
  "Add `1` to `n`."
  ((Integer n))
  (+ n 1))

(fun dec
  "Substract `1` from `n`."
  ((Integer n))
  (- n 1))

(fun sqr
  "Returns the squre of `x`."
  ((Number x))
  (* x x))

(fun cub
  "Returns the cube of `x`."
  ((Number x))
  (* x x x))

(fun divides?
  "`true` if `a` is a dvisior of `b`."
  ((Integer a) (Integer b))
  (zero? (rem b a)))

;; more complex functions.
(fun pow
  "Fast power (tail recursive)."
  ((Number x) (Integer n))
  (label loop ((x x) (n n) (acc 1))
    (cond ((zero? n) acc)
          ((even? n) (loop (sqr x) (hlf n) acc))
          (otherwise (loop x (dec n) (* x acc))))))

(fun sqrt
  "Heron's formula."
  ((Real x))
  (val delta 0.000001)
  (label loop ((guess x))
    (if (> delta (abs (- x (sqr guess))))
        guess
        (loop (hlf (+ guess (/ x guess)))))))

(fun fac
  "Factorial."
  ((Integer n))
  (label loop ((n n) (acc 1))
    (if (zero? acc)
        acc
        (loop (dec n) (* n acc)))))

(fun fib
  "Fibonacci sequence. Remains to be seen whether this will ever be used outside of examples."
  ((Integer n))
  (label loop ((a 0) (b 1) (n n))
    (if (zero? n)
        b
        (loop b (+ a b) (dec n)))))

(fun gcd
  "Euclid's algorithm."
  ((Integer a) (Integer b))
  (cond ((< b a)        (gcd b a))
        ((divides? a b) a)
        (otherwise      (gcd a (rem b a)))))

(fun lcm
  "Least common multiple."
  ((Integer a) (Integer b))
  (/ (* a b) (gcd a b)))

(fun sum
  "Sum of the given collection."
  (xs)
  (fold + xs 0))

(fun prod
  "Product of the numbers in the given collection."
  (xs)
  (fold * xs 1))

(fun norm
  "Get the euclidean norm (hypotenuse) of a collection."
  (xs)
  (sqrt (sum (map sqr xs))))

(fun norm
  "If more than one argument is given, treat as though a single collection was given."
  (& args)
  (norm args))

(fun avg
  "Get the mean of the elements of `xs`."
  (xs)
  (/ (sum xs) (cnt xs)))

(fun avg
  "Get the mean of the `args`."
  (& args)
  (avg args))

(fun max
  "Just what it sounds like."
  ((Number x) (Number y))
  (if (< x y) y x))

(fun max
  ((Number x) (Number y) & more)
  (max (max x y) & more))

(fun max
  "Get the greatest element of `xs`."
  (xs)
  (fold max xs))

(fun max
  "Get the greatest element of `xs` according to key function `fn`."
  ((Function f) xs)
  (fold (fun (x y) (if (< (f x) (f y)) y x)) xs))

(fun min
  "Just what it sounds like."
  ((Number x) (Number y))
  (if (> x y) y x))

(fun min
  ((Number x) (Number y) & more)
  (min (min x y) & more))

(fun min
  (xs)
  (fold min xs))

(fun min
  ((Function f) xs)
  (fold (fun (x y) (if (> (f x) (f y)) y x)) xs))