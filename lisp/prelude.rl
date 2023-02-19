;; begin prelude.rl

;; core utilities defined before macro expansion has been defined.

;; type predicates
(def not (lmb (x)
              (cond (x     false)
                    (:else true))))

;; end prelude.rl