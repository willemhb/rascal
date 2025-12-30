;; rascal standard library (such as it is)

;; miscellaneous globals
(def lib-c (ffi-open "libc.so.6"))

;; basic syntax
(def-stx stx
  (fn (&form &env name args & body)
    (def fn-form (cons 'fn (cons '&form '&env args) body))
     (if (defined? name &env)
       (list 'def-method name fn-form)
       (list 'def-stx name fn-form))))

(stx fun
  (name args & body)
  (if (isa? name List) ;; unnamed variant, treat exactly like `fn` form.
    (cons 'fn name args body)
    (if (local-env? &env)
      (list 'def name (cons 'fn args body))
      (if (defined? name &env)
        (list 'def-method name (cons 'fn args body))
        (list 'def-multi name (cons 'fn args body))))))

(stx handle
  ;; slightly prettier try/catch form
  (handler & body)
  (list (cons 'fn () body)
        (cons 'fn handler)))

;; miscellaneous utilities
(fun not (x)
  (if x false true))

;; type predicates
(fun list? (x)
  (isa? x List))

(fun tuple? (x)
  (isa? x Tuple))

(fun sym? (x)
  (isa? x Sym))

(fun str? (x)
  (isa? x Str))

(fun func? (x)
  (isa? x Fun))

(fun num? (x)
  (isa? x Num))

(fun atom? (x)
  (if (list? x)
    (=? x ())
    true))

;; list utilities
(fun empty? (xs)
  (if (isa? xs List)
    (=? xs ())
    (raise [:eval-error "not a list."])))

(fun append (xs ys)
 (if (empty? xs)
  ys
  (cons (head xs) (append (tail xs) ys))))

(fun concat (xss)
  (if (empty? xss)
    ()
    (append (head xss) (concat (tail xss)))))

(fun fst (xs)
  (head xs))

(fun snd (xs)
  (head (tail xs)))

(fun map-f (f xs)
  ;; you know this one.
  (if (=? xs ())
    ()
    (cons (f (head xs))
          (map-f f (tail xs)))))

(fun filter (p? xs)
  (if
    (empty? xs)
      ()
    (p? (head xs))
      (cons (head xs) (filter p? (tail xs)))
    otherwise
      (filter p? (tail xs))))

;; number utilities and extended arity arithmetic builtins
(fun zero? (x)
  (=? x 0))

(fun one? (x)
  (=? x 1))

(fun + (x) x)
(fun + (x y & more)
  (apply + (+ x y) more))

(fun - (x) (* -1 x))
(fun - (x y & more)
  (apply - (- x y) more))

(fun * (x y & more)
  (if (or (= x 0)
          (= y 0))
      0
      (apply * (* x y) more)))

(fun / (x) (/ 1 x))

(fun / (x y & more)
  (if (zero? y)
    (raise [:eval-error "division by zero."])
    (apply / (/ x y) more)))

(fun even? (n)
  (zero? (rem n 2)))

(fun odd? (n)
  (one? (rem n 2)))

(fun inc (x)
  (+ x 1))

(fun dec (x)
  (- x 1))

(fun sqr (x)
  (* x x))

(fun cub (x)
  (* x x x))

;; IO helpers
(fun print (x)
  (print &outs x))

(fun print (io & xs)
  (if
    (empty? xs)
      nul
    otherwise
      (do (print io (head xs))
          (apply print io xs))))

(fun newline ()
  (newline &outs))

(fun println (io x)
  (print io x)
  (newline io))

(fun println (x)
  (println &outs x))
