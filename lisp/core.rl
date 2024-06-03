(module core
  "The rascal core module (mostly bootstrapping and macro definitions)."
  (export (val var fun mac type))
  (begin
    ;; binding syntax.
    ;;
    ;; binding syntaxes are bootstrapped using the special forms def*, put*, lmb*, and spec*,
    ;; with the following syntaxes:
    ;; 
    ;; (def* options? identifier value?)
    ;; (put* identifier value)
    ;; (lmb* options? (formals...) body...)
    ;; (spec* options? identifier signature method)
    ;;
    ;; def*  - Creates a new binding for `identifier` in the nearest enclosing environment.
    ;; 
    ;;         `identifier` must be an unqualified non-literal Symbol. It must not already be defined
    ;;         in the current environment.
    ;;
    ;;         If supplied, `options` must be a keyword Map.
    ;;
    ;;         If supplied, `value` must match any constraints given in `options`.
    ;;
    ;; put*  - Updates the binding associated with `identifier`, setting it to `value`.
    ;;
    ;;         `identifier` must be a (possibly qualified) Symbol defined in the current environment.
    ;;          It must either be a variable (final = false) or uninitialized.
    ;;
    ;;         `value` must satisfy the binding's type constraint.
    ;; 
    ;; lmb* -  Creates a new closure that extends the current environment with `formals` and executes
    ;;         `body` with bindings for `formals` when called.
    ;;
    ;;         If `options` is supplied, it must be a keyword Map.
    ;;
    ;;         `formals` must be a list of unqualified, non-literal Symbols. No identifier in `formals` may
    ;;         appear more than once.
    ;;
    ;;         `body` is a sequence of one or more expressions.
    ;;
    ;; spec* - Adds a method to the value associated with the binding for `identifier`.
    ;;
    ;;         If `options` is supplied, it must be a keyword Map.
    ;;
    ;;         `identifier` must be a Symbol with a specializable binding (multi = true) in the current environment.
    ;;
    ;;         `signature` must be a list of Symbols bound to Types. If `identifier` resolves to a macro binding (macro = true),
    ;;         those bindings must be resolvable at compile time.
    ;;
    ;;         `method` must be a lmb* form.
    ;;
    ;; The following higher-level binding syntaxes are supplied:
    ;;
    ;; val   - (val  options?  identifier  docstring? value?)
    ;;
    ;;         Creates a new final binding.
    ;;
    ;; var   - (var  options?  identifier  docstring? value?)
    ;; fun   - (fun  options?  identifier? docstring? (formals...) body...)
    ;; mac   - (mac  options?  identifier  docstring? (formals...) body...)
    ;; type  - (type options?  identifier  docstring? spec)

    (def* { :final  true
            :macro  true
            :multi  true
            :constr Function
            :doc    "Create a new immutable binding." }
          val)

    (spec* val
      (List Environ Symbol Any)
      (lmb* (&form &env name value)
        (list 'def* { :final true } name value)))

    (spec* val
      (List Environ Map Symbol Any)
      (lmb* (&form &env meta name value)
        (list 'def* (merge { :final true } meta) name value)))

    (spec* val
      (List Environ Symbol String Any)
      (lmb* (&form &env name doc value)
        (list 'def* { :final true :doc doc } name value)))

    (spec* val
      (List Environ Map Symbol String Any)
      (lmb* (&form &env meta name doc value)
        (list 'def*
          (merge { :final true :doc doc } meta)
          name
          value)))

    (spec* val
      (List Environ List Any)
      (lmb* (&form &env annot value)
        (list 'def*
              { :final true
                :constr (hd annot) }
              (hd|tl annot)
              value)))

    (def* { :final true
            :macro true
            :multi true
            :doc   "Create a new mutable binding." }
         var)))
