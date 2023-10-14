;; bootstrapping the language.

;; special forms
;; (quote x)              => Returns `x` unevaluated.
;; 
;; (var n v)              => Creates a new binding for `n` and sets its value to `v`.
;;                           `n` must be a symbol.
;;
;; (fun (& as) & xs)      => Creates a new function object with one method whose formal
;;                           parameters are given by `a` and whose body is `x`.
;;
;; (fun n (& as) & xs)    => If `n` is unbound in the current scope, a new immutable
;;                           binding is created for `n` and initialized to a new empty
;;                           function object.
;;
;; (mac n (& as) & xs)    => 
;;
;; (put n v)              => Sets the value of `n` to `v`. `n` must be a symbol, and
;;                           its value must have been previously established by an
;;                           `lmb` or `var` form.
;;
;; (do & vs)              => Evaluates the `vs` in order, returning the result of
;;                           the last evaluation. `vs` must not be empty.
;;
;; (if t? c & more)       => Basic branching form.
;;
;;                           Evaluates `t?`. If `t?` is any expression other
;;                           than `false` or `nul`, evaluates `c` and returns the
;;                           result. Otherwise, evaluates (t & more).
;; 
;;                           If `t?` is the symbol `otherwise`, `c` is evaluated
;;                           without evaluating `t?`.
;;
;;                           If `more` has only one expression, that expression is
;;                           evaluated and its result returned.
;;
;;                           If `more` has zero expressions, `nul` is returned.
;;
;; (with (& bind) & xpr)  => Creates a new private scope with bindings created by
;;                           evaluating each `bind` in order. Evaluates each `xpr` in
;;                           in the resulting private scope, returning the last
;;                           result.
;;
;;                           Each `bind` must macro-expand to a `var` form.
;;
;;                           A private scope is captured by closures but ignored by
;;                           binding forms.
;;
;;                           A with form is only licit at global scope.
;;
;; (use (& path))         => Loads each `path` if it hasn't already been loaded.
;;                           Paths are loaded in the same order they appear in `path`.
;;
;; (perform name & args)  => Evaluates `args` as in a function call. `name` remains
;;                           unevaluated. 
;;
;;                           Suspends execution at the call site and searches for
;;                           the nearest enclosing effect handler that matches the
;;                           call to `perform`.
;;
;;                           If a matching effect handler is found, it is executed
;;                           with `args` as its formal parameters and `resume` bound
;;                           to the suspended continuation.
;;
;;                           If a matching effect handler is not found but an
;;                           enclosing context exists, it is searched recursively.
;;
;;                           If a matching effect handler is not found and no
;;                           enclosing context exists, a runtime error is signaled
;;                           and execution escapes to the toplevel (this probably
;;                           results in a failure and non-zero exit unless the code
;;                           is running in the repl).
;;
;;                           `name` may be a keyword (symbol prefixed with ':') or
;;                           a namespaced list of keywords. Any other type is a
;;                           runtime error.
;; 
;; (handle (& hdl) & xpr)  => Establishes a new effect context with effect handlers
;;                           described by each `hdl`. Eavaluates each `xpr` in order,
;;                           returning the last result if the `handle` form exits
;;                           normally.
;;
;;                           A `handle` form fails to exit normally if one of its own
;;                           bound handlers or an enclosing handler is invoked and
;;                           does not invoke `resume`.
;;
;;                           Each `hdl` has the form `((name & arg) & xpr)`. Each
;;                           `name` is either a keyword or a namespaced list of
;;                           keywords. `arg` has the same syntax as the formal
;;                           parameter list of an `lmb` form.
;;
;;                           A `hdl` clause with the form `((name & arg) & xpr)`
;;                           matches a call to `(perform op & parms)` if `op` is an
;;                           exact match for `name` (including namespacing) and
;;                           `parms` matches `arg` according to the same procedure
;;                           used to match method calls.

(with
  ((fun walk ;; temporary shittier version of `walk`.
     (xform coll)
     (if (empty? coll) ()
         otherwise     (cons (xform (head coll))
                             (walk xform (tail coll)))))

   (mac let ;; temporary shittier version of `let`.
     (binds & body)
     `((fun ~(map  first binds) ~@body) ~@(map second binds))))

  (mac val
    @doc "Establishes an immutable binding."
     (name bind)
    `(var ~name (annot :const true) ~bind))

  (mac guard
    @doc "Establishes an escape point for calls to `raise`."
    (handlers & body)
    (fun transform-handler
      (handler)
      (cond ()))
    `(handle ~(map tansform-handler handlers))))
