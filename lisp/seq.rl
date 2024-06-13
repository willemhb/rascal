(ns seq
  "Rascal sequence library."

  @private
  (union Key
    "Convenience definition for types commonly used as `Map` or `Vec` keys."
    (Int Sym Str))

  (fun map
    "Lisp classic! Rascal version has additional semantics."
    [f: Func]
      (fun [x] (map f x))
    [k: Key] @promote
      (fun [x] (map #(k %) x))
    [k: Key, x: Any] @promote
      (map #(k %) x)
    [k: Key, x: Any, & r] @promote
      (map #(k %) x & r))

  (fun map
    "Implmentation for `List`s."
    [f: Func, x: List]
      (label
        [f: Func, ac: List, _: ()]
          (reverse ac)
        [f: Func, ac: List, [h & t]: List]
          (loop f t (conj ac (f h)))
        ;; Entry.
        (loop f () x)))

  (fun any?
    "Implement for `List`s."
    [p: Func, _: ()]
      false
    [p: Func, [h & _]: List] @when (p h)
      true
    [p: Func, [_ & t]: List]
      (any? p t))

  (fun all?
    "Implement for `List`s."
    [p: Func, _: ()]
      true
    [p: Func, [h & _]: List] @when (!p h)
      false
    [p: Func, [_ & t]: List]
      (all? p t))