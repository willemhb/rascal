;; bootstrapping the base language.
(load "prelude.rl")

;; binding forms.
(def
  @final
  @type Function
  @doc "Standard form for introducing macro bindings."
  mac
  @macro
  (lmb ((List &form) (Envrionment &envt) (Symbol name) (List args) & body)
    (list 'def
          (annot name :type Function :final true)
          (annot (cons 'lmb (cons (list List '&form) (list Envrionment '&envt) args) body) :macro true))))

(mac mac
  ((Symbol name) (String doc) (List args) & body)
  (cons 'mac (annot name :doc doc) args body))

(mac fun
  "Without an initial argument, this expands to a plain lambda (preferred)."
  ((List args) & body)
  (cons 'lmb args body))

(mac fun
  "Common case, expands to (def name (lmb args & body))."
  ((Symbol name) (List args) & body)
  (list 'def
        (annot name :type Function :final true)
        (cons 'lmb args body)))

(mac fun
  "As with `mac`, expand doc string to an annotation."
  ((Symbol name) (String doc) (List args) & body)
  (list 'def
        (annot name :type Function :final true :doc doc)
        (cons 'lmb args body)))

(mac fun
  "If both arguments are symbols, binds the same function object to the new name."
  ((Symbol name) (Symbol fname))
  (list 'def (annot name :type Function :final true) fname))

(mac fun
  "Like above but with doc string syntax."
  ((Symbol name) (String doc) (Symbol fname))
  (list 'def (annot name :type Function :final true :doc doc) fname))

(mac var
  "Standard form for introducing general bindings."
  ((Symbol name) val)
  (list 'def name val))

(mac val
  "Standard form for introducing immutable bindings (preferred over `var` unless assignment is desired explicitly)."
  ((Symbol name) val)
  (list 'def (annot name :final true) val))

(mac let
  "Standard syntax for introducing temporary variables. Should probably implement destructuring at some point."
  ((List formals) & body)
  (fun exp0
    (formal)
    (list 'put (htl formal) (httl formal)))
  (fun exp1
    (formals body)
    (cons '_let formals body))
  (fun exp2
    (formals body)
    (_let ((arity (len formals))
           (names (map hhd  formals))
           (vals  (map thd  formals))
           (init  (map exp0 formals)))
      (cons (cons 'lmb names (cat init body)) (build-list nul arity))))
  (if (= (len formals) 1) ;; Simple optimization for common cases.
      (exp1 formals body)
      (exp2 formals body)))

(mac inc!
  "Common macro for rebinding counters."
  (counter)
  (list 'put counter (list '+ counter 1)))

(mac dec!
  "Common macro for rebinding counters."
  (counter)
  (list 'put counter (list '- counter 1)))

;; exceptions.
(mac guard
  "Sets a save point for calls to `raise`."
  ((List handlers) & body)
  (fun expand-handler
    (handler)
    (cons (cons (cons* :exception (hhd handler))
                (thd handler))
          (tl handler)))
  (cons 'handle
        (map expand-handler handlers)
        body))

(mac raise
  "Returns to a save point previously established by guard, invoking the matching handler."
  ((String msg) & args)
  (cons 'perform :exception msg & args))

(mac raise
  "Exceptions can be matched on an exception type."
  ((Symbol type) (String msg) & args)
  (cons 'perform (list :exception type) msg args))

(mac raise
  "Or a namespaced exception type."
  ((List type) (String msg) & args)
  (cons 'perform (cons :exception type) msg args))

;; branching forms.
(mac when
  "Helper syntax for executing a sequence of instructions when a condition holds true."
  (test & body)
  (list 'if test (cons 'do body)))

(mac unless
  "Like `when`, but negates the test."
  (test & body)
  (list 'if
        (list 'not test)
        (cons 'do body)))

(mac and
  "Classic recursive definition. With 0 inputs, returns true."
  () true)

(mac and
  "With 1 input, return its value."
  (x) x)

(mac and
  "With many, only evaluate tail if head is true."
  (x & more)
  (let ((tmp (sym)))
    (cons 'let
          (list (list tmp x))
          (list 'if tmp (cons 'and more) tmp))))

(mac or
  "Classic recursive definition. With 0 inputs, returns false."
  () false)

(mac or
  "With 1 input, returns its value."
  (x) x)

(mac or
  "With many, only evaluate tail if head is false."
  (x & more)
  (let ((tmp (sym)))
    (cons 'let
          (list (list tmp x))
          (list 'if tmp tmp (cons 'or more)))))

(mac cond
  "Classic lisp semantics."
  (& clauses)
  (if (id? clauses '())
      (list 'raise "Unhandled `cond` case.")
      (if (id? (hhd clauses) 'otherwise)
          (cons 'do (thd clauses))
          (list 'if
                (hhd clauses)
                (cons 'do (thd clauses))
                (cons 'cond (tl clauses))))))

(mac do*
  "Like `do`, but returns its first input."
  (x & more)
  (let ((result (sym)))
    (list 'let
          (list (list result x))
          (cons 'do more)
          result)))

;; quasiquote.
(mac unquote
  (_)
  (list 'raise :syntax-error "Bare unquote."))

(mac splice
  (_)
  (list 'raise :syntax-error "Bare splice."))

(fun expand-backquote
  "Handler for backquote."
  (quoted)
  (cond ((not (list? quoted))      (list 'quote quoted))
        ((id? 'unqote (hd quoted)) (htl quoted))
        ((id? 'splice (hd quoted)) ())
        (otherwise                 ())))

(mac backquote
  "Entry point for `backquote`."
  (expression)
  (expand-backquote expression 0))

;; more difficult macros (like yield) defined after backquote.
;; iteration macros.
(mac label
  ((Symbol name) (List formals) & body)
  `(do (fun ~name ~(map hd formals) ~@body)
       (~name ~@(map htl formals))))

(mac for
  ((Symbol loop-var) :in collection :do & body)
  `(let ((#c collection))
    (if (empty? #c)
        nul
        (label #l ((~loop-var (first #c))
                       (#c    (rest #c)))
          (let ((#r (do ~@body)))
            (if (empty? #c)
                #r
                (#l (first #c)
                    (rest #c))))))))

(mac while
  (test :do & body)
  `(label #l ((#r nul))
     (if ~test
         #r
         (#l (do ~@body)))))

;; generators.
(mac yield
  "Pass argument out of the generator."
  (arg)
  `(perform :yield ~arg))

(mac generator
  "Creates an anonymous generator."
  ((List args) & body)
  `(fun ~args
     (let ((#r nul))
       (handle
         (((:yield #x) (put #r resume) #x))
         (fun (#v)
           (if #r (#r #v) (do ~@body)))))))

(mac generator
  "Creates a named generator."
  ((Symbol name) (List args) & body)
  `(fun ~name ~args
     (let ((#r nul))
       (handle
         (((:yield #x) (put #r resume) #x))
         (fun (#v)
           (if #r (#r #v) (do ~@body)))))))

(mac generator
  "Support docstring."
  ((Symbol name) (String doc) (List args) & body)
  `(fun ~(annot name :doc doc) ~args
    (let ((#r nul))
      (handle
        (((:yield #x) (put #r resume) #x))
        (fun (#v)
          (if #r (#r #v) (do ~@body)))))))

;; miscellaneous.
(mac use
  "We don't have any sort of namespacing yet so this is the best we can do."
  ((List modules))
  (for module in modules do
    (unless (has? &used module)
      (let ((result (load module)))
        (xef &used module result)))
    (ref &used module)))

