(module core
  "The rascal core module (mostly bootstrapping and macro definitions)."
  (export (val var fun mac type))
  (begin
    (fun map
      "A lisp classic!"
      [f: Function xs: List]
      (if (empty? xs)
          ()
          (let [[x & xs] xs]
            (conj (map f xs) (f x)))))))
