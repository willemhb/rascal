(ns seq
  "Rascal sequence library."

  ;; Exports: map, keep, take, drop, any?, all?, fold, sort, each

  @private
  (union Key
    "Convenience definition for types commonly used as `Map` or `Vec` keys."
    (Int Sym Str))

  (trait Seq: S[X] @as T
    "Trait type for elements that can be iterated over."
    (fst: Opt[X]   [T])
    (rst: Opt[T]   [T])
    (seq: Opt[Seq] [T]))

  (fun seq?
    "Returns `true` if `s` implements `Seq`."
    )

  (impl Seq: List[X]
    "Implementation for `List` type."
    (method first: Opt[X]
      [_: ()] nul)
    (method first: Opt[X]
      [s: List[X]]))

  (fun map
    "Lisp classic! Rascal version has additional semantics."
    [f: Func]
      #(map f %)
    [k: Key] @flatten
      #(map #(k %) %)
    [k: Key, s: Seq] @flatten
      (map #(k %) s)
    [k: Key, s: Seq, & r] @flatten
      (map #(k %) s & r))

  (fun any?
    "Returns `true` if `(p x)` is true for at least one `x` in `s`."
    [s: Seq]
      (any? id s)
    [p: Func]
      #(any? p s)
    [k: Key, s: Seq] @flatten
      (any? #(k %) s))

  (method map
    "Implmentation for `List`s."
    [f: Func, s: List]
      (label
        [_: Func, a: List, _: ()]
          (rev a)
        [f: Func, a: List, [h & t]: List]
          (loop f t (conj a (f h)))
        ;; Entry.
        (loop f () s)))

  (method map
    [f: Func, s: List, & r]
      (label
        [f: Func, a: List, r: List] @when (any? empty? r)
          (reverse a)
        [f: Func, a: List, r: List]
          (let fs (map first r)
               rs (map rest r)
               mf (f & fs)
               ma (conj a mf)
            (loop f ma rs))
         ;; Entry.
         (loop f () s & r)))

  (method any?
    "Implement for `List`s."
    [_: Func, _: ()]
      false
    [p: Func, [h &]: List] @when (p h)
      true
    [p: Func, [& t]: List]
      (any? p t))

  (method all?
    "Implement for `[Func, List]`"
    [_: Func, _: ()]
      true
    [p: Func, [h &]: List] @when (!p h)
      false
    [p: Func, [& t]: List]
      (all? p t))

  (method each
    "Implement for `[Func, List]`."
    [f: Func, _: ()]
      nul
    [f: Func, [h]: List]
      (f h)
    [f: Func, [h & t]: List]
      (do (f h)
        (each f t)))

  (method each
    "Implement for `[Func, Vec]`."
    [f: Func, ]))