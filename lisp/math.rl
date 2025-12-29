;; miscellaneous math utilities.
(def lib-mc (ffi-open "libm.so.6"))

;; standard numeric constants.
(def &pi 3.14159)
(def &e  2.71828)

;; foreign function handles (surely an insane way to do this)
(def c-sin   (ffi-sym lib-mc "sin"))
(def c-cos   (ffi-sym lib-mc "cos"))
(def c-tan   (ffi-sym lib-mc "tan"))
(def c-pow   (ffi-sym lib-mc "pow"))
(def c-sqrt  (ffi-sym lib-mc "sqrt"))
(def c-cbrt  (ffi-sym lib-mc "cbrt"))
(def c-hypot (ffi-sym lib-mc "hypot"))
(def c-exp   (ffi-sym lib-mc "exp"))
(def c-exp2  (ffi-sym lib-mc "exp2"))
(def c-expm1 (ffi-sym lib-mc "expm1"))
(def c-log   (ffi-sym lib-mc "log"))
(def c-log10 (ffi-sym lib-mc "log10"))
(def c-log2  (ffi-sym lib-mc "log2"))
(def c-logm1 (ffi-sym lib-mc "log1p"))

;; ffi wrappers
(fun sin (x)
  (ffi-call c-sin :double (list :double) x))

(fun cos (x)
  (ffi-call c-cos :double (list :double) x))

(fun tan (x)
  (ffi-call c-tan :double (list :double) x))

(fun pow (x y)
  (ffi-call c-pow :double (list :double :double) x y))

(fun sqrt (x)
  (ffi-call c-sqrt :double (list :double) x))

(fun cbrt (x)
  (ffi-call c-cbrt :double (list :double) x))

(fun hypot (x y)
  (ffi-call c-hypot :double (list :double :double) x y))

(fun exp (x)
  (ffi-call c-exp :double (list :double) x))

(fun exp2 (x)
  (ffi-call c-exp2 :double (list :double) x))

(fun expm1 (x)
  (ffi-call c-expm1 :double (list :double) x))

(fun log (x)
  (ffi-call c-log :double (list :double) x))

(fun log2 (x)
  (ffi-call c-log2 :double (list :double) x))

(fun log10 (x)
  (ffi-call c-log10 :double (list :double) x))

(fun log1p (x)
  (ffi-call c-log1p :double (list :double) x))

