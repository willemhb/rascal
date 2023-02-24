;;; begin math.rl
;;; rascal math standard library

(require "base.rl")

;;; core mathematical functions & helpers.

(type number #{ real fixnum int ratio bigint complex })

(fn zero?
  ((number x)) (= x 0))

(fn one?
  ((number x)) (= x 1))

(fn /=
  ((number x) (number y))
  (not (= x y)))

(fn sqr
  ((number x))
  (* x x))

(fn cub
  ((number x))
  (* x x x))

(fn dec
  ((int n)) (- n 1))

(fn inc
  ((int n)) (+ n 1))

(fn halve
  ((number x))
  (/ x 2))

(fn pow
  ((x number) (n int))
  (cond
    (zero? n)   1
    (odd? n)    (* x (pow x (dec n)))
    :otherwise  (pow (sqr x) (halve n))))

;;; end math.rl