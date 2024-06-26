(ns seq
  "Rascal sequence library."
  (use [core math])

  ;; exports: map, keep, take, drop, any?, all?, fold, sort, each

  @private
  (union Key
    "Convenience definition for types commonly used as `Map` or `Vec` keys."
    (Int Sym Str))

  (fun map
    "Lisp classic! Rascal version has additional semantics."
    [f: Func]
      #(map f %)
    [k: Key]
      #(map #(k %) %)
    [k: Key, s: Seq]
      (map #(k %) s)
    [k: Key, s: Seq, & r]
      (map #(k %) s & r))

  (fun any?
    "Returns `true` if `(p x)` is true for at least one `x` in `s`."
    [s: Seq]
      (any? id s)
    [p: Func]
      #(any? p s)
    [k: Key, s: Seq]
      (any? #(k %) s))

  (method map
    "Implmentation for `List`s."
    [f: Func, s: List]
      (label
        [_: Func, a: List, _: ()]
          (rev a)
        [f: Func, a: List, [h & t]: List]
          (loop f (conj a (f h)) t)
        ;; Entry.
        (loop f () s)))

  (method map
    [f: Func, s: List, & r]
      (label
        [_: Func, a: List, r: List] @when (any? empty? r)
          (rev a)
        [f: Func, a: List, r: List]
          (let fs (map fst r)
               rs (map rst r)
               mf (f & fs)
               ma (conj a mf)
            (loop f ma rs))
         ;; Entry.
         (loop f () (conj r s))))

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