(module core
  "The rascal core module (mostly bootstrapping and macro definitions)."
  (export (fun mac struct val var
           let label loop for while
           cond case and or
           catch throw))
  (begin
    ;; binding syntax.
    
    ;; exception syntax.
    (mac throw
      "Raise part of exception syntax."
      (& args)
      `(abort ~(qualify 'exception :exception) ~args))

    (mac throw
      "Exceptiosn can be namespaced."
      ((Symbol exc-type) & args)
      `(abort ~(qualify 'exception exc-type) ~args))

    (mac catch
      "Handle part of exception syntax."
      ((List handler) & body)
      (let (((h-args & h-body)  handler)
            ((h-op h-parms)     h-args))
        `(handle
           (~h-args
             (if (not|has-ns? 'exception ~h-op)
                 (abort ~h-op ~h-parms)
                 (do ~@h-body)))
           ~@body))))
