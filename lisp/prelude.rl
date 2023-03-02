(var *syntax* #{})

(var list?   (lmb (x) (isa? x 'list)))
(var symbol? (lmb (x) (isa? x 'symbol)))
(var vector? (lmb (x) (isa? x 'vector)))
(var dict?   (lmb (x) (isa? x 'dict)))
(var set?    (lmb (x) (isa? x 'set)))
(var tuple?  (lmb (x) (isa? x 'tuple)))
(var binary? (lmb (x) (isa? x 'binary)))

(var empty?
  (lmb (x)
    (if (list? x)   (id? x  ())
        (vector? x) (id? x #[])
        (tuple? x)  (id? x  [])
        (dict? x)   (id? x #{})
        (set? x)    (id? x  {})
        (binary? x) (id? x #"")
        :otherwise  (error x "not a sequence"))))

(var add-syntax
  (lmb (name transformer)
    (put *syntax*
      (dict-set *syntax* name transformer))))

(var literal?
  (lmb (x)
    (if (list? x)   (empty? x)
        (symbol? x) (not (keyword? x))
        :otherwise  true)))

;; eval implementation
(var quote?
  (lmb (x)
    (id? (hd x) 'quote)))

(var if?
  (lmb (x)
    (id? (hd x) 'if)))

(var %eval% eval)

(var eval
  (lmb (x env)
    (if (literal? x)    x
        (symbol? x)     (lookup x env)
        (macro-call? x) (eval-macro-call x env)
        (macro-def? x)  (eval-macro-def x env)
        (quote? x)      (eval-quote x)
        (var? x)        (eval-var x env)
        (put? x)        (eval-put x env)
        (do? x)         (eval-do x env)
        (if? x)         (eval-if x env)
        (lmb? x)        (eval-lmb x env)
        :otherwise      (eval-application x env))))

(var repl
  (lmb ()
    (do (var x (read))
        (var v (eval x))
        (print v)
        (repl))))
