;; bootstrapping the base language.
(load "prelude.rl")

;; binding forms
@final
@type Function
@macro
@doc "Standard syntax for creating new macro bindings."
@name 'mac
(def mac
  (lmb ((List &form) (Environment &envt) (Symbol name) (List args) & body)
    (annot (list 'def
                 name
                 (cons 'lmb
                       (cons (list 'List '&form) (list 'Environment '&envt) args)
                       body)))
           :macro true
           :type  Function
           :final true
           :name  name))


@doc "Support docstring."
(mac mac
  ((Symbol name) (String doc) (List args) & body)
  (annot (list 'def
               name
               (cons 'lmb
                     (cons (list 'List '&form) (list 'Environment '&envt) args)
                     body))
         :macro true
         :type  Function
         :final true
         :doc   doc
         :name  name))

(mac fun
  "Standard syntax for creating new function bindings and anonymous functions."
  ((List args) & body)
  (cons 'lmb args body))

(mac fun
  "If a name is provided, expand to `(def name (lmb args & body))`."
  ((Symbol name) (List args) & body)
  (annot (list 'def
              name
              (cons 'lmb args body))
         :type  Function
         :final true
         :name  name))

(mac fun
  "Support docstrings."
  ((Symbol name) (String doc) (List args) & body)
  (annot (list 'def
               name
               (cons 'lmb args body))
         :type  Function
         :final true
         :doc   doc
         :name  name))

(mac var
  "Standard syntax for introducing mutable bindings."
  ((Symbol name) value)
  (list 'def name value))

(mac val
  "Standard syntax for introducing immutable bindings."
  ((Symbol name) value)
  (annot (list 'def name value) :final true))

(mac struct
  "Standard syntax for introducing new structure bindings."
  ((Symbol name) (List slots))
  (annot (list 'def
               name
               (list 'ensure-type 'Struct slots))
         :final true
         :type  Type
         :name  name))
