;; All of the standard function definitions to be used in core.rl

(def
  @final
  @type Function
  @doc "Alias for `head`."
  hd
  head)

(def
  @final
  @type Function
  @doc "Alias for `tail`."
  tl
  tail)

(def
  @final
  @type Function
  hhd
  (lmb
    ((List xs))
    (hd (hd xs))))

(def
  @final
  @type Function
  htl
  (lmb
    ((List xs))
    (hd (tl xs))))

(def
  @final
  @type Function
  thd
  @returns List
  (lmb
    ((List xs))
    (tl (hd xs))))

(def
  @final
  @type Function
  ttl
  (lmb
    ((List xs))
    (tl (tl xs))))

(def
  @final
  @type Function
  httl
  (lmb
    ((List xs))
    (hd (tl (tl xs)))))

(def
  @final
  @type Function
  hhtl
  (lmb
    ((List xs))
    (hd (hd (tl xs)))))

(def
  @final
  @type Function
  @doc "Like cons, but treats atoms as implicit lists."
  cons*
  @returns List
  (lmb
    (x (List xs))
    (cons x xs)))

(def
  @final
  @type Function
  @doc "Like cons, but treats atoms as implicit lists."
  cons*
  @returns List
  (lmb
    (x (List xs))
    (cons x xs)))

(def
  @final
  @type Function
  @doc "Make a list with x repeated n times."
  build-list
  (lmb
    (x (Integer n))
    (if (= n 0)
      ()
      (cons x (build-list x (- n 1))))))

(def
  @final
  @type Table
  @doc "Loaded modules cache."
  &used
  (table))

(def
  @finaly
  @type Function
  @doc "Apply `fn` to the elements of `xs` in turn."
  each
  (lmb
    ((Function fn) (List xs))
    (if (id? xs '())
      nul
      (if (= (len xs) 1)
        (fn (head xs))
        (do (fn (head xs))
          (each fn (tail xs)))))))

;; predicates.
(def
  @final
  @type Function
  @doc "List type predicate."
  list?
  @final
  @returns Boolean
  (lmb (x)
    (isa? List xs)))

(def
  @final
  @type Function
  @doc "Symbol type predicate."
  symbol?
  @final
  @returns Boolean
  (lmb (x)
    (isa? Symbol xs)))

(def
  @final
  @type Function
  @doc "String type predicate."
  string?
  @final
  @returns Boolean
  (lmb (xs)
    (isa? String xs)))

;; macros that are either very simple or useful for making other macros.

(def
  @final
  @type Function
  @doc "Bitchmade version of `let`."
  _let
  @macro
  (lmb
    ((List &form) (Environment &envt) (List formals) & body)
    (cons (cons 'lmb (map hhd formals) body) (map htl formals))))

(def
  @final
  @type Function
  @doc "This macro is simple enough that it's not such a pain in the ass to define it here."
  thunk
  @macro
  (lmb
    ((List &form) (Environment &envt) & expression)
    (cons 'lmb () expressions)))