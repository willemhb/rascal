(toplevel
  ;; binding macro definitions.
  (def fun
    {
      :macro => true,
      :doc   => "Idiomatic macro for creating new function bindings."
    }
    (lmb
      ([&form &env a b]     `(lmb ~a ~b))
      ([&form &env n a b]   `(def ~n (lmb ~a ~b)))
      ([&form &env n m a b] `(def ~n ~m (lmb ~a ~b)))))
  (def &required
    "Cache of loaded modules." #{}))