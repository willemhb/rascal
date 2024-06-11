(ns core
  "The rascal core module (mostly bootstrapping macro definitions)."

  ;; helpers.
  (fun destructure
    "Used internally by macros to destructure vectors."
    ([b x bs xs]))

  (method destructure
    [b: Symbol])

  ;; binding syntax.

  ;; block and loop syntax.
  (mac let
    "Create a new scope, binding each `bind` in `binds` to the matching `init` expression in order. Evaluates `body` in "
    ([binds & body]))

  (method let
    [binds: Vector & body]
    (val []))

  ;; branching syntax.
  (mac cond
    "Like `if`, but supports arbitrarily many conditions."
    ([]
     `(throw :eval-error ~$"Unhandled condition in ${&form}."))
    ([t c]
     (if (= t 'otherwise)
       c
       `(if ~t ~c (throw :eval-error ~$"Unhandled condition in ${&form}."))))
    ([t c & b]
     (if (= t 'otherwise)
       (throw :syntax-error $"Non-terminal 'otherwise' in ${&form}.")
       `(if ~t
           ~c
           (cond ~@b)))))

  (mac case
    "Like `cond`, but each `t` is compared to the first expression."
    ([m & body]))