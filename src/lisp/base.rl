(lib base
  "rascal standard library."
  (require (prelude))
  (provide (type number glyph collection sequence))

  ; basic builtin union types
  (type type   (data union))
  (type number (real fixnum small big complex ratio))
  (type glyph  (ascii latin1 utf8 utf16 utf32))

  ; type queries
  (fn isa? (x y)
    "Fallback case, treat y as a singleton value type."
    (=? x y))

  (fn isa? (x (union t))
    "Handle top and bottom specially, otherwise check child types."
    (case t
      top    true
      bottom false
      _      (or (isa? x (:a t))
                 (isa? x (:b t)))))

  (fn isa? (x (data t))
    "Simplest case."
    (=? (typeof x) t))

  ; basic numeric utilities
  (fn zero? (x)
    "x is numerically equivalent to 0?"
    (= x 0))

  (fn one? (x)
    "x is numerically equivalent to 1?"
    (= x 1))

  (fn inc (n)
    "add 1 to x."
    (+ n 1))

  (fn dec (n)
    "sub 1 from x."
    (- n 1))

   (fn sqr (x)
     "Simple x^2."
     (* x x))

   (fn cub (x)
     "Simple x^3."
     (* x x x))

   (fn pow (x n)
     "Fast power algorithm (tail recursive)."
     (labl loop ((x x) (n n) (acc 1))
       (cond (zero? n) acc
             (even? n) (loop (sqr x) (hlv n) acc)
             otherwise (loop x (inc n) (* x acc)))))

    ; sequence interface
    (fn first (xs)
      (throw :not-implemented))

    (fn rest (xs)
      (throw :not-implemented))

    (fn seq (xs)
      (throw :not-implemented))

    (fn seq? (xs)
      (and (implemented? first (list xs))
           (implemented? rest  (list xs))
           (implemented? seq   (list xs))))

    ; sequence implementation for lists
    (fn first ((list xs))
      "Just get the head."
      (hd xs))

    (fn rest ((list xs))
      "Return nul if xs is empty, otherwise just get the tail."
      (if (empty? xs)
          nul
          (tl xs)))

    (fn seq ((list xs))
      "Return xs unchanged."
      xs))