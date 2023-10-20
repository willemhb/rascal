;; bootstrapping the base language.

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

;; a few helper function definitions.
(fun hd head)
(fun tl tail)
(fun hhd
  ((List xs))
  (hd (hd xs)))

(fun htl
  ((List xs))
  (hd (tl xs)))

(fun thd
  ((List xs))
  (tl (hd xs)))

(fun ttl
  ((List xs))
  (tl (tl xs)))

(fun httl
  ((List xs))
  (hd (tl (tl xs))))

(fun build-list
  "Make a list with x repeated n times."
  (x (Integer n))
  (if (= n 0)
    ()
    (cons x (build-list x (- n 1)))))

(mac _let
  ((List formals) & body)
  (cons (cons 'lmb (map hhd formals) body) (map htl formals)))

(mac let
  ((List formals) & body)
  (_let ((arity (len formals))
         (names (map hhd formals))
         (vals  (map thd formals))
         (init  (map (fun (f) (list 'put (htl f) (httl f))) formals)))
        (cons (cons 'lmb names (cat init body)) (build-list nul arity))))

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
  (let (())

(mac cond
  "Classic lisp semantics."
  (fun expand-cond-clause
    ((List clause))
    (if ())))

;; cache of loaded modules.
(val &used (table))

(fun each
  "Apply `fn` to the elements of `xs` in turn."
  ((Function fn) (List xs))
  (if (id? xs '())
    nul
    (if (= (len xs) 1)
      (fn (head xs))
      (do (fn (head xs))
        (each fn (tail xs))))))

(mac use
  "We don't have any sort of namespacing yet so this is the best we can do."
  ((List modules))
  (each (fun (module)
          (unless (has? &used module)
            (xef &used module (load module)))
          (ref &used module))
        modules))
