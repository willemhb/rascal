;; operating system library (such as it is)

(def c-getenv (ffi-sym lib-c "getenv"))

(fun getenv (key)
  (ffi-call c-getenv :str (list :str) key))
