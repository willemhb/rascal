;; right now this file is just for brainstorming desired syntax and whatnot. The code here
;; doesn't run and isn't intended to.

;; example of desired medium-term `def` semantics and definitions for basic binding macros.

(def-stx stx
  (fn (&form &env name args & body)
    (def fn-form `(fn (&form &env ~@args) ~@body))
     (if (defined? name &env)
       `(def-method ~name ~fn-form)
       `(def-stx ~name ~fn-form))))

(stx fun
  (name args & body)
  (if (list? name) ;; unnamed version, treat exactly like `fn` form.
    `(fn ~name ~args ~@body)
    (if (local-env? &env)
      `(def ~name (fn ~@args ~@body))
      (if (defined? name &env)
        `(def-method ~name (fn ~@args ~@body))
        `(def-multi ~name (fn ~@args ~@body))))))

(stx let
  (vars & body)
  (def names (map fst vars))
  (def vals  (map snd vars))
  `((fn ~names ~@body) ~vals))

;; ideas for pattern matching syntax and limited infix reader macros.
;; who knows how pattern matching will work with generic functions.
(fun* reduce
  ((_ ()) nul)
  ((f (h & t))
    (reduce f t h))
  ((_ () a) a)
  ((f (h & t) a)
    (reduce f t (f a h))))

(fun map
  (f xs)
  (label* (f f, xs xs, ac ())
    ((_ () ac)
      (reverse ac))
    ((f (h & t) ac)
      (loop f t (cons f|h ac)))))

(fun filter
  (p? xs)
  (label* (p? p?, xs xs, ac ())
    ((_ () ac)
      (reverse ac))
    ((p? (h & t) ac) :when (p? h)
      (loop p? t h|ac))
    ((p? (h & t) ac)
      (loop p? t ac))))

(fun pow (x: Num n: Int)
  (label* (x x, n n, a 1)
    ((_ 0 a) a)
    ((x n a) :when (even? n)
      (loop sqr|x (/ n 2) a))
    ((x n a)
      (loop x dec|n (* x a)))))

;; messing around with ideas for self-hosted compiler/interpreter.
