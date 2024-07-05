(ns io
  "Rascal IO standard library module."
  (use [base])

  ;; Standard streams.
  (val &in
    "Standard input port."
    (*standard-input-port*))

  (val &out
    "Standard output port."
    (*standard-output-port*))

  (val &err
    "Standard error port."
    (*standard-error-port*)))

  ;; generic functions.
  ;; output functions.
  (fun prn
    "Write's lisp representation to given output. If the given object has reader support, reading the printed representation should yield an identical object."
    [x]
    [p x]
    [p x & r])

  (method pr
    "Implementation for `List`s."
    [xs: List]
      (prn &out xs)
    [p: Port xs: List]
      (label
        [p xs]
        (do (pr p (hd xs))
          (if (empty?|tl xs)
            (prc p \))
            (do (prc p \space)
              (pr p (tl xs))))))
        ;; Entry point.
        (if (empty? xs)
          (prs p "()")
          (do (prc p \()
            (loop p xs))))

  (fun show
    "Write's a human-readable representation to given output. Does not have the same constraints as prn."
    ([x] (show &out x))
    ([p x])
    ([p x & r]
      (do (show p x)
        (apply show p r))))

  (fun write
    "Write's a textual or binary representation to given output."
    ([x] (write &out x))
    ([p x])
    ([p x & r]
      (do (write x)
        (apply write x r)))))