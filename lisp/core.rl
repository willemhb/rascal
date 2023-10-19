;; bootstrapping the language.

(fun raise
  (xtype & args)
  (perform (list :raise xtype) & args))
