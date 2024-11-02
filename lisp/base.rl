(module 'base
  "Rascal base module."
  (import '[core])
  (export '[Real Glyph Unit Port Sym List Pair Vec Map])
  (export '[real? glyph? unit? port? list? pair?])

  ;; builtin types
  (type Real
    "High-level type alias for builtin type 'real'."
    #real-type)

  (type List
    "High-level type alias for builtin type 'list'."
    #list-type)

  (type Pair
    "High-level type alias for builtin type 'pair'."
    #bt-pair))