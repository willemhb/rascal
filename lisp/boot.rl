;; rascal standard library (such as it is)

;; basic macros
(def-stx
  stx
  (fun (&form &env name args & body)
    (list 'def-stx
          name
          (cons* 'fun
                (cons* '&form '&env args)
                body))))

(stx unless
  ;; classic simple macro, good for testing that it works.
  (test & exprs)
  (list 'if
        test
        (cons 'do exprs)))

;; type predicates
(fun isa? (t x)
  (=? t (typeof x)))

(fun list? (x)
  (isa? List x))

(fun num? (x)
  (isa? Num x))

(fun sym? (x)
  (isa? Sym x))

(fun str? (x)
  (isa? Str x))

(fun glyph? (x)
  (isa? Glyph x))

;; miscellaneous
(fun id (x) x) ;; identity function

(fun not (x)
  ;; logical negation
  (if x false true))

(fun atom? (x)
  (not (list? x)))

(fun inc (n)
  (+ n 1))

(fun dec (n)
  (- n 1))

(fun sqr (x)
  (* x x))

;; list utilities
(fun list-len=? (xs n)
  (= n (list-len xs)))

(fun empty? (xs)
  ;; empty list predicate
  (if (list? xs)
    (=? xs ())
    false))

(fun map (f xs)
  ;; return a list where `f` has been applied to the elements of `xs`.
  (if (empty? xs)
    ()
    (cons (f (head xs))
          (map f (tail xs)))))

(fun filter (p? xs)
  ;; return a list of all the elements from `xs` that satisfy `p?`.
  (if (empty? xs)
    ()
    (if (p? (head xs))
      (cons (head xs)
            (filter p? (tail xs)))
      (filter p? (tail xs)))))

(fun reduce (f xs acc)
  (if (empty? xs)
    acc
    (reduce f (tail xs) (f acc (head xs)))))

(fun reduce (f xs)
  ;; with two arguments automatically pass first element of xs as accumulator.
  (reduce f (tail xs) (head xs)))

;; testing multimethod + variadic functions
(fun + (x & more)
  (reduce + more x))