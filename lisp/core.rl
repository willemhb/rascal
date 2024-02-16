(module core
  "The rascal core module (mostly bootstrapping and macro definitions)."
  (export (fun mac struct val var
           let label loop for while
           cond case and or
           catch throw generator yield))
  (begin
    ;; Binding syntax.
    (def
    ;; `val` methods.
    
    (def
      ^{:macro true
        :doc   "Defines and initializes a new immutable binding."
        :sig   '(List Environment Symbol Any)}
       val
       (lmb (&form &env name bind)
         (list 'def (ann name :final true) bind)))

    (def
      ^{:macro true
        :doc   "Like the simple form, but second argument interpreted as docstring."
        :sig   '(List Environment Symbol String Any)}
      val
      (lmb (&form &env name doc bind)
        (list 'def (ann name :doc doc :final true) bind)))

    (def
      ^{:macro true
        :doc   "The first input is interpreted as a type-annotated name."
        :sig   '(List Environment List Any)}
      val
      (lmb (&form &env spec bind)
        (list 'def
              (ann (hd|tl spec)
                   :type  (hd spec)
                   :final true)
              bind)))

    (def
      ^{:macro true
        :doc   "The first input is interpreted as a type-annotated name and the second is interpreted as a docstring."
        :sig   '(List Environment List String Any)}
      val
      (lmb (&form &env spec doc bind)
        (list 'def
              (ann (hd|tl spec)
                   :type  (hd spec)
                   :final true
                   :doc   doc)
              bind)))

    ;; `var` methods.
    (def
      ^{:macro true
        :doc   "Defines and initializes a new mutable binding."
        :sig   '(List Environment Symbol Any)}
       var
       (lmb (&form &env name bind)
         (list 'def (ann name :final false) bind)))

    (def
      ^{:macro true
        :doc   "Like the simple form, but second argument interpreted as docstring."
        :sig   '(List Environment Symbol String Any)}
      var
      (lmb (&form &env name doc bind)
        (list 'def (ann name :doc doc :final false) bind)))

    (def
      ^{:macro true
        :doc   "The first input is interpreted as a type-annotated name."
        :sig   '(List Environment List Any)}
      val
      (lmb (&form &env spec bind)
        (list 'def
              (ann (hd|tl spec)
                   :type (hd spec)
                   :final false)
              bind)))

    (def
      ^{:macro true
        :doc   "The first input is interpreted as a type-annotated name and the second is interpreted as a docstring."
        :sig   '(List Environment List String Any)}
      val
      (lmb (&form &env spec doc bind)
        (list 'def
              (ann (hd|tl spec)
                   :type  (hd spec)
                   :final false
                   :doc   doc)
              bind)))

    ;; Branching syntax.
    (mac cond
      "Extended form of `if`."
      (& body)
      (if (id? body '())
          '(throw :unhandled-condition (fmt "Unhandled condition in ~a" &form))
          (if (id? 'otherwise (hd|hd body))
              (tl|hd body)
              `(if (hd|hd body)
                   (do ~@(tl|hd body))
                   (cond ~@(tl body))))))

    (mac throw
      "Raise part of exception syntax."
      (& args)
      `(raise ~(with-ns 'exception :exception) '~args))

    (mac throw
      "Exceptiosn can be namespaced."
      ((Symbol exc-type) & args)
      `(raise ~(with-ns 'exception exc-type) '~args))

    (mac catch
      "Handle part of exception syntax."
      ((List handler) & body)
      (let ((h-args  (hd handler))
            (h-body  (tl handler))
            (h-k     (and (= 3 (len h-args)) (hd h-args)))
            (h-op    (if h-k (hd|tl h-args) (hd h-args)))
            (h-parms (if h-k (hd|tl|tl h-args)
                              (hd|tl h-args))))
        `(handle
           (~h-args
             (if (not|has-ns? 'exception ~h-op)
                 ~(if h-k
                      `(raise ~h-k ~h-op ~h-parms)
                      `(raise ~h-op ~h-parms))
                 (do ~@h-body))))
           ~@body))))
