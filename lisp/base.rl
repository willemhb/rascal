;; standard library.

(use (core))

(mac thunk
  @doc "Wrap a block of code in a zero-arity function."
  (& body)
  `(lmb () ~@body))
