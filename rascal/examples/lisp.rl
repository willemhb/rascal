;; examples of lisp syntax

;;; type declarations
(type time real)                            ;; type alias
(type number {real, small, complex, ratio}) ;; union type
(type person [name, age, gender, student?]) ;; record type
(type tree                                  ;; algebraic type (just a bunch of syntactic sugar)
      (empty [])
      (leaf  [key, val])
      (node  [left, right]))

;;; method definition examples
(fun length [(tr tree)]
     (case tr
     	   [(empty)        0]
	   [(leaf  [k, v]) 1]
	   [(node  [l, r]) (+ (length l) (length r))]))

(fun map [(fn Call), (tr tree)]
     (case tr
     	   [(empty) tr]
	   [(leaf [k, v]) (leaf k v.fn)]
	   [(node [l, r]) (node (map fn l) (map fn r))]))

(fun filter [(fn? Call), (tr tree)]
     (case tr
     	   [(empty) tr]
	   [(leaf [k, v]) (if (fn? k) tr (empty))]
	   [(node [l, r]) (node (filter fn? l) (filter fn? r))]))

(export [time, number, person, tree, length, filter])