 ;; begin stdlib.rl

#| block comment |#

;; syntax table
(var *syntax* {})

(var add-syntax
  (lmb (name transformer)
    (if (has? *syntax* name)
        (add-method (ref *syntax* name) transformer)
        (xef *syntax* name) (function name nul transformer))))

(freeze! '*syntax*)
(freeze! 'add-syntax)

(add-syntax 'mac
  (lmb (name args & body)
    `(add-syntax '~name (lmb ~args ~@body))))

(mac val
  [(name symbol) bind]
  `(do (var ~name ~bind)
       (freeze! ~name)))

(mac fn
  [(args tuple) & body]
  `(lmb ~args ~@body))

(mac fn
  [(overload list) & more]
  `(add-method (fn ~@more) (fn ~@overload)))

(mac fn
  ((name symbol) args & body)
  (var f `(lmb ~args ~@body))
  `(if (bound? '~name)
       (add-method ~name ~f)
       (eval '(val ~name (func ~name nul ~f)))))

;; end stdlib.rl