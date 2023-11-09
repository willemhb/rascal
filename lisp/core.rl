;; syntax definitions.

(load "prelude.rl")

;; branching macros.
(mac when
  "Like `if`, but with implicit `do` and no consequent."
  (test & body)
  `(if ~test (do ~@body)))

(mac unless
  "Like `when`, but negates the test."
  (test & body)
  `(if (not ~test) (do ~@body)))

(mac use
  "Checks whether a file has already been loaded before loading it."
  (names)
  (each names
        #(if (table-has? &used %)
             (table-get &used %)
             (table-set &used % (load %)))))
