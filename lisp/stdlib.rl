;; begin stdlib.rl

;; core data types
(defv *syntax* {})

(defv add-syntax
  (fn (name xform)
    (table-set *syntax* name xform)))



;; end stdlib.rl