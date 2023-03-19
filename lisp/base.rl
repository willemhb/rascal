(require prelude)

;; type predicates & other queries
;; numeric types
(fn small?   [x] (isa? x small))
(fn fixnum?  [x] (isa? x fixnum))
(fn real?    [x] (isa? x real))
(fn big?     [x] (isa? x big))
(fn ratio?   [x] (isa? x ratio))
(fn complex? [x] (isa? x complex))

;; other atomic types
(fn unit?    [x] (isa? x unit))
(fn boolean? [x] (isa? x boolean))
(fn glyph?   [x] (isa? x glyph))
(fn pointer? [x] (isa? x pointer))
(fn symbol?  [x] (isa? x symbol))
(fn stream?  [x] (isa? x stream))

;; function types

;; mutable collections

;; user collection types

;; fucked up types

(provide base)