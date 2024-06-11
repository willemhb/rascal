(ns io
  "Rascal IO standard library module."

  ;; Standard streams.
  (val &in
    "Standard input port."
    (*standard-input-port*))

  (val &out
    "Standard output port."
    (*standard-output-port*)

  (val &err
    "Standard error port."
    (*standard-error-port*)))

  ;; generic functions.
  ;; output functions.
  (fun prn
    "Write's lisp representation to given output. If the given object has reader support, reading the printed representation should yield an identical object."
    ([x] (prn &out x))
    ([p x])
    ([p x & r]
     (do (prn p x)
       (apply prn p r))))

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