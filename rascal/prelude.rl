;; begin prelude.rl

;; core utilities defined before macro expansion has been defined.

;; type predicates
(def isa?
     (lmb (x type)
          (id? (type-of x) type)))

(def isa?-maker
     (lmb (type)
          (lmb (x) (isa? x type))))

(def real?   (isa?-maker 'real))
(def int?    (isa?-maker 'int))
(def unit?   (isa?-maker 'unit))
(def bool?   (isa?-maker 'bool))
(def glyph?  (isa?-maker 'glyph))
(def bin?    (isa?-maker 'bin))
(def sym?    (isa?-maker 'sym))
(def stream? (isa?-maker 'stream))
(def func?   (isa?-maker 'func))
(def cons?   (isa?-maker 'cons))
(def vec?    (isa?-maker 'vec))
(def table?  (isa?-maker 'table))

;; misc. predicates
(def not
     (lmb (x)
          (cond x    false
                else true)))

(def atom?
     (lmb (x) (not (cons? x))))

(def seq?
     (lmb (x)
          (cond (bin? x)   true
                (cons? x)  true
                (vec? x)   true
                (table? x) true
                else       false)))



(provide (isa? real? int? unit? bool? glyph? bin? sym? stream? func? cons? vec? table?))

;; end prelude.rl