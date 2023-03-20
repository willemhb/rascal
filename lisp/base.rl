(require (prelude))

;; builtin union types
(type data-type (primitive-type object-type))
(type type (primitive-type object-type union-type))
(type glyph (ascii latin-1 utf-8 utf-16 utf-32))
(type int (uint-8 sint-8 uint-16 sint-16 uint-32 sint-32 fixnum big))
(type number (uint-8 sint-8 uint-16 sint-16 uint-32 sint-32 real-32 fixnum real big ratio complex))

(provide 'base)