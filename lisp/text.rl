(use (base))

;; string and character handling utilities.
;; character class predicates.
(fun alpha?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "alpha")))

(fun alnum?
  ((Glyph g))
 (c-call iswctype Boolean (Glyph String) (g "alnum")))

(fun blank?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "blank")))

(fun cntrl?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "cntrl")))

(fun digit?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "digit")))

(fun graph?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "graph")))

(fun lower?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "lower")))

(fun print?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "print")))

(fun space?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "space")))

(fun upper?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "upper")))

(fun xdigit?
  ((Glyph g))
  (c-call iswctype Boolean (Glyph String) (g "xdigit")))

(fun upper
  ((Glyph g))
  (c-call towupper Glyph (Glyph) (g)))

(fun lower
  ((Glyph g))
  (c-call towlower Glyph (Glyph) (g)))
