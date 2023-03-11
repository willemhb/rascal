(require 'prelude)
(require 'base)

#| numeric utilities |#

(type number   (real fixnum big small ratio complex))
(type integer  (fixnum big small))
(type rational (integer ratio))

(fn zero?
  "Return `true` if `x` is numerically equal to `0`."
  (x)
  (= x 0))

(fn one?
  "Return `true` if `x` is numerically equal to `1`."
  (x)
  (= x 1))

(fn inc
  "Return `n + 1`."
  ((integer n))
  (+ n 1))

(fn dec
  "Return `n - 1`."
  ((integer n))
  (- n 1))

(fn pow
  "Fast exponentiation."
  ((number x) (integer n))
  (labl loop
    ((x x) (n n) (acc 1))
    (cond
      (zero? n)  acc
      (even? n)  (loop (* x x) (/ n 2) acc)
      :otherwise (loop x (dec n) (* x acc)))))

(provide (zero? one? inc dec))
(provide 'main)