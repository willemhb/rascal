;; begin syntax.rl

(defc *syntax* {})

(defc add-syntax
      (lmb (name xform)
      	   (table-set *syntax* name xform)))

(defc macro-call-p
      (lmb (form)
      	   (if (list? form)
	       (if (sym? (head form))
	       	   (assoc (head form) *syntax*)
		   nil)
	       nil)))

(defc macro-expand-1
      (lmb (form)
      	   (if (atom? form)
	       form
	       (do (defv f (macro-call? form))
	       	   (if f
		       (apply (car f) (cdr form))
		       form)))))

(defc macro-expand
      (lmb (form)
      	   (if (atom? form)
	       form
	       (do (defv f (macro-call? form))
	       	   (if f
		       (macro-expand (apply (car f)
		       		     	    (cdr form)))
		       (map macro-expand-1 form))))))

;; store original definition of eval
(defc *eval* eval)
(defc *comp* comp)

(defv comp (lmb (x) (*comp* (macro-expand x))))

(defv eval
      (lmb (x)
      	   (if (literal? x)
	       x
	       (if (sym? x)
	       	   (lookup x (current-ns) (current-env))
		   (exec (comp x))))))

(defv repl
      (lmb ()
      	   (print ">> ")
      	   (print (eval (read)))
	   (repl)))

;; basic macros
(add-syntax 'defm
	    (lmb (name args & body)
	    	 (list 'add-syntax
		       name
		       (cons* 'lmb args body))))

(defm defn (name args & body)
      (list 'defv
      	    name
	    (cons* 'lmb args body)))

(defn unquote (form)
      (raise :syntax "unquote outside of backquote"))

(defn unquote-splicing (form)
      (raise :syntax "unquote-splicing outside of backquote"))

(defm quasiquote
      (form)
      (expand-quasiquote form))

(defn expand-quasiquote
      (form)
      ())

;; end syntax.rl