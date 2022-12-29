;; examples of lisp syntax

;;; type declarations
(type cons pair)               ;; type alias
(type list {nul, cons})        ;; union type

(provide [(list, first, rest, empty?, length, filter, map, reduce, take, drop)])

@doc "Constructor."
(fun list  [.. args] args)

@doc "Accessors."
(fun first [(cons x _)] x)
(fun rest  [(cons _ xs)] xs)

@doc "Sequence predicate."
(fun empty?
     ([(nul _)] true)
     ([(cons _)] false))

@doc "Sequence length method."
(fun length
     ([(nul _)] 0)
     ([(cons _ tail)]
      (+ 1 (length tail))))

@doc "Implement for lists."
(fun map
     ([_ (nul)] ())
     ([fn (cons x xs)]
      (cons (fn x) (map fn xs))))

@test even? ()    :is  ()
@test even? (1)   :is  ()
@test even? (1 2) :is (2)
@doc "Implement for lists."
(fun filter
     ([_ (nul)] ())
     ([fn? (cons x xs)]
      (if (fn? xs)
     	  (cons x (filter fn? xs))
	      (filter fn? xs))))

(fun reduce "Implement for list type (with no initial)."
     ([fn (cons x xs)] (reduce fn xs x))
     ([_ (nul _) acc] acc)
     ([fn (cons x xs) acc] (reduce fn (fn acc x) xs)))

fun take "Implement for [int list] type."
    ([_ (nul _)] ())
    ([(int n) (cons x xs)]
     (if (= n 0)
     	 () (cons x (take (dec n) xs))))

(fun take "Implement for [func list]."
     ([_ (nul _)] ())
     ([(func fn?) (cons x xs)]
      (if (fn? x)
      	  (cons x (take fn? xs)) ())))

@doc "Implement for [int list]."
(fun drop
     ([_ (nul _)] ())
     ([(func fn?) (cons x xs :as xxs)]
      (if (fn? x)
      	  (drop fn? xs) xxs)))

;; pseudo-algebraic type example
(type tree
      (empty-tree [])
      (tree-leaf  [key val])
      (tree-node  [left right]))

(provide [(tree, empty?, filter, map, reduce)])

;; effect examples
@doc "Exceptions."
(mac catch
     [(vec handlers) .. body]
     `(with ~(map #(conj 'raise (first %)) handlers)
     	    ~@body))
