;; examples of lisp syntax

;;; type declarations
(type time real)                            ;; aliased type
(type number {real, small, complex})        ;; union type
(type person [name, age, gender, student?]) ;; record type
(type list {nul, cons})                     ;; pseudo-algebraic type

(provide [list, length, filter, map, reduce])

(fun first [(cons [x,  _])] x)
(fun rest  [(cons [_, xs])] xs)

(fun length [(nul)] 0)
(fun length [(cons _, xs)] (inc xs))

(fun map "Implement for list type."
     [fn, (nul xs)] ())

(fun map "Implement for list type."
     [fn, (cons [x, xs])]
     (cons (fn x)
           (map fn xs)))



(fun filter "Implement for list type."
     [fn?, (xs list)]
     (cond (nul? xs) ()
           (fn? (first xs))
            (cons (first xs) (filter fn? (rest xs)))
           otherwise (filter fn? (rest xs))))

(fun reduce "Implement for list type (with no initial)."
     [fn, (xs cons)]
     (reduce fn (rest xs) (first xs)))

(fun reduce "Implement for list type (initial supplied)."
     [fn, (xs list), acc]
     (if (nul? xs)
          acc (reduce fn (rest xs) ())))

(mac catch "Syntactic sugar for handling raise effect."
     [irritant, handlers, & body]
     (fun transform [irritant, handlers] ...)
     `(with ~(transform irritant handlers) ~@body))
