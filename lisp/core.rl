#| functions and macros for bootstrapping the language.
   mostly very simple core utilities, core syntax, and helpers for syntactic analysis. |#

;; type & value predicates.
(fun list?
  "List type predicate."
  (x)
  (isa? x List))

(fun vector?
  "Vector type predicate."
  [x]
  (isa? x Vector))

(fun map?
  "Map type predicate."
  [x]
  (isa? x Map))

(fun bits?
  "Bits type predicate."
  [x]
  (isa? x Bits))

(fun string?
  "String type predicate."
  [x]
  (isa? x String))

(fun symbol?
  "Symbol type predicate."
  [x]
  (isa? x Symbol))

(fun type?
  "Type type predicate."
  [x]
  (isa? x Type))

(fun empty?
  ("Empty List predicate."
   [xs::List]
   (id? xs ()))
  ("Empty Vector predicate."
   [vs::Vector]
   (id? vs []))
  ("Empty Map predicate."
   [kvs::Map]
   (id? kvs {})))

;; simple syntax manipulation helpers and macros.
(fun syntax-error
  "Helper for raising a syntax error."
  [&form &envt message]
  (perform (:exception :syntax-error) ("syntax error in `#{&form}`: #{message}.")))

(fun unhandled-condtion
  [&form]
  (perform (:exception :unhandled-condition) "unhandled condition in #{&form}."))

;; simple branching macros.
(mac and
  "Classic definition."
  ([] true)
  ([x] x)
  ([x & more]
   `(let [#test ~x]
      (if #test
          (and ~@more)
          #test))))

(mac or
  "Classic definition."
  ([] false)
  ([x] x)
  ([x & more]
   `(let [#test ~x]
      (if #test
          #test
          (or ~@more)))))

(mac cond
  "Branching macro for multiple cases."
  ([]
   `(unhandled-condition ~&form))
  ([test then]
   (if (id? test 'otherwise)
       then
       `(if ~test
            ~then
            (unhandled-condition ~&form))))
  ([test then & else]
   `(if ~test
        ~then
        (cond ~@else))))

;; advanced syntax manipulation helpers and macros.
(fun cons?
  "Pseudo-type predicate for non-empty lists."
  [x]
  (and (list? x)
       (not|empty? x)))

(fun atom?
  "Syntactic type predicate for terminals."
  [x]
  (or (not|list? x)
      (empty? x)))

(fun identifier?
  "Syntactic type predicate for symbols used to name things."
  [x]
  (and (symbol? x)
       (not|metadata x :literal)))

(fun keyword?
  "Syntactic type predicate for symbols that always evaluate to themselves without quoting."
  [x]
  (and (symbol? x)
       (metadata x :literal)))

(fun annotation?
  "Sytactic type predicate for annotated names."
  [x]
  (and (l)))