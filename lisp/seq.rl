(ns seq
  "Rascal sequence library."

  @private
  (union Key
    "Convenience definition for types commonly used as `Map` or `Vec` keys."
    (Int Sym Str))

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
        [f: Func, a: List, _: ()]
          (reverse a)
        [f: Func, a: List, [h & t]: List]
          (loop f t (conj a (f h)))
        ;; Entry.
        (loop f () s))
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
    [p: Func, _: ()]
      false
    [p: Func, [h & _]: List] @when (p h)
      true
    [p: Func, [_ & t]: List]
      (any? p t))

  (method all?
    "Implement for `List`s."
    [p: Func, _: ()]
      true
    [p: Func, [h & _]: List] @when (!p h)
      false
    [p: Func, [_ & t]: List]
      (all? p t))

  (method keep
    "Implement for `List`s."
    [p: Func, x: List]
      (label
        [p: Func, a: List, _: ()]
          (reverse a)
        [p: Func, a: List, [h & t]: List] @when (p h)
          (loop p (conj a h) t)
        [p: Func, a: List, [_ & t]: List]
          (loop p a t)
        ;; Entry.
        (loop p () x))))