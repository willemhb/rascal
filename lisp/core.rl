(ns core
  "The rascal core module (mostly bootstrapping macro definitions)."

  ;; helpers.
  (fun destructure
    "Used internally by macros to destructure vectors."
    ([a b x]))

  (method destructure
    [a b: Sym x]
    (conj a `(val ~a ~x)))

  (method destructure
    [a b: Vec x]
    (fold λ(destructure %1 (%2 x)) b a))

  ;; binding syntax.
  (mac val
    "Standard syntax for creating new immutable bindings."
    [name bind]
    [name metadata bind])

  ;; block and loop syntax.
  (mac let
    "Create a new scope "
    [& body])

  (method let
    [& body]
    (label
      [accum [expression]]
        ~(do ~@(rev (conj accum expression)))
      [accum [name: Sym, bind & more]]
        (loop (conj accum `(val ~name ~bind)) more)
      [accum [name: Vec, bind & more]]
        (loop (destructure accum name bind) more)
      ;; Entry point.
      `((fn* [] ~(loop () body)))))

  ;; branching syntax.
  (mac cond
    "Like `if`, but supports arbitrarily many conditions."
    []
     `(throw :eval-error ~$"Unhandled condition in ${&form}.")
    [t c]
     (if (= t 'otherwise)
       c
       `(if ~t ~c (throw :eval-error ~$"Unhandled condition in ${&form}.")))
    [t c & b]
     (if (= t 'otherwise)
       (throw :syntax-error $"Non-terminal 'otherwise' in ${&form}.")
       `(if ~t
           ~c
           (cond ~@b))))

  (mac case
    "Like `cond`, but each `t` is compared to the first expression."
    [m & body]
    (label
      expand-body [x]
        `(throw :eval-error ~$"Unhandled case in ${&form}.")
      expand-body [x t c]
        (if (= t 'otherwise)
          c
          `(if (= ~x ~t) ~c (throw ~$"Unhandled condition in ${&form}.")))
      expand-body [x t c & b]
        (if (= t 'otherwise)
          (throw :syntax-error $"Non-terminal 'otherwise' in ${&form}.")
          `(if (= ~x ~t)
            ~c
            ~(expand-body x & b)))
      ;; Entry point.
      (let x (sym)
        `(let ~x ~m
           ~(expand-body x & body)))))

  (union Func
    "All valid function types."
    (Native Closure Generic)))