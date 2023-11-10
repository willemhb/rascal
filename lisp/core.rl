#| functions and macros for bootstrapping the language.
   mostly very simple core utilities, core syntax, and helpers for syntactic analysis. |#

;; type & value predicates.
(fun list?
  "List type predicate."
  (x)
  (isa? x List))

(fun empty?
  "Empty sequence predicate for Lists."
  ((List xs))
  (id? xs ()))

(fun vector?
  "Vector type predicate."
  (x)
  (isa? x Vector))

(fun empty?
  "Empty sequence predicate for Vectors."
  ((Vector xs))
  (id? xs []))

(fun map?
  "Type predicate for maps."
  (x)
  (isa? x Map))

(fun empty?
  "Empty sequence predicate for maps."
  ((Map kvs))
  (id? kvs {}))

(fun symbol?
  "Symbol type predicate."
  (x)
  (isa? x Symbol))

(fun type?
  "Type type predicate."
  (x)
  (isa? x Type))

;; simple binding macros.
(mac val
  "Standard syntax for introducing new constant bindings."
  ((Symbol name) value)
  `(do (var ~name ~value)
       (set-metadata! (resolve ~name) :final true)
       (set-metadata! (resolve ~name) :type  (type ~name))))

(mac let*
  "Bitchmade let."
  ((List bindings) & body)
  `((fun ~(map hd bindings) ~@body) ~@(map hd|tl bindings)))

(mac mac*
  "Introduce multiple methods at once."
  ((Symbol name) & argspecs)
  (fun expand-argspec
    (argspec)
    `(mac ~name ~(hd argspec) ~@(tl argspec)))
  `(do ~@(map expand-argspec argspecs)))

(mac mac*
  "Support docstrings."
  ((Symbol name) (String doc) & argspecs)
  (fun expand-argspec
    (argspec)
    `(mac ~name ~doc ~(hd argspec) ~@(tl argspec)))
  `(do ~@(map expand-argspec argspecs)))

(mac* fun*
  "Introduce multiple methods at once."
  (((Symbol name) & argspecs)
   (fun expand-argspec
     (argspec)
     `(fun ~name ~(hd argspec) ~@(tl argspec)))
   `(do ~@(map expand-argspec argspecs)))
  (((Symbol name) (String doc) & argspecs)
   (fun expand-argspec
     (argspec)
     `(fun ~name ~doc ~(hd argspec) ~@(tl argspec)))
   `(do ~@(map expand-argspec argspecs))))

(mac* iflet*
  "Like if, but saves test to supplied binding."
  (((Symbol test-name) test then)
   `(let* ((~test-name ~test))
      (if ~test-name ~then)))
  (((Symbol test-name) test then else)
   `(let* ((~test-name ~test))
      (if ~test-name ~then ~else))))

;; simple branching macros.
(mac* and
  "Classic definition."
  (() true)
  ((x) x)
  ((x & more)
   `(iflet* #test ~x (and ~@more) #test)))

(mac* or
  "Classic definition."
  (() false)
  ((x) x)
  ((x & more)
   `(iflet* #test #test (or ~@more))))