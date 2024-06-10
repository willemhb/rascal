(ns base
  "Main rascal standard library file."
  (use [core])

  ;; common union types.
  (union Function
    "Union of all function types."
    (Native Closure Generic))

  (union Callable
    "Union of all types licit in head position."
    (Function Type))

  ;; Sequence type.
  (struct Sequence[X]
    "Stores state required ")

  ;; Sequence interface.
  (generic seq
    [xs])