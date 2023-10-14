(use (base math bits list vector map))

;; Core sequence and enumeration utilities.
;; collection interface.
(class (Term Collection)
  @doc "Class for types storing multiple values."
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

(fun collection?
  @doc "General type predicate for Collections."
  (x)
  (isa? x Collection))

(class (Collection Sequence)
  @doc "Class for ordered collection types."
  ((fun first ((Sequence xs))
     @doc "Return the first element of `xs`. Raise an exception if `xs` has no elements.")
   (fun rest  ((Sequence xs))
     @doc "Return a sequence of the same type as `xs` containing all but the first element. Return `nul` if `xs` has no more elements. Raise an exception if `xs` is empty."))
   (fun conj ((Sequence xs) x)
     @doc "Return a sequence of the same type as `xs` with the element `x` added to it. Whether `x` is added at the start or the end is not specified.")
   (fun conj ((Sequence xs) x & more)
     @doc "Return a sequence of the same type as `xs` with the element `x` and all of the `more`s added to it. Whether the new elements are added at the start or the end is not specified."))

(fun sequence?
  @doc "General type predicate for Sequences."
  (x)
  (isa? x Sequence))

;; implementations.
(impl (Sequence List)
  (fun cnt ((List xs))
    (list-length xs))

  (fun empty? ((List xs))
    (id? xs ()))

  (fun first ((List xs))
    (cond ((empty? xs) (raise :value-error "`first` called on `()`."))
          (otherwise   (head xs))))

  (fun rest ((List xs))
    (case (xs)
      ((())       (raise :value-error "`rest` called on `()`."))
      (((_))      nul)
      (((_ & xs)) xs)))

  (fun ref ((List xs) (Integer k))
    (case (k xs)
      ((_ ())       (raise :value-error "`ref` called on `()`."))
      ((0 (x & _))  x)
      ((i (_ & xs)) (ref (dec i) xs))))

  (fun xef ((List xs) (Integer k) v)
    (case (k xs)
      ((_ ())       (raise :value-error "`xef` called on `()`."))
      ((0 (_ & xs)) (cons v xs))
      ((i (x & xs)) (cons x (xef (dec i) xs)))))

  (fun conj ((List xs) x)
    (cons x xs))

  (fun conj ((List xs) x & more)
    (cons x (conj xs & more))))

;; Fallback implementations of sequence utilities.
(fun walk
  @doc "Just like `map` in traditional lisps."
  ((Function f) (Sequence xs))
  (if (empty? xs)
    ()
    (conj (walk f (rest xs))
      (f (first xs)))))

(fun keep
  @doc "Just like `filter` in traditional lisps."
  ((Function p?) (Sequence xs))
  (cond
    ((empty? xs)     xs)
    ((p? (first xs)) (conj (keep p? (rest xs)) (first xs)))
    (otherwise       (keep p? (rest xs)))))

(fun fold
  @doc "Just like `fold` in traditional lisps. If The initializer is not supplied, it's taken from the sequence (but the sequence must be non-empty)."
  ((Function op) (Sequence xs))
  (if (empty? xs)
    (raise :value-error "`fold` called on empty sequence with no initializer given.")
    (fold op (rest xs) (first xs))))

(fun fold
  @doc "Just like `fold` in traditional lisps."
  ((Function op) (Sequence xs) acc)
  (if (empty? xs)
    acc
    (fold op (rest xs) (op acc (first xs)))))
