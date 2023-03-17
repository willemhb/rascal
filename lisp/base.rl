(require 'prelude)

#| type predicates & other queries |#
(fn unit?    (x) (isa? x unit))
(fn any?     (x) (isa? x any))
(fn none?    (x) (isa? x none))
(fn boolean? (x) (isa? x boolean))
(fn glyph?   (x) (isa? x glyph))
(fn native?  (x) (isa? x native))
(fn small?   (x) (isa? x small))
(fn fixnum?  (x) (isa? x fixnum))
(fn big?     (x) (isa? x big))
(fn ratio?   (x) (isa? x ratio))
(fn complex? (x) (isa? x complex))
(fn stream?  (x) (isa? x stream))
(fn real?    (x) (isa? x real))
(fn symbol?  (x) (isa? x symbol))
(fn list?    (x) (isa? x list))
(fn tuple?   (x) (isa? x tuple))
(fn vector?  (x) (isa? x vector))
(fn dict?    (x) (isa? x dict))
(fn set?     (x) (isa? x set))

(provide
  (unit? any? none? small? fixnum? big? ratio? complex? boolean? glyph? stream?
   native? real? symbol? list? tuple? vector? dict? set?))

#| numeric utilities |#

(type number   (real fixnum big small ratio complex))
(type integer  (fixnum big small))
(type rational (fixnum big small ratio))

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

(provide 'base)