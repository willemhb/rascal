;; load prelude
(load "macro.rl")

;; basic numeric helpers
(fun zero?
  (n)
  (= n 0))

(fun one?
  (n)
  (= n 1))

(fun even?
  (n)
  (zero? (mod n 2)))

(fun odd?
  (n)
  (one? (mod n 2)))

(fun inc
  (n)
  (+ n 1))

(fun dec
  (n)
  (- n 1))

;; basic sequence helpers
(fun len=?
  (xs n)
  (= (len xs) n))

(fun len-even?
  (xs)
  (even? (len xs)))

(fun len-odd?
  (xs)
  (odd? (len xs)))

(fun evens
  (xs)
  (if (empty? xs)
      ()
      (if (len-even? xs)
          (cons (head xs) (evens (tail xs)))
          (evens (tail xs)))))

(fun odds
  (xs)
  (if (empty? xs)
      ()
      (if (len-odd? xs)
          (cons (head xs)
                (odds (tail xs)))
          (odds (tail xs)))))

;; basic binding macros
(mac %let%
  (args & body)
  (def formals (evens args))
  (def values (odds args))
  `((lmb ~formals ~@body) ~@values))

;; basic control macros
(mac cond
  (test consequent & body)
  (if (empty? body)
      (if (id? test 'otherwise)
          consequent
          `(if ~test ~consequent (error nul "unhandled case in cond")))
      `(if ~test ~consequent (cond ~@body))))

(mac and
  (& body)
  (cond (len=? body 0) true
        (len=? body 1) (head body)
        otherwise      `(if ~(head body) (and ~@(tail body)))))

(mac or
  (& body)
  (cond (len=? body 0) false
        (len=? body 1) (head body)
        otherwise      (%let% (var (gensym))
                         `(%let% (~var ~(head body))
                            (if ~var ~var (or ~@body))))))

(mac catch
  (& body)
  `(ccc throw ~@body))
