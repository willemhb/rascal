;; rascal standard library (such as it is)

;; basic syntax
(def-stx stx
  (fn (&form &env name args & body)
    (def fn-form (cons* 'fn (cons* '&form '&env args) body))
     (if (defined? name &env)
       (list 'def-method name fn-form)
       (list 'def-stx name fn-form))))

(stx fun
  (name args & body)
  (if (isa? name List) ;; unnamed variant, treat exactly like `fn` form.
    (cons* 'fn name args body)
    (if (local-env? &env)
      (list 'def name (cons* 'fn args body))
      (if (defined? name &env)
        (list 'def-method name (cons* 'fn args body))
        (list 'def-multi name (cons* 'fn args body))))))

;; list utilities
(fun empty? (xs)
  (if (isa? xs List)
    (=? xs ())
    (error "not a list")))

(fun fst (xs)
  (head xs))

(fun snd (xs)
  (head (tail xs)))

(fun map (f xs)
  (if (=? xs ())
    ()
    (cons (f (head xs))
          (map f (tail xs)))))

(fun filter (p? xs)
  (if (=? xs ())
    ()
    (if (p? xs)
      (cons (head xs)
            (filter p? (tail xs)))
      (filter p? (tail xs)))))

(stx let
  (vars & body)
  (def names (map fst vars))
  (def binds (map snd vars))
  (cons* (cons* 'fn names body) binds))

;; miscellaneous utilities
(fun not (x)
  (if x false true))

;; type predicates
(fun list? (x)
  (isa? x List))

(fun sym? (x)
  (isa? x Sym))

(fun str? (x)
  (isa? x Str))

(fun fun? (x)
  (isa? x Fun))

(fun num? (x)
  (isa? x Num))

(fun atom? (x)
  (if (list? x)
    (=? x ())
    true))

;; number utilities
(fun zero? (x)
  (=? x 0))

(fun one? (x)
  (=? x 1))

(fun inc (x)
  (+ x 1))

(fun dec (x)
  (- x 1))

(fun sqr (x)
  (* x x))
