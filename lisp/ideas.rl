;; speculative lisp syntax.

(load "boot.rl")

(provide Core)

@doc "Rascal core library (basic syntax, not including the really primitive stuff)."

;; example of simple function definitions (demonstrates multimethod syntax).
(fun count
  @doc "Return the number of elements in `xs`."
  [xs])

(method count
  [xs: List]
  (list-count xs))

(method count
  [xs: Vec]
  (vec-count xs))

(method count
  [xs: Str]
  (str-count xs))

(method count
  [xs: Bin]
  (bin-count xs))

(method count
  [xs: Map]
  (map-count xs))

(fun empty?
  @doc "Return `true` if `xs` is an empty sequence."
  [xs:: Seq]
  (= (count xs) 0))

(fun map
  @doc "Core sequence operation."
  [xform]
  [xform coll])

(method map
  [xform: Fun]
  (fun [coll] (map xform coll)))

(method map
  [key: (Num Sym Str)]
  (fun [coll] (map (fn [x] (key x)) coll)))

(method map
  [xform: Fun coll: List]
  (label loop
    [xform xform, coll coll, acc ()]
    (if (empty? coll)
      (reverse acc)
      (loop xform
        (tail coll)
        (cons (xform (head coll)) acc)))))

;; examples of algebraic effect usage.
