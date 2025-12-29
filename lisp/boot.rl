;; rascal standard library (such as it is)

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

(fun map (f xs)
  ;; you know this one.
  (if (=? xs ())
    ()
    (cons (f (head xs))
          (map f (tail xs)))))

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

;; backquote implementation.
;; Helper: check if x is (unquote expr)
(fun unquote? (x)
  (and (list? x)
       (not (empty? x))
       (=? (head x) 'unquote)))

;; Helper: check if x is (unquote-splice expr)
(fun unquote-splice? (x)
  (and (list? x)
       (not (empty? x))
       (=? (head x) 'unquote-splice)))

;; Helper: check if a list contains any unquote-splice at top level
(fun has-splice? (xs)
  (if (empty? xs)
    false
    (or (unquote-splice? (head xs))
        (has-splice? (tail xs)))))

;; Transform a backquoted expression into code that builds the result
(fun bq-transform (x)
  (if
    (not (list? x))
      ;; Atom: quote it
      (list 'quote x)
    (empty? x)
      ;; Empty list: quote it
      (list 'quote x)
    (unquote? x)
      ;; (unquote expr): return the expression
      (head (tail x))
    (unquote-splice? x)
      ;; unquote-splice at top level is an error
      (raise [:syntax-error "unquote-splice (~@) not valid here"])
      ;; Regular list: transform elements
    otherwise
      (bq-transform-list x)))

;; Transform a list, handling splices
(fun bq-transform-list (xs)
  (if (has-splice? xs)
    ;; Has splices: use concat
    (list 'concat (cons 'list (bq-build-segments xs)))
    ;; No splices: use list
    (cons 'list (map bq-transform xs))))

;; Build segments for concat: each segment is either a single-element list
;; or a spliced list
(fun bq-build-segments (xs)
  (if
    (empty? xs)
      ()
    (unquote-splice? (head xs))
      ;; Splice: the expression itself (should be a list)
      (cons (head (tail (head xs)))
            (bq-build-segments (tail xs)))
      ;; Regular element: wrap in list
    otherwise
      (cons (list 'list (bq-transform (head xs)))
            (bq-build-segments (tail xs)))))

;; The backquote macro
(stx backquote
  (expr)
  (bq-transform expr))

;; other macros that benefit from a working backquote
(stx when
  (test & body)
  `(if ~test
     (do ~@body)))

(stx unless
  (test & body)
    `(if (not ~test)
      (do ~@body)))

;; block and looping forms
(stx let
  (binds & body)
  (fun xform-bind (bind) `(def ~@bind))
  (def def-forms (map xform-bind binds))
  `((fn () ~@def-forms ~@body)))

(stx label
  ;; standard looping syntax.
  ;; defines a recursive function inside
  ;; a thunk, invokes the body inside the thunk,
  ;; and immediately invokes the thunk.
  ;; this really calls for actually implementing tail recursion.
  (name inits & body)
  (def formals (map fst inits))
  (def values  (map snd inits))
  (def defn-form
    `(def ~name (fn ~formals ~@body)))
  `((fn () ~defn-form (~name ~@inits))))
