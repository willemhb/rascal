(ns seq
  "Sequence interface. And functions making use of the interface."

  (fun first
    "Get the first item in a sequence."
    ([x] (first (seq x))))

  (fun rest
    "Get the remaining items in a sequence. Returns `nul` if the sequence has no more items."
    ([x] (rest (seq x))))

  (fun seq
    "Return a seqable object. Return `nul` if the sequence has no more items. May return `xs`, a `Seq` proxy object, or `nul` if the sequence is empty."
    ([x] (get-seq x nul)))

  (fun seq?
    "Should return `true` if the object implements the `seq` interface."
    ([x]))

  (fun done?
    "Return `true` if a call to `first` and `rest` would return `nul`."
    ([x] (= x nul)))

  (fun get-first
    "Types that use a `Seq` proxy object should implement this instead of `first`."
    ([x s]))

  (fun get-rest
    "Types that use a `Seq` proxy object should implement this instead of `rest`."
    ([x s p]))

  (fun get-seq
    "Types use a `Seq` proxy object should implement this instead of `seq`."
    ([x p]))

  ;; Implementations for builtin types.
  ;; List
  (method seq
    [x: List]
    (if (empty? x) nul x))

  (method seq?
    [x: List] true)

  (method done?
    [x: List] (empty? x))

  (method first
    [x: List]
    (hd x))

  (method rest
    [x: List]
    (if (empty? (tl xs))
      nul
      (tl xs)))

  ;; Pair
  (method seq
    [x: Pair] x)

  (method seq?
    [x: Pair] true)

  (method first
    [x: Pair] (car x))

  (method rest
    [x: Pair]
    (if (seq? (cdr x))
      (cdr x)
      nul))

  ;; Vectors.
  (method get-seq
    [x: Vector p]
    (let [p (Seq p x 0)]
      (if (> (depth x) 0)
        (get-seq (root x) p)
        p)))

   (method get-seq
     [x: VecNode p]
     (let [p (Seq p x 0)]
       (if (leaf? x)
         p
         (get-seq (nth x 0) p))))

   ;; Functions and macros for operating on sequences.
   (fun map
     "Lisp classic!"
     ([f])
     ([f x] (map f (seq x)))
     ([f x & r]))

  (method map
    [i: Integer]
    (map #(nth % i)))

  (method map
    [s: Symbol]
    (map #(ref % s)))

  (method map
    [f: Function]
    #(map f %))

  (method map
    [f: Function s: Seq &&& :into c]))
