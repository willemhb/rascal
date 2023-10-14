(use (base math bits list vector map))

;; Core sequence and enumeration utilities.
;; collection interface.
(class Collection
  @doc "Class for types storing multiple values."
  ()
  ((fun cnt   ((Collection xs))
     @doc "Get the number of items in `xs`.")
   (fun empty? ((Collection xs))
     @doc "Redturn `true` if `xs` has no elements."
     (zero? (cnt xs)))
   (fun ref   ((Collection xs) k)
     @doc "Lookup the value of `k` in the collection `xs`. Raise an exception if `xs` is has no key `k`.")
   (fun xef   ((Collection xs) k v)
     @doc "Return a new collection identical to `xs`, but with the value `v` associated to the key `k`. Raise an exception if `xs` has no entry for `k`.")
  (fun assoc  ((Collection xs) k v)
     @optional
     @doc "Return a new collection identical to `xs`, but with the value `v` associated to the key `k`. Update the collection if `xs` has no entry for `k`.")))

(class Sequence
  @doc "Class for ordered collection types."
  (Collection)
  ((fun first ((Sequence xs))
     @doc "Return the first element of `xs`. Raise an exception if `xs` has no elements.")
   (fun rest  ((Sequence xs))
     @doc "Return a sequence of the same type as `xs` containing all but the first element.
           Return `nul` if `xs` has no more elements.
           Raise an exception if `xs` is empty.")))

;; implementations.
(impl (Sequence List)
  (fun cnt ((List xs))
    (list-length xs))
  (fun empty? ((List xs))
    (id? xs ())))
