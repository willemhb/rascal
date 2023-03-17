#| global tables used by the interpreter |#
val __syntax__   = table() ;; symbol => procedure
val __globals__  = table() ;; symbol => any
val __loaded__   = table() ;; string => boolean
val __provided__ = table() ;; symbol => string
val __reader__   = table() ;; glyph  => procedure

