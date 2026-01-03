;; Code in this file is for playing around with ideas. It's not expected to run.
;; Loading pretty much guaranteed to crash the interpreter.

;; function literal syntax: as in clojure but uses'λ' as the dispatch characgter instead of '#'.
(map λ(^ 2 &) (range 0 9)) ; => (1 2 4 8 16 32 64 128 256 512)

;; splatting syntax: turns a regular function call '(f & args)' into '(apply f args)'.
(+ & '(1 2 3)) ;; => 6

;; destructuring syntax: borrowed from clojure with alteration. in most binding forms that expect a
;; symbol, a data structure can be provided instead, with the corresponding semantics depending on the
;; form provided.
(def decimals-list  '(0 1 2 3 4 5 6 7 8 9))
(def decimals-names '(:zero :one :two :three :four :five :six :seven :eight :nine))
(def decimals-tuple  [0 1 2 3 4 5 6 7 8 9])
(def decimals-map    (map & (zip λ[&1 &2] decimal-digits-list decimal-digits-names)))

;; binds 'zero' and 'one' to the first two elements of the value expression, and the rest of the sequence to 'big-digits'.
(def (zero one & big-digits) decimal-digits-names)

(def (w i l & ljukes) "will jukes") ; → w = \w, i = \i, l = \l, ljukes = "l jukes"

;; Raises an exception if the value expression does not have at least two elements.
(def [zero one &] decimal-gits-tuple) ; → zero = 0, one = 1 (other tuple elements ignored)

;; Raises an exception if the value expression does not have exactly two elements.
(def [zero one] [0 1]) ; → zero = 0, one = 1

;; because lists are secretly two-tuples, they can be destructured with tuples.
(def [car cdr] '(x y z)) ; → car = x, cdr = (y z)

(def {:one one, :two two} {:one 1, :two 2}) ; → one = 1, two = 2

;; infix application syntax: 'a.x' reads as '(a x)' (useful for making common function calls more readable).
(fun map (f: Fun xs: List)
 (if (=? xs ())
     ()
     (cons f.head.x (map f tail.xs))))

;; infix composition syntax: '(a|b c)' → '(a (b c))', a|b → (fn (x) (a (b x)))
(head|tail '(a b c)) ; => b
(map head|tail '((a b) (c d) (e f) (g h) (i j) (k l))) ; => (b d f h j l)

;; literal application as get: non-symbolic atoms as the first element of a lsit is treated as a reference
;; eg (0 [1 2 3 4]) becomes (ref 0 [1 2 3 4]), (:x {:x 1, :y 2, :z 3}) becomes (ref :x {:x 1, :y 2, :z 3})
(def decimal-digits [0 1 2 3 4 5 6 7 8 9])
(def smallest-digit (0 decimal-digits))

;; collection application as get: basically the inverse of the above
(def largest-digit (decimal-digits 9))

;; can be combined with infix application syntax
(def middle-digits [decimal-digits.4, decimal-digits.5])

;; predicate let forms: if the first expression in a `let` form is the name of one
;; of the forms `if`, `and`, `or`, `cond`, or `case`, the let form has different semantics corresponding
;; roughly to those predicate forms:
;;
;;  1. (let if   (test-var test-expr) then & otherwise)
;;  2. (let and  ((test-var test-expr) & binds) & body)
;;  3. (let or   ((test-var test-expr) & binds) & body)
;;  4. (let cond ((test-var test-expr) & then) & binds)
;;  5. (let case test-expr ((case-var case-expr) & then) & binds)

(let if
 (test tail.args)
 (print "test passed.")
 (print "test failed."))

(let and
 ((hd   head.xs)
  (ht   tail.hd)
  (else (print "bindings failed.")))
 (print "head = #{hd}\ntail = #{tl}.\n"))

(let cond
 (((h & t) cons?.xs)  ...)
 (([x y z] tuple?.xs) ...)
 (({:drink drink, :fries? fries} map?.xs) ...))

;; keyword and options arguments: 
(fun open
 (name: Str && opts {
   :text text?,
   :binary binary?,
   :read read?,
   :write write?,
   :append append? })
 ...)
