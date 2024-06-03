(module core
  "The rascal core module (mostly bootstrapping and macro definitions)."
  (export (val var))
  (begin
    ;; binding syntax.
    (def {:final true
          :macro true
          :multi true
          :doc   "Create a new immutable binding."}
         val
         ())
          

    (mac let
      "Create a new scope with given bindings."
      ((List binds) & body)))
