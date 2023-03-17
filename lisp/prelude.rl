#| global tables used by the interpreter, should not change ever |#

(val *syntax*   (table))   ;; symbol => procedure
(val *globals*  (table))   ;; symbol => any
(val *loaded*   (table))   ;; string => boolean
(val *provided* (table))   ;; symbol => string
