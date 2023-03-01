(var *syntax* #{})

(var set-syntax!
  (lmb (name transformer)
    (put *syntax* (assk *syntax* name transformer))))

(var add-syntax
  (lmb (name transformer)
    (if (has? *syntax* name)
    	(set-syntax! name (add-method (ref *syntax* name) transformer))
	(set-syntax! name transformer))))

(add-syntax 'mac
  (lmb (name args & body)
    `(add-syntax '~name (lmb ~args ~@body))))

(mac fn
  ((list args) & body)
    `(lmb ~args ~@body))

(mac fn
  ((symbol name) (list args) & body)
    `(if (bound? '~name)
    	 (put ~name (add-method ~name (lmb ~args ~@body)))
	 (put ~name (lmb ~args ~@body))))

;; for explicitly overriding methods
(mac fn!
  ((symbol name) (list args) & body)
    `(if (bound? '~name)
         (put ~name (add-method! ~name (lmb ~args ~@body)))
	 (put ~name (lmb ~args ~@body))))

;; basic function definitions (used to define further macros)
;; math basics
(fn zero?
  (x) (= x 0))

(fn one?
  (x) (= x 1))

(fn even?
  (x) (zero? (band x 1)))

(fn odd?
  (x) (one? (band x 1)))

(fn inc
  (x) (+ x 1))

(fn dec
  (x) (- x 1))

;; a couple of sequence utilities
(fn evens
  (xs)
  (fn loop
    (xs n)
    (if (empty? xs)
      ()
      (if (even? n)
    	  (list* (hd xs) (evens (tl xs) (inc n)))
	  (evens (tl xs) (inc n)))))
  (loop xs 0))

(fn odds
  (xs)
  (fn loop
    (xs n)
    (if (empty? xs)
      ()
      (if (odd? n)
    	  (list* (hd xs) (odds (tl xs) (inc n)))
	  (odds (tl xs) (inc n)))))
  (loop xs 0))

;; more macros
(mac let
  ((list args) & body)
  `((fn ~(evens args) ~@body) ~@(odds args)))

(mac and
  () true)

(mac and
  (x) x)

(mac and
  (x & more)
  `(let (test# ~x)
     (if test#
     	 (and ~@more)
	 test#)))

(mac or
  () false)

(mac or
  (x) x)

(mac or
  (x & more)
  `(let (test# ~x)
     (if test#
     	 test#
	 (or ~@more))))

