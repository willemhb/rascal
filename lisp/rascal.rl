;; rascal standard library (such as it is)

;; miscellaneous utility macros
(mac thunk
  (xpr) `(fn () ~thunk))

