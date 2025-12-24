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

(fun map
  (f xs)
  (label ((f f) (xs xs) (ac ac))
    (if (empty? xs)
      (reverse ac)
      (let (((h & t) xs))
        (loop f t (cons (f h) ac))))))