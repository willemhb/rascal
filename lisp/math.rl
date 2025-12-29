;; miscellaneous math utilities.
;; standard numeric constants.
(def &pi 3.14159)
(def &e  2.71828)

;; foreign function handles (surely an insane way to do this)
(def c-sin   (ffi-sym lib-c "sin"))
(def c-cos   (ffi-sym lib-c "cos"))
(def c-tan   (ffi-sym lib-c "tan"))
(def c-pow   (ffi-sym lib-c "pow"))
(def c-sqrt  (ffi-sym lib-c "sqrt"))
(def c-cbrt  (ffi-sym lib-c "cbrt"))
(def c-hypot (ffi-sym lib-c "hypot"))

;; ffi wrappers
(def sin (x)
  (ffi-call c-sin :double (list :double) x))

(def cos (x)
  (ffi-call c-cos :double (list :double) x))

(def tan (x)
  (ffi-call c-tan :double (list :double) x))

(def pow (x)
  (ffi-call c-pow :double (list :double :double) x))

(def sqrt (x)
  (ffi-call c-sqrt :double (list :double) x))

(def cbrt (x)
  (ffi-call c-cbrt :double (list :double) x))

(def hypot (x)
  (ffi-call c-hypot :double (list :double :double) x))
