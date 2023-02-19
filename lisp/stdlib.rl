;; begin stdlib.rl

(load "prelude.rl")
(load "syntax.rl")

;; predicates
(defn not
  (x)
  (if x false true))

;; numbers
(defn zero?
  (x)
  (= x 0))

(defn one?
  (x)
  (= x 1))

(defn sqr
  (x)
  (* x x))

(defn cub
  (x)
  (* x x x))

(defn <=
  (x y)
  (or (< x y)
      (= x y)))

(defn >=
  (x y)
  (not (< x y)))

(defn >
  (x y)
  (not (or (< x y)
           (= x y))))

;; sequences
(defn empty?
  (xs)
  (len=? xs 0))

(defn len=?
  (xs n)
  (= (len xs) n))

(defn hd=?
  (xs x)
  (eql? (hd xs) x))

(defn hhd=?
  (xs x)
  (eql? (hd (hd xs)) x))

;; macros


;; end stdlib.rl