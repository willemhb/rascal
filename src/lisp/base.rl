;; binding syntax
(def mac
  (macro (lmb (name args & body)
    `(def ~name (macro (lmb ~args ~@body))))))

(mac fun
  (name args & body)
  `(def ~name (lmb ~args ~@body)))

(mac type
  (name slots)
  `(def ~name (record-type '~name '~slots)))

;; control syntax
(mac catch
  (lmb (& body)
    `(ccc throw ~@body)))

;; quasiquote
