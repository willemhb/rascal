#| begin base.rl |#

(val *loaded* #{})
(val *syntax*  {})

;; builtin type predicates
(fn real?
  (x)
  (isa? x real))

(fn fixnum?
  (x)
  (isa? x fixnum))

(fn int?
  (x)
  (isa? x int))

(fn symbol?
  (x)
  (isa? x symbol))

(fn function?
  (x)
  (isa? x function))

(fn stream?
  (x)
  (isa? x stream))

(fn list?
  (x)
  (isa? x list))

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

;;; sequences
(fn len
  ((list xs))
  (list-len xs))

(fn len
  ((tuple xs))
  (tuple-len xs))

(fn len
  ((vector xs))
  (vector-len xs))

(fn len
  ((string xs))
  (string-len xs))

(fn len
  ((binary xs))
  (binary-len xs))

(fn len
  ((dict xs))
  (dict-len xs))

(fn len
  ((set xs))
  (set-len xs))

(fn empty?
  (xs)
  (= 0 (len xs)))

(fn first
  ((list xs))
  (if (empty? xs)
      nul
      (head xs)))

(fn rest
  ((list xs))
  (if (empty? xs)
      nul
      (tail xs)))

(fn first
  ((string cs))
  (if (empty? cs)
      nul
      (string-ref cs 0)))

(fn rest
  ((string cs))
  (if (empty? cs)
      nul
      (substring cs 1)))

(fn first
  ((binary bs))
  (if (empty? bs)
      nul
      (string-ref bs 0)))

;;; error handling
(mac catch
  (handler & body)
  `(with (raise (fn ~@handler))
     (do ~@body)))

#| end base.rl |#
