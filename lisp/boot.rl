;; rascal standard library (such as it is)

;; type predicates
(def isa?   (fn (t x) (=? t (type x))))
(def list?  (fn (x) (isa? :list x)))
(def num?   (fn (x) (isa? :num x)))
(def sym?   (fn (x) (isa? :sym x)))
(def str?   (fn (x) (isa? :str x)))
(def glyph? (fn (x) (isa? :glyph x)))

;; miscellaneous
(def id
  ;; identity
  (fn (x) x))

(def not
  ;; negation
  (fn (x)
    (if x false true)))

(def atom?
  (fn (x)
    (not (list? x))))

(def inc
  (fn (n)
    (+ n 1)))

(def dec
  (fn (n)
    (- n 1)))

(def sqr
  (fn (x)
    (* x x)))

;; list utilities
(def empty?
  ;; empty list predicate
  (fn (xs)
    (if (list? xs)
      (=? xs ())
      false)))

(def map
  ;; return a list where `f` has been applied to the elements of `xs`.
  (fn (f xs)
    (if (empty? xs)
      ()
      (cons (f (head xs))
            (map f (tail xs))))))

(def filter
  ;; return a list of all the elements from `xs` that satisfy `p?`.
  (fn (p? xs)
    (if (empty? xs)
      ()
      (if (p? (head xs))
        (cons (head xs)
              (filter p? (tail xs)))
        (filter p? (tail xs))))))
