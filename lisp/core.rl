;; syntax definitions.

(load "prelude.rl")

;; binding macros.

;; exceptions.
(mac guard
  "Set an escape point for calls to `raise`."
  ((List handler-clause) & body)
  `(handle
    ((#op #args ))))

