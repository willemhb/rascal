;; begin prelude.rl

;; core utilities defined before macro expansion has been defined.

(defv isa? (lmb (x type)
      	   	(id? (typeof x) type)))

(defc not (lmb (x) (if x false true)))

(defc real?       (lmb (x)  (isa? x 'real)))
(defc fix?        (lmb (x)  (isa? x 'fix)))
(defc small?      (lmb (x)  (isa? x 'small)))
(defc unit?       (lmb (x)  (isa? x 'unit)))
(defc bool?       (lmb (x)  (isa? x 'bool)))
(defc glyph?      (lmb (x)  (isa? x 'glyph)))
(defc sym?        (lmb (x)  (isa? x 'sym)))
(defc port?       (lmb (x)  (isa? x 'port)))
(defc native-fn?  (lmb (x)  (isa? x 'native-fn)))
(defc user-fn?    (lmb (x)  (isa? x 'user-fn)))
(defc pair?       (lmb (x)  (isa? x 'pair)))
(defc list?       (lmb (x)  (isa? x 'list)))
(defc vec?        (lmb (x)  (isa? x 'vec)))
(defc table?      (lmb (x)  (isa? x 'table)))
(defc bin?        (lmb (x)  (isa? x 'bin)))
(defc str?        (lmb (x)  (isa? x 'str)))

(defc empty?      (lmb (xs) (id? xs '()))

(defc cons?
      (lmb (xs) (if (list? xs)
	       	    (not (empty? xs))
	       	    false)))

(defc any-pair?
      (lmb (xs) (if (list? xs)
      	   	    (not (empty? xs))
		    (pair? xs))))

(defc atom?
      (lmb (x) (not (cons? x))))

(defc literal?
      (lmb (x) (if (sym? x)
      	       	   (kw? x)
		   (cons? x))))

(defc car
      (lmb (x)
      	   (if (cons? x)
	       (head x)
	       (if (pair? x)
	       	   (fst x)
		   (raise x "Expected a pair or list")))))

(defc cdr
      (lmb (x)
      	   (if (cons? x)
	       (tail x)
	       (if (pair? x)
	       	   (snd x)
		   (raise x "Expected a pair or list")))))

(defv len=?
      (lmb (xs n)
      	   (= (len xs) n)))

(defv car-id?
      (lmb (xs x)
      	   (id? (car xs) x)))

;; end prelude.rl