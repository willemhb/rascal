;;; begin base.rl

(fn real?
  (x) (id? (type-of x) real))

(fn fixnum?
  (x) (id? (type-of x) fixnum))

(fn not
  (x)
  (if x true false))

;;; higher order functions
(fn id
  (x) x)

(fn const
  (x)
  (fn (_) x))

(fn neg
  (f)
  (fn (& xs)
    (not (f .. xs))))

(fn circ
  (f) f)

(fn circ
  (f g)
  (fn (& xs) (f (g .. xs))))

(fn circ
  (f & fs)
  (let (g (circ .. fs))
    (circ f g)))

(fn curry
  (f x)
  (fn (& xs)
    (f x .. xs)))

;;; end base.rl