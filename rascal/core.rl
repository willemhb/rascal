;;; begin core.rl
;;; rascal basics module.

;;; globals
;;; example of defining a custom type and implementing methods
(type list {nul, cons})

(provide [list, head, tail, length, map, filter, reduce])

(fun head "Get the first element."
     [(xs cons)] (car xs))

(fun tail "Get the list tail."
     [(xs cons)] (cdr xs))

(fun length "Find the length of the list."
     [(xs list)]
     (if (nul? xs)
     	 0 (+ 1 (length (tail xs)))))

(fun map "Return a list derived by applying fn to each element in turn."
     [fn (xs list)]
     (if (nul? xs)
     	 () (cons (fn (head xs)) (map fn (tail xs)))))

(fun fiter "Return a list derived by removing in turn elements for which fn? is false."
     [fn? (xs list)]
     (if (nul? xs)
     	 () (let [[head, tail] xs]
	    	 (if (fn? head)
		     (cons head (filter fn? tail))
		     (filter fn? tail)))))

;;; end core.rl