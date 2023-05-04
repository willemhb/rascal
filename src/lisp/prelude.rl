#| a few important utilities |#

(def not
  (lmb (x)
    (if x true false)))

(describe! #'not :doc "Logical negation." :type function :immutable true)

(def list?
  (lmb (x)
    (isa? x list)))

(describe! #'list? (list :doc "List type predicate." :type function :immutable true))

(def atom?
  (lmb (x)
    (not (list? x))))

(describe! #'atom? :doc "Leaf s-expression?" :type function :immutable true)

#| global tables used by the interpreter, should not change ever |#

(def *syntax* (table))

(describe #'*syntax* :doc "Macro replacements namespace." :type table :immutable true)

(def add-syntax!
  (lmb (name xform)
    (table-set *syntax* name xform))

